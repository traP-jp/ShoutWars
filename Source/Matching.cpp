//FIXME:ユーザが途中退室すると確実にバグる
#include "Matching.hpp"
# include "common_function.hpp"
# include <ranges>
using namespace std;

namespace
{
	//シーンの切り替えと Game の読み込みより長く取る
	constexpr Duration StartDelay = 2s;
	//CPU と対戦するとき、自分がキャラを確定してから、CPU が参加する・CPU がキャラを確定する・対戦を始めるまでの時間。一瞬で進むと何が起きたか分からないので間を置く
	constexpr Duration CpuJoinDelay = 0.6s;
	constexpr Duration CpuDecideDelay = 1.4s;
	constexpr Duration CpuStartDelay = 2.4s;
}

Matching::Matching(const InitData& init) : IScene(init)
{
	// 光らせるやつの初期化
	for (const auto chars = { select_char_img1, select_char_img2, select_char_img3, select_char_img4 };
		auto& charactors : chars) {
		select_char_glow[&charactors - chars.begin()].init(charactors);
	}
	for (auto&& [img, glow] : std::views::zip(stand_char_img, character_glow)) {
		glow.init(img);
	}
	decide_button_glow.init(decide_img);
	setting_glow.init(setting_img);
	return_glow.init(return_img);

	vs_cpu = (getData().room_mode == 2);
	is_owner = (getData().room_mode != 1);
	room_ID = getData().room_ID;

	//設定画面から戻ってきた場合は同じ部屋を使い続ける
	if (!getData().room) requestRoom();
}

void Matching::requestRoom()
{
	if (vs_cpu) {
		getData().room_ID.clear();
		room_ID.clear();
		getData().room = std::make_unique<Multiplay::LocalRoom>();
		return;
	}
	if (is_owner) {
		getData().room_ID.clear();
		room_ID.clear();
		joining = getData().server.api.create(U"Owner", 2);
	}
	else {
		joining = getData().server.api.join(Unicode::Widen(getData().room_ID), U"Guest");
	}
}

void Matching::showError(const Multiplay::APIError& error)
{
	const String& code = error.code;
	if (code == U"room_not_found") {
		error_ID = (getData().room_ID == "114514") ? 2 : 1;
	}elif(code == U"version_mismatch") {
		error_ID = 3;
	}elif((code == U"room_full") || (code == U"game_started") || (code == U"room_limit_reached")) {
		error_ID = 5;
	}elif(code == U"invalid_session") {
		error_ID = 6;
	}elif((code == U"internal") || (code == U"timeout") || (code == U"network")) {
		error_ID = 4;
	}
	else {
		error_ID = 0;
	}
	OutputLogFile("(" + code.narrow() + ")\n" + error.message.narrow());
	error_mode = 1;
}

