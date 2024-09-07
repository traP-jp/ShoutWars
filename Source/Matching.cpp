//FIXME:ユーザが途中退室すると確実にバグる
#include "Matching.hpp"
# include "common_function.hpp"
using namespace std;

Matching::Matching(const InitData& init) : IScene(init)
{
	//資格を満たしているかどうか
	if (!getData().phoneme.isMFCCUnset()) {
		error_ID = 7;
		error_mode = 1;
	}
	
	//通信関連の処理
	if (getData().before_scene != State::Calibration) {
		//鯖との接続を確立する
		//todo:urlとかはテキストに書く
		const auto api = std::make_shared<APIClient>(U"0.0", U"https://shoutwars.trap.games/api/v2");
		const auto status = api->fetchServerStatus().get();

		try {
			//部屋を作る
			if (getData().room_mode == 0) {
				getData().client = SyncClient::createRoom(api, U"Owner").get();
				getData().room_ID =(Parse<String>(getData().client->roomName.str())).narrow();
				getData().timer = (int)Time::GetSec() - 1;
				is_owner = true;
				//部屋に入る
			}else {
                getData().client = SyncClient::joinRoom(api, Unicode::Widen(getData().room_ID), U"Guest").get();
				//既に決定している場合、反映する
				//ただし、現時点では２人プレイのみを想定
				if ((getData().client)->roomInfo.player.size()) {
					opponent_decided = (getData().client)->roomInfo.is_ready.at(0);
					opponent_character_number = (getData().client)->roomInfo.character.at(0);
				}
			}
		}
		catch (const APIClient::HTTPError& error) {
			setErrorMessage(FromEnum(error.statusCode), error.what());
		}
		//TODO:エラーダイアログに変える
		catch (const Error& error) {
			Print << U"INTERNAL ERROR:" << error.what();
			OutputLogFile("(INTERNAL ERROR)\n"+error.what().narrow());
		}
	}
	room_ID = getData().room_ID;
}

void Matching::setErrorMessage(int error_code,String message)
{
	if (error_code == 404) {
		string error_message = message.narrow();
		if (error_message.find("version") != string::npos) {
			error_ID = 3;
		}
		else {
			error_ID = (getData().room_ID == "114514") ? 2 : 1;
		}
		error_mode = 1;
	}elif(error_code == 403) {
		error_ID = 5;
		error_mode = 1;
	}elif(error_code == 400) {
		error_ID = 3;
		error_mode = 1;
	}elif(error_code == 500) {
		error_ID = 4;
		error_mode = 1;
	//TODO:完全なエラーダイアログに変える
	}else {
		Print << U"[SERVER ERROR:" << error_code << U"] " << message;
		OutputLogFile("(SERVER ERROR:CODE [" +to_string(error_code) + "])\n" + message.narrow());
	}
	return;
}

