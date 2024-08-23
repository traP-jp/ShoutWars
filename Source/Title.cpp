//XXX:最後にやれ！！
#include "Title.hpp"

Title::Title(const InitData& init) : IScene(init)
{
}

void Title::update()
{
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
		getData().room_mode = 1;
		//TODO:効果音を流してルームイン！
		changeScene(State::Matching);
	}

}

void Title::draw() const
{
	background_img.draw(0, 0);
	button1_img.draw(390, 500);
	button2_img.draw(1190, 500);
}

void Title::drawFadeIn(double t) const
{
}

void Title::drawFadeOut(double t) const
{
}