void Matching::updateRoom()
{
	if (joining.isReady()) {
		const auto joined = joining.get();
		if (!joined) {
			//アプリに届く前に返った応答なので、送り直しても部屋は重複しない
			if (joined.error().code == U"unavailable") requestRoom();
			else showError(joined.error());
			return;
		}
		getData().room_ID = joined->code.narrow();
		room_ID = getData().room_ID;
		getData().room = std::make_unique<Multiplay::Room>(getData().server.api, *joined, getData().server.syncLogDirectory);
	}

	if (!getData().room) return;
	auto& room = *getData().room;
	room.update();
	if (room.error()) {
		showError(*room.error());
		return;
	}

	room.sendReport(U"lobby", JSON{ { U"character", character_number }, { U"decided", getData().decided_character } });
	//CPU と対戦するときは、自分がキャラを確定したら、CPU が参加して自分と違うキャラを選んで確定する
	if (auto* local = dynamic_cast<Multiplay::LocalRoom*>(&room); local && getData().decided_character) {
		if (!cpu_wait.isStarted()) cpu_wait.start();
		if (!local->hasCpu() && (CpuJoinDelay <= cpu_wait.elapsed())) {
			Array<int> others;
			for (int i = 0; i < 4; i++) {
				if (selectable_characters[i] && (i != character_number)) others << i;
			}
			//選べるキャラが 1 体しかなければ、同じキャラ同士で戦う
			const int cpu_character = others.isEmpty() ? character_number : others.choice();
			local->addCpu();
			local->sendCpuReport(U"lobby", JSON{ { U"character", cpu_character }, { U"decided", false } });
		}
		if (local->hasCpu() && !opponent_decided && (CpuDecideDelay <= cpu_wait.elapsed())) {
			local->sendCpuReport(U"lobby", JSON{ { U"character", opponent_character_number }, { U"decided", true } });
		}
	}

	for (const auto& event : room.receiveReports()) {
		if (event.type != U"lobby") continue;
		opponent_character_number = event.data[U"character"].get<int32>();
		const bool decided = event.data[U"decided"].get<bool>();
		//相手のキャラが確定！
		if (decided && !opponent_decided) decision_sound.playOneShot();
		opponent_decided = decided;
	}
	opponent_present = (2 <= room.users().size());
	//相手が抜けた
	if (!opponent_present) opponent_decided = false;

	//相手が確定したキャラを選んでいたら選び直す。ほぼ同時に同じキャラで確定したときは、部屋主を優先してゲストの確定を取り消す
	if (!isSelectable(character_number) && !(room.isOwner() && getData().decided_character)) {
		getData().decided_character = false;
		stepCharacter(1);
	}

	//双方が別のキャラで確定したら、部屋主がキャラの組み合わせを確定させる (CPU 戦は、選べるキャラが 1 体しかなければ同じキャラ同士で戦う)
	if (room.isOwner() && getData().decided_character && opponent_decided && (vs_cpu || (character_number != opponent_character_number)) && !start_sent && (!vs_cpu || (CpuStartDelay <= cpu_wait.elapsed()))) {
		room.sendAction(U"start", JSON{ { U"owner", character_number }, { U"guest", opponent_character_number } });
		room.start();
		start_sent = true;
	}

	for (const auto& event : room.receiveActions()) {
		if (event.type != U"start") continue;
		getData().player[0] = event.data[U"owner"].get<int32>();
		getData().player[1] = event.data[U"guest"].get<int32>();
		//CPU 戦は読み込みを待ち合わせる相手がいないので、すぐ始める
		getData().start_tick = event.tick + (vs_cpu ? 0 : static_cast<uint64>(StartDelay / room.joined().tickDuration));
		gotoGame = true;
		getData().before_scene = State::Matching;
		changeScene(State::Game, 0.8s);
	}
}

bool Matching::isSelectable(int character) const
{
	return selectable_characters[character] && !(opponent_decided && (opponent_character_number == character));
}

void Matching::stepCharacter(int step)
{
	do {
		character_number = (character_number + step + 4) % 4;
	} while (!isSelectable(character_number));
	character_changed = true;
}

String Matching::CalcRemainingTime()
{
	//制限時間は部屋の作成から5分 (tick は部屋の作成時が 0)。サーバーの部屋の期限 (10分) より短く取る
	int remaining_int_time = 300;
	if (const auto& room = getData().room; room && room->lastTick()) {
		remaining_int_time -= static_cast<int>(*room->lastTick() * room->joined().tickDuration.count());
	}
	//時間切れ☆
	if (remaining_int_time < 1) {
		remaining_int_time = 0;
		error_mode = 1;
		error_ID = 6;
	}
	string minute = to_string(remaining_int_time / 60);
	string second = to_string(remaining_int_time % 60);
	//0埋め
	minute.insert(minute.begin(), 2 - minute.size(), '0');
	second.insert(second.begin(), 2 - second.size(), '0');
	return Unicode::Widen(minute + ":" + second);
}

