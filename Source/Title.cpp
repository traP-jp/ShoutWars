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
		//TODO:効果音を流してルームメイク！
	}
	if (button2_shape.leftClicked()) {
		//TODO:効果音を流してルームイン！
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
