#include "Matching.hpp"
using namespace std;

#define dont_connect

Matching::Matching(const InitData& init) : IScene(init)
{
#ifndef dont_connect
	//鯖との接続を確立する
	//todo:urlとかはテキストに書く
	const auto api = std::make_shared<APIClient>(U"0.0", U"https://shoutwars.trap.games/api/v2");
	const auto status = api->fetchServerStatus().get();

	try {
		//部屋を作る
		if (getData().room_mode == 0) {
			getData().client = SyncClient::createRoom(api, U"Owner").get();
			getData().room_ID = Parse<int>(getData().client->roomName.str());
			//部屋に入る
		}else{
			getData().client = SyncClient::joinRoom(api, Format(getData().room_ID), U"Guest").get();
		}
	}
	//TODO:エラーダイアログに変える
	catch (const APIClient::HTTPError& error) {
		if (FromEnum(error.statusCode) == 404) {
			error_mode = 1;
		}else {
			Print << U"1 [" << FromEnum(error.statusCode) << U"] " << error.what();
		}
	}
	catch (const Error& error) {
		Print << error.what();
	}

	room_ID = to_string(getData().room_ID);
#else
	room_ID = "123456";
#endif
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
			changeScene(State::Title, 0.8s);
		}
		return;
	}

	//ホバーしたらカーソルを変える
	if (select_char_shape1.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape2.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape3.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape4.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (random_select_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
	if (RoomID_shape.mouseOver())       Cursor::RequestStyle(CursorStyle::Hand);
	if (return_shape.mouseOver())       Cursor::RequestStyle(CursorStyle::Hand);
	if (setting_shape.mouseOver())      Cursor::RequestStyle(CursorStyle::Hand);

	//戻る
	if (return_shape.leftClicked()) {
		cancel_sound.playOneShot();
		changeScene(State::Title,0.8s);
	}

	//設定
	if (setting_shape.leftClicked()) {
		getData().before_scene = State::Matching;
		changeScene(State::Config, 0.5s);
	}

	//キャラ選択
	if (select_char_shape1.leftClicked()) {
		character_number = 0;
	}
	if (select_char_shape2.leftClicked()) {
		character_number = 1;
	}
	if (select_char_shape3.leftClicked()) {
		character_number = 2;
	}
	if (select_char_shape4.leftClicked()) {
		character_number = 3;
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
		character_number = Random(0, 3);
		//TODO:後で消す
		changeScene(State::Game,0.8s);
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
		copy_pos_y = -30+EaseOutExpo(now_rate)*80;
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
		copy_pos_y = 50 - EaseInExpo(now_rate) * 80;
	}
}

void Matching::draw() const
{
	background_img.draw(0, 0);
	return_img.draw(20, 20);
	setting_img.drawAt(1852, 68);
	select_char_img1.draw(230, 720);
	select_char_img2.draw(540, 720);
	random_select_img.draw(850, 720);
	select_char_img3.draw(1035, 720);
	select_char_img4.draw(1345, 720);

	//ルームIDを表示
	RoomID_shape.draw(Palette::Black);
	font(Unicode::FromUTF8(room_ID)).draw(800, 30, Palette::White);
	//コピー通知
	if (copy_mode) {
		copied_img.drawAt(960, copy_pos_y);
	}

	//エラーダイアログ
	if (error_mode)drawErrorDialog();
}

void Matching::drawErrorDialog() const
{
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, back_alpha });
	if (room_ID == "114514") {
		not_found_img2.drawAt(960, error_pos_y);
	}else {
		not_found_img.drawAt(960, error_pos_y);
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
}
