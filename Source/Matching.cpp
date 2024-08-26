//TODO:ネット関連のものが完成したら実装しろ

#include "Matching.hpp"

Matching::Matching(const InitData& init) : IScene(init)
{
	//TODO:ネット関連のものが完成したら実装しろ
}

void Matching::update()
{
	//ホバーしたらカーソルを変える
	if (select_char_shape1.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape2.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape3.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (select_char_shape4.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
	if (random_select_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);

	if (return_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);

	//戻る
	if (return_shape.leftClicked()) {
		changeScene(State::Title,0.8s);
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
	if (random_select_shape.leftClicked()) {
		character_number = Random(0, 3);
		//TODO:後で消す
		changeScene(State::Game,0.8s);
	}
}

void Matching::draw() const
{
	background_img.draw(0, 0);
	return_img.draw(20, 20);
	select_char_img1.draw(230, 720);
	select_char_img2.draw(540, 720);
	random_select_img.draw(850, 720);
	select_char_img3.draw(1035, 720);
	select_char_img4.draw(1345, 720);
}

void Matching::drawFadeIn(double t) const
{
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF(0, 0, 0, 1.0 - t/0.8));
}
