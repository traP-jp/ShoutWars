#include "Config.hpp"

Config::Config(const InitData& init) : IScene(init)
{
}

void Config::update()
{
	if (return_shape.mouseOver())Cursor::RequestStyle(CursorStyle::Hand);
	if (return_shape.leftClicked()) {
		cancel_sound.playOneShot();
		changeScene(getData().before_scene, 0.5s);
	}
}

void Config::draw() const
{
	background_img.draw(0, 0);
	return_img.draw(20, 20);
}

void Config::drawFadeIn(double t) const
{
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, t});
}

void Config::drawFadeOut(double t) const
{
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0, t});
}