void Matching::syncRoomInfo()
{
	try {
		//自分のキャラ変更を伝える
		if (character_changed) {
			getData().client->sendReport(U"character", character_number);
			character_changed = false;
		}
		//(鯖主の場合)残り時間を伝える
		if (is_owner&&(old_remaining_time != remaining_time)&&(member_sum != (getData().client->getUsers()).size())) {
			getData().client->sendReport(U"ElapsedTime", ((int)Time::GetSec() - getData().timer));
			member_sum = (getData().client->getUsers()).size();
		}
		//鯖主以外は整合性の確認を問い合わせる
		if ((!is_owner) && (opponent_decided && getData().decided_character) && (!confirm_accuracy)) {
			//整合性確認用の数値を送信
			confirm_num = character_number + opponent_character_number * 10;
			getData().client->sendReport(U"ConfirmAccuracy", confirm_num);
			confirm_accuracy = true;
		}
		//同期
		getData().client->update();
		//相手の変更を取得
		while (const auto event = getData().client->receiveReport()) {
			//相手がキャラを変更したら更新
			if (event->type == U"character") {
				opponent_character_number = event->data.get<int>();
			}
			//(鯖主じゃない場合)鯖主の待機時間を取得
			if ((!is_owner) &&(!recieved_time)&& (event->type == U"ElapsedTime")) {
				getData().timer = (int)Time::GetSec() - (event->data.get<int>());
				recieved_time = true;
			}
			//相手のキャラが確定！
			if (event->type == U"decided") {
				decision_sound.playOneShot();
				opponent_character_number = event->data.get<int>();
				opponent_decided = true;
			}
			//整合性の確認(鯖主)
			if (is_owner && (event->type == U"ConfirmAccuracy")) {
				confirm_num = character_number * 10 + opponent_character_number;
				//整合性の確認が取れたらそのことを伝える
				if (event->data.get<int>() == confirm_num) {
					//問題なければゲーム画面への移行許可を発報してゲーム画面へ
					getData().client->sendReport(U"IsOK",true);
				}else {
					//整合性に問題があれば鯖側で勝手に予測して押し付けてゲーム画面へ
					getData().client->sendReport(U"IsOK", false);
					opponent_character_number = event->data.get<int>() % 10;
					getData().client->sendReport(U"AcuurateData", character_number);
					OutputLogFile("整合性の確認が取れませんでした。\n鯖:" + to_string(confirm_num) + ",ユーザー:"+to_string(event->data.get<int>()));
				}
				//ガラガラ閉店
				getData().client->sendStart();
				gotoGame = true;
				getData().player[0] = character_number;
				getData().player[1] = opponent_character_number;
				getData().before_scene = State::Matching;
				changeScene(State::Game, 0.8s);
			}
			//整合性の確認(鯖主以外)
			if (confirm_accuracy && (event->type == U"IsOK")) {
				if (event->data.get<bool>()) {
					//問題なければゲーム画面へ
					gotoGame = true;
					getData().player[0] = opponent_character_number;
					getData().player[1] = character_number;
					getData().before_scene = State::Matching;
					changeScene(State::Game, 0.8s);
				}
			}
			//整合性の確認が取れなかったら押し付けられたやつを渋々使う(鯖主以外)
			if (confirm_accuracy && (event->type == U"AcuurateData")) {
				opponent_character_number = event->data.get<int>();
				gotoGame = true;
				getData().player[0] = opponent_character_number;
				getData().player[1] = character_number;
				getData().before_scene = State::Matching;
				changeScene(State::Game, 0.8s);
			}
		}
	}
	catch (const APIClient::HTTPError& error) {
		setErrorMessage(FromEnum(error.statusCode), error.what());
	}
	catch (const Error& error) {
		Print <<  error.what();
		OutputLogFile("(INTERNAL ERROR)\n" + error.what().narrow());
	}
}

String Matching::CalcRemainingTime()
{
	//制限時間は10分
	int remaining_int_time = 600-((int)Time::GetSec() - getData().timer);
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
    return Unicode::Widen(minute+":"+second);
}

