﻿#include "Matching.hpp"
# include "common_function.hpp"
using namespace std;

//#define dont_connect

Matching::Matching(const InitData& init) : IScene(init)
{
#ifndef dont_connect
	if (getData().before_scene != State::Config) {
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
				//INFO:現時点では2人プレイのみを想定
				map user_map = getData().client->getUsers();
				//定員オーバー
				if (user_map.size() > 2) {
					error_ID = 5;
					error_mode = 1;
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
#else
	room_ID = "123456";
#endif
}

//TODO:エラーダイアログに変える
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
	}elif(error_code == 400) {
		error_ID = 3;
		error_mode = 1;
	}elif(error_code == 500) {
		error_ID = 4;
		error_mode = 1;
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
			getData().client->sendAction(U"character", character_number);
		}
		//残り時間を伝える
		if (is_owner&&(old_remaining_time != remaining_time)) {
			getData().client->sendAction(U"RemaingTime", remaining_time);
			old_remaining_time = remaining_time;
		}
		//同期
		getData().client->update();
		//相手の変更を取得
		while (const auto event = getData().client->receiveReport()) {
			//相手がキャラを変更したら更新
			if (event->type == U"character") {
				opponent_character_number = event->data.get<int>();
			}
			//(鯖主じゃない場合)残り時間を取得
			if ((!is_owner) && (event->type == U"RemainTime")) {
				remaining_time = event->data.get<String>();
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
		getData().before_scene = State::Matching;
		changeScene(State::Title,0.8s);
	}

	//設定
	if (setting_shape.leftClicked()) {
		getData().before_scene = State::Matching;
		changeScene(State::Config, 0.5s);
	}

	//キャラ選択
	if (select_char_shape1.leftClicked()) {
		if (character_number != 0) {
			character_number = 0;
			character_changed = true;
		}
	}
	if (select_char_shape2.leftClicked()) {
		if (character_number != 1) {
			character_number = 1;
			character_changed = true;
		}
	}
	if (select_char_shape3.leftClicked()) {
		if (character_number != 2) {
			character_number = 2;
			character_changed = true;
		}
	}
	if (select_char_shape4.leftClicked()) {
		if (character_number != 3) {
			character_number = 3;
			character_changed = true;
		}
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
			character_number = tmp_character_number;
			character_changed = true;
		}
		//TODO:後で消す
		getData().before_scene = State::Matching;
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
	//通信
	syncRoomInfo();
	remaining_time = CalcRemainingTime();
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
	font(Unicode::FromUTF8(room_ID)).drawAt(960, 70, Palette::White);
	//残り時間
	timer_shape.draw(Palette::White);
	font2(remaining_time).drawAt(960, 1000, Palette::Red);
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
