//XXX:最後にやれ！！
#include "Title.hpp"
using namespace std;

Title::Title(const InitData& init) : IScene(init)
{
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
		if (button1_shape.mouseOver()) {
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		if (button2_shape.mouseOver()) {
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		if (button1_shape.leftClicked()) {
			getData().room_mode = 0;
			//TODO:効果音を流してルームメイク！
			changeScene(State::Matching);
		}
		if (button2_shape.leftClicked()) {
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
		}
	}elif(calc_mode == 2) {
		//範囲外をクリックしたら戻る
		if (MouseL.down()) {
			if ((Cursor::Pos().x < 660) || (Cursor::Pos().x > 1260) || (Cursor::Pos().y < 140) || (Cursor::Pos().y > 940)) {
				calc_mode = 3;
				animation_timer = (int)Time::GetMillisec();
			}
		}

		//数字の入力
		for (int i = 0; i < 10; i++) {
			if ((shape_of_number[i].mouseOver()) && (room_ID_digit < 6)) {
				Cursor::RequestStyle(CursorStyle::Hand);
			}
			if (shape_of_number[i].leftClicked()) {
				if (room_ID_digit < 6) {
					room_ID += to_string(i);
					room_ID_digit++;
				}
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
		if (cancel_shape.leftClicked()) {
			if (room_ID_digit > 0) {
				room_ID.pop_back();
				room_ID_digit--;
			}
		}
		//確認
		if ((decide_shape.leftClicked()) && (room_ID_digit == 6)) {
			getData().room_mode = 1;
			getData().room_ID = stoi(room_ID.c_str());
			//TODO:効果音を流してルームイン！
			changeScene(State::Matching);
		}
	}elif(calc_mode == 3) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - animation_timer <= 200) {
			animation_y = 540 + 940 * (now_time - animation_timer) / 200;
			back_alpha = 0.8 - 0.8 * (now_time - animation_timer) / 200;
		}
		else {
			animation_y = 1480;
			back_alpha = 0.0;
			calc_mode = 0;
			room_ID = "";
			room_ID_digit = 0;
		}
	}
}

void Title::draw() const
{
	background_img.draw(0, 0);
	button1_img.draw(390, 500);
	button2_img.draw(1190, 500);
	if (calc_mode) {
		Rect(0,0,1920,1080).draw(ColorF{0,back_alpha});
		calc_img.drawAt(960, animation_y);
		if (calc_mode == 2) {
			font(Unicode::FromUTF8(room_ID)).drawAt(660+300, 140+250, Palette::Black);
		}
	}
}

void Title::drawFadeIn(double t) const
{
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0,1.0-t });
}

void Title::drawFadeOut(double t) const
{
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0,t });
}