void Matching::update()
{
	//エラーダイアログ
	if (error_mode == 1) {
		error_timer = (int)Time::GetMillisec();
		error_mode = 2;
		return;
	}elif (error_mode == 2) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - error_timer <= 200) {
			error_pos_y = 1400 - 1040 * (now_time - error_timer) / 200;
			back_alpha = 0.8 * (now_time - error_timer) / 200;
		}else {
			error_pos_y = 360;
			back_alpha = 0.8;
			error_mode = 3;
		}
		return;
	}elif(error_mode == 3) {
		if (OK_shape.mouseOver())  Cursor::RequestStyle(CursorStyle::Hand);
		if (Yes_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		//タイトルに戻る
		if (OK_shape.leftClicked()|| Yes_shape.leftClicked()) {
			cancel_sound.playOneShot();
			getData().before_scene = State::Matching;
			changeScene(State::Title, 0.8s);
		}
		return;
	}

	//確定したら変更不可
	if (!getData().decided_character) {
		//ホバーしたらカーソルを変える
		if (select_char_shape1.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (select_char_shape2.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (select_char_shape3.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (select_char_shape4.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		if (random_select_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (decide_button_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (setting_shape.mouseOver())      Cursor::RequestStyle(CursorStyle::Hand);

		//設定
		if (setting_shape.leftClicked()) {
			getData().before_scene = State::Matching;
			changeScene(State::Calibration, 0.5s);
		}

		//キャラ選択
		if (select_char_shape1.leftClicked()) {
			if (character_number != 0) {
				click_sound.playOneShot();
				character_number = 0;
				character_changed = true;
			}
		}
		if (select_char_shape2.leftClicked()) {
			if (character_number != 1) {
				click_sound.playOneShot();
				character_number = 1;
				character_changed = true;
			}
		}
		if (select_char_shape3.leftClicked()) {
			if (character_number != 2) {
				click_sound.playOneShot();
				character_number = 2;
				character_changed = true;
			}
		}
		if (select_char_shape4.leftClicked()) {
			if (character_number != 3) {
				click_sound.playOneShot();
				character_number = 3;
				character_changed = true;
			}
		}
		//キャラ確定
		if (decide_button_shape.leftClicked()) {
			decision_sound.playOneShot();
			getData().decided_character = true;
			getData().client->sendReport(U"decided", character_number);
			//RoomInfoに自分の情報を追加
			(getData().client->roomInfo.player).push_back(getData().client->userId.str());
			(getData().client->roomInfo.character).push_back(character_number);
			(getData().client->roomInfo.is_ready).push_back(true);
		}
		decide_button_size = 1.0 + 0.02 * sin(0.0005 * M_PI * (double)Time::GetMillisec());
	}

	//ホバーしたらカーソルを変える
	if (RoomID_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (return_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);

	//戻る
	if (return_shape.leftClicked()) {
		cancel_sound.playOneShot();
		if (getData().decided_character)getData().decided_character = false;
		getData().before_scene = State::Matching;
		changeScene(State::Title, 0.8s);
	}

	//部屋IDをコピー
	if (RoomID_shape.leftClicked()) {
		Clipboard::SetText(Unicode::FromUTF8(room_ID));
		copied_se.playOneShot();
		copy_mode = 1;
		copy_pos_y = -30;
		copy_timer = (int)Time::GetMillisec();
	}
	if (random_select_shape.leftClicked()) {
		int tmp_character_number = Random(0, 3);
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
		copy_pos_y = (int)(- 30.0 + EaseOutExpo(now_rate) * 80.0);
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
	if (recieved_time || is_owner)remaining_time = CalcRemainingTime();
	//通信
	syncRoomInfo();
}

void Matching::draw() const
{
	background_img.draw(0, 0);
	//キャラの立ち絵の表示
	if (is_owner) {
		you_img.drawAt(360, 60);
		stand_char_img[character_number].drawAt(360, 540);
		stand_char_img[opponent_character_number].mirrored().drawAt(1560, 540);
	}else {
		you_img.drawAt(1560, 60);
		stand_char_img[character_number].mirrored().drawAt(1560, 540);
		stand_char_img[opponent_character_number].drawAt(360, 540);
	}
	//キミに決めた！
	if (getData().decided_character) {
		fixed_img.drawAt(960, 540);
	}else {
		decide_img.scaled(decide_button_size).drawAt(960, 540);
	}
	//相手が確定したら表示
	if (opponent_decided) {
		decided_img.drawAt(is_owner?1560:360, 60);
	}

	//各種ボタンの表示
	return_img.draw(20, 20);
	select_char_img1.draw(230, 720);
	select_char_img2.draw(540, 720);
	random_select_img.draw(850, 720);
	select_char_img3.draw(1035, 720);
	select_char_img4.draw(1345, 720);
	if (getData().decided_character) {
		disabled_setting_img.drawAt(1852, 68);
	}else {
		setting_img.drawAt(1852, 68);
	}

	//ルームIDを表示
	RoomID_shape.draw(Palette::Black);
	font(Unicode::FromUTF8(room_ID)).drawAt(960, 70, Palette::White);
	//残り時間
	timer_shape.draw(Palette::White);
	font2(remaining_time).drawAt(960, 1000, Palette::Red);
	//コピー通知
	if (copy_mode)copied_img.drawAt(960, copy_pos_y);
	//通信中
	if (confirm_accuracy)connecting_img.drawAt(1500, 950);

	//エラーダイアログ
	if (error_mode)drawErrorDialog();
}

void Matching::drawErrorDialog() const
{
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, back_alpha });
	if (error_ID == 0) {
		error_img.drawAt(960, error_pos_y);
	}elif (error_ID == 1) {
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
	}elif(error_ID == 7) {
		calibration_img.drawAt(960, error_pos_y);
	}
}


void Matching::drawFadeIn(double t) const
{
	if (!bgm.isPlaying()) bgm.play();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, 1.0 - t});
	connecting_img.drawAt(1500, 950, ColorF{ 1, 1.0 - t});
}

void Matching::drawFadeOut(double t) const
{
	if (bgm.isPlaying()) bgm.stop();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, t});
	if (gotoGame)connecting_img.drawAt(1500, 950, ColorF{ 1, t });
}