void Matching::update()
{
	//エラーダイアログ
	if (error_mode == 1) {
		error_timer = (int)Time::GetMillisec();
		error_mode = 2;
		return;
	}elif(error_mode == 2) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - error_timer <= 200) {
			error_pos_y = 1400 - 1040 * (now_time - error_timer) / 200;
			back_alpha = 0.8 * (now_time - error_timer) / 200;
		}
		else {
			error_pos_y = 360;
			back_alpha = 0.8;
			error_mode = 3;
		}
		return;
	}elif(error_mode == 3) {
		if (OK_shape.mouseOver())  Cursor::RequestStyle(CursorStyle::Hand);
		if (Yes_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		//タイトルに戻る
		if (OK_shape.leftClicked() || Yes_shape.leftClicked()) {
			cancel_sound.playOneShot();
			getData().before_scene = State::Matching;
			changeScene(State::Title, 0.8s);
		}
		return;
	}

	//確定したら変更不可
	if (!getData().decided_character) {
		//ホバーしたらカーソルを変える
		if (isSelectable(0) && select_char_shape1.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (isSelectable(1) && select_char_shape2.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (isSelectable(2) && select_char_shape3.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (isSelectable(3) && select_char_shape4.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (random_select_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (isSettingImageHovered = setting_shape.mouseOver())
			Cursor::RequestStyle(CursorStyle::Hand);
		if (isDecideImageHovered = decide_button_shape.mouseOver())
			Cursor::RequestStyle(CursorStyle::Hand);

		//設定
		if (setting_shape.leftClicked()) {
			getData().before_scene = State::Matching;
			changeScene(State::Calibration, 0.5s);
		}

		//キャラ選択
		if (isSelectable(0) && select_char_shape1.leftClicked()) {
			if (character_number != 0) {
				click_sound.playOneShot();
				character_number = 0;
				character_changed = true;
			}
		}
		if (isSelectable(1) && select_char_shape2.leftClicked()) {
			if (character_number != 1) {
				click_sound.playOneShot();
				character_number = 1;
				character_changed = true;
			}
		}
		if (isSelectable(2) && select_char_shape3.leftClicked()) {
			if (character_number != 2) {
				click_sound.playOneShot();
				character_number = 2;
				character_changed = true;
			}
		}
		if (isSelectable(3) && select_char_shape4.leftClicked()) {
			if (character_number != 3) {
				click_sound.playOneShot();
				character_number = 3;
				character_changed = true;
			}
		}
		// キーボードやコントローラーでも選択できるように
		const Directions directions = PressedDirections();
		const bool confirm = KeyEnter.pressed() || ControllerFaceButtonPressed();
		const bool left_down = directions.left && !previous_directions.left;
		const bool right_down = directions.right && !previous_directions.right;
		const bool confirm_down = confirm && !previous_confirm;
		previous_directions = directions;
		previous_confirm = confirm;
		if (left_down) {
			click_sound.playOneShot();
			stepCharacter(-1);
		}
		if (right_down) {
			click_sound.playOneShot();
			stepCharacter(1);
		}

		//キャラ確定
		if (decide_button_shape.leftClicked() || confirm_down) {
			decision_sound.playOneShot();
			getData().decided_character = true;
		}
		decide_button_size = 1.0 + 0.02 * sin(0.0005 * M_PI * (double)Time::GetMillisec());
	}

	//ホバーしたらカーソルを変える
	if (!vs_cpu && RoomID_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (isReturnImageHovered = return_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);

	//戻る
	if (return_shape.leftClicked()) {
		cancel_sound.playOneShot();
		if (getData().decided_character)getData().decided_character = false;
		getData().before_scene = State::Matching;
		changeScene(State::Title, 0.8s);
	}

	//部屋IDをコピー
	if (!vs_cpu && RoomID_shape.leftClicked()) {
		Clipboard::SetText(Unicode::FromUTF8(room_ID));
		copied_se.playOneShot();
		copy_mode = 1;
		copy_pos_y = -30;
		copy_timer = (int)Time::GetMillisec();
	}
	if (!getData().decided_character && random_select_shape.leftClicked()) {
		Array<int> selectable_numbers;
		for (int i = 0; i < 4; i++) {
			if (isSelectable(i)) selectable_numbers << i;
		}
		int tmp_character_number = selectable_numbers.choice();
		if (character_number != tmp_character_number) {
			click_sound.playOneShot();
			character_number = tmp_character_number;
			character_changed = true;
		}
	}

	//copiedアニメーション
	int now_time = (int)Time::GetMillisec();
	if (copy_mode == 1) {
		double now_rate = (now_time - copy_timer) / 100.0;
		if (now_rate > 1.0) {
			copy_pos_y = 50;
			copy_mode = 2;
			copy_timer = now_time;
		}
		copy_pos_y = (int)(-30.0 + EaseOutExpo(now_rate) * 80.0);
	}elif(copy_mode == 2) {
		if (now_time - copy_timer > 1500) {
			copy_mode = 3;
			copy_timer = now_time;
		}
	}elif(copy_mode == 3) {
		double now_rate = (now_time - copy_timer) / 100.0;
		if (now_rate > 1.0) {
			copy_pos_y = -30;
			copy_mode = 0;
		}
		copy_pos_y = (int)(50.0 - EaseInExpo(now_rate) * 80.0);
	}
	if (!vs_cpu) remaining_time = CalcRemainingTime();
	//通信
	updateRoom();
}

void Matching::draw() const
{
	background_img.draw(0, 0);
	//キャラの立ち絵の表示
	if (is_owner) {
		you_img.drawAt(360, 60);
		character_glow[character_number].drawAt(true, { 360, 540 }, Palette::Silver);
		if (opponent_present) stand_char_img[opponent_character_number].mirrored().drawAt(1560, 540);
	}
	else {
		you_img.drawAt(1560, 60);
		character_glow[character_number].drawAt(true, { 1560, 540 }, Palette::Silver, 1.0, true);
		if (opponent_present) stand_char_img[opponent_character_number].drawAt(360, 540);
	}
	//キミに決めた！
	if (getData().decided_character) {
		fixed_img.drawAt(960, 540);
	}
	else {
		decide_button_glow.drawAt(isDecideImageHovered, { 960, 540 }, Palette::White, decide_button_size);
	}
	//相手が確定したら表示
	if (opponent_present && opponent_decided) {
		decided_img.drawAt(is_owner ? 1560 : 360, 60);
	}

	//各種ボタンの表示
	return_glow.draw(isReturnImageHovered, { 20, 20 });
# define draw_select_char_img(i,x,y,color) \
	select_char_glow[i-1].draw((character_number == i-1),{x,y}, color);

	draw_select_char_img(1, 230, 720, Palette::Blueviolet);
	draw_select_char_img(2, 540, 720, Palette::Orangered);
	random_select_img.draw(850, 720);
	draw_select_char_img(3, 1035, 720, Palette::Yellowgreen);
	draw_select_char_img(4, 1345, 720, Palette::Dodgerblue);
# undef draw_select_char_img
	for (auto&& [i, shape] : std::views::enumerate(std::array{ select_char_shape1, select_char_shape2, select_char_shape3, select_char_shape4 })) {
		if (!isSelectable(static_cast<int>(i))) shape.draw(ColorF{ 0.1, 0.85 });
	}
	if (getData().decided_character) {
		disabled_setting_img.drawAt(1852, 68);
	}
	else {
		setting_glow.drawAt(isSettingImageHovered, { 1852, 68 });
	}

	//ルームIDを表示
	if (!vs_cpu) {
		RoomID_shape.draw(Palette::Black);
		font(Unicode::FromUTF8(room_ID)).drawAt(960, 70, Palette::White);
	}
	//残り時間
	if (!vs_cpu) {
		timer_shape.draw(Palette::White);
		font2(remaining_time).drawAt(960, 1000, Palette::Red);
	}
	//コピー通知
	if (copy_mode)copied_img.drawAt(960, copy_pos_y);
	//通信中
	if (joining.isValid() || (start_sent && !vs_cpu))connecting_img.drawAt(1500, 950);

	//エラーダイアログ
	if (error_mode)drawErrorDialog();
}

void Matching::drawErrorDialog() const
{
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, back_alpha });
	if (error_ID == 0) {
		error_img.drawAt(960, error_pos_y);
	}elif(error_ID == 1) {
		not_found_img.drawAt(960, error_pos_y);
	}elif(error_ID == 2) {
		not_found_img2.drawAt(960, error_pos_y);
	}elif(error_ID == 3) {
		error400_img.drawAt(960, error_pos_y);
	}elif(error_ID == 4) {
		error500_img.drawAt(960, error_pos_y);
	}elif(error_ID == 5) {
		suneo_img.drawAt(960, error_pos_y);
	}elif(error_ID == 6) {
		timeout_img.drawAt(960, error_pos_y);
	}
}


void Matching::drawFadeIn(double t) const
{
	if (!bgm.isPlaying()) bgm.play();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, 1.0 - t });
	if (!vs_cpu) connecting_img.drawAt(1500, 950, ColorF{ 1, 1.0 - t });
}

void Matching::updateFadeOut(double)
{
	//Game の開始まで同期を止めない
	if (getData().room) getData().room->update();
}

void Matching::drawFadeOut(double t) const
{
	if (bgm.isPlaying()) bgm.stop();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, t });
	if (gotoGame && !vs_cpu)connecting_img.drawAt(1500, 950, ColorF{ 1, t });
}
