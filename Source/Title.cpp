//INFO:このファイルではusing namespace std;禁止
#include "Title.hpp"

Title::Title(const InitData& init) : IScene(init)
{
	//タイトル画面でほぼすべてを初期化
	getData().decided_character = false;
	getData().timer = 0;
	getData().room_mode = 0;

	//電卓のボタンの当たり判定を作成
	shape_of_number[1] = Rect{ 660+55 ,140+347,150,80 };
	shape_of_number[2] = Rect{ 660+225,140+347,150,80 };
	shape_of_number[3] = Rect{ 660+395,140+347,150,80 };
	shape_of_number[4] = Rect{ 660+55 ,140+445,150,80 };
	shape_of_number[5] = Rect{ 660+225,140+445,150,80 };
	shape_of_number[6] = Rect{ 660+395,140+445,150,80 };
	shape_of_number[7] = Rect{ 660+55 ,140+543,150,80 };
	shape_of_number[8] = Rect{ 660+225,140+543,150,80 };
	shape_of_number[9] = Rect{ 660+395,140+543,150,80 };
	shape_of_number[0] = Rect{ 660+225,140+641,150,80 };
}

void Title::update()
{
	if (calc_mode == 0) {
		if (button1_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (button2_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (setting_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
		if (button1_shape.leftClicked()) {
			getData().room_mode = 0;
			decision_sound.playOneShot();
			getData().before_scene = State::Title;
			changeScene(State::Matching,0.8s);
		}
		if (setting_shape.leftClicked()) {
			click_sound.playOneShot();
			setting_flag = true;
			getData().before_scene = State::Title;
			changeScene(State::Calibration, 0.5s);
		}
		//電卓出現
		if (button2_shape.leftClicked()) {
			click_sound.playOneShot();
			calc_mode = 1;
			animation_timer = (int)Time::GetMillisec();
			animation_y = 1480;
			back_alpha = 0.0;
		}
	}elif(calc_mode == 1) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - animation_timer <= 400) {
			animation_y = 1480 - 940 * (now_time - animation_timer) / 400;
			back_alpha = 0.8 * (now_time - animation_timer) / 400;
		}else {
			animation_y = 540;
			back_alpha = 0.8;
			calc_mode = 2;
			clip_flag = true;
		}
	}elif(calc_mode == 2) {
		//範囲外をクリックしたら戻る
		if (MouseL.down()) {
			if ((Cursor::Pos().x < 660) || (Cursor::Pos().x > 1260) || (Cursor::Pos().y < 140) || (Cursor::Pos().y > 940)) {
				calc_mode = 3;
				animation_timer = (int)Time::GetMillisec();
			}
		}

		//数字のマウス入力
		for (int i = 0; i < 10; i++) {
			if ((shape_of_number[i].mouseOver()) && (room_ID_digit < 6)) {
				Cursor::RequestStyle(CursorStyle::Hand);
			}
			if (shape_of_number[i].leftClicked()) {
				if (room_ID_digit < 6) {
					click_number_sound.playOneShot();
					room_ID += std::to_string(i);
					room_ID_digit++;
				}
			}
		}
		//数字のキー入力
		int key = key_num();
		if (key != -1) {
			if (room_ID_digit < 6) {
				click_number_sound.playOneShot();
				room_ID += std::to_string(key);
				room_ID_digit++;
			}
		}
		//クリップボードから自動入力
		if (!clip_flag)clip_flag = Clipboard::HasChanged();
		String clip;
		if (clip_flag && Clipboard::GetText(clip) && (std::all_of(clip.begin(),clip.end(),isdigit))) {
			if (clip.size() == 6) {
				room_ID = clip.narrow();
				room_ID_digit = 6;
				clip_flag = false;
			}
		}

		//カーソルの形状変更
		if ((cancel_shape.mouseOver()) && (room_ID_digit > 0)) {
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		if ((decide_shape.mouseOver())&&(room_ID_digit == 6)) {
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		//消去
		if (cancel_shape.leftClicked() || KeyBackspace.down()) {
			if (room_ID_digit > 0) {
				choice_sound.playOneShot();
				room_ID.pop_back();
				room_ID_digit--;
			}
		}
		//確認
		if ((decide_shape.leftClicked()||KeyEnter.down()) && (room_ID_digit == 6)) {
			getData().room_mode = 1;
			getData().room_ID = room_ID;
			decision_sound.playOneShot();
			getData().before_scene = State::Title;
			changeScene(State::Matching,0.8s);
		}
	}elif(calc_mode == 3) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - animation_timer <= 200) {
			animation_y = 540 + 940 * (now_time - animation_timer) / 200;
			back_alpha = 0.8 - 0.8 * (now_time - animation_timer) / 200;
		}else {
			animation_y = 1480;
			back_alpha = 0.0;
			calc_mode = 0;
			room_ID = "";
			room_ID_digit = 0;
		}
	}
}

int Title::key_num()
{
	if (Key1.down()) return 1;
	if (Key2.down()) return 2;
	if (Key3.down()) return 3;
	if (Key4.down()) return 4;
	if (Key5.down()) return 5;
	if (Key6.down()) return 6;
	if (Key7.down()) return 7;
	if (Key8.down()) return 8;
	if (Key9.down()) return 9;
	if (Key0.down()) return 0;
	if (KeyNum0.down()) return 0;
	if (KeyNum1.down()) return 1;
	if (KeyNum2.down()) return 2;
	if (KeyNum3.down()) return 3;
	if (KeyNum4.down()) return 4;
	if (KeyNum5.down()) return 5;
	if (KeyNum6.down()) return 6;
	if (KeyNum7.down()) return 7;
	if (KeyNum8.down()) return 8;
	if (KeyNum9.down()) return 9;
	return -1;
}

void Title::draw() const
{
	background_img.draw(0, 0);
	button1_img.draw(390, 500);
	button2_img.draw(1190, 500);
	setting_img.drawAt(1852, 68);
	if (calc_mode) {
		Rect(0,0,1920,1080).draw(ColorF{0,back_alpha});
		calc_img.drawAt(960, animation_y);
		if (calc_mode == 2) {
			//数字の表示
			font(Unicode::FromUTF8(room_ID)).drawAt(660+300, 140+250, Palette::Black);
		}
	}
}

void Title::drawFadeIn(double t) const
{
	if (!bgm.isPlaying())bgm.play();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0,1.0-t});
}

void Title::drawFadeOut(double t) const
{
	if (bgm.isPlaying()) bgm.stop();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0,t });
	if (!setting_flag)connecting_img.drawAt(1500, 950,ColorF{1,t });
}
