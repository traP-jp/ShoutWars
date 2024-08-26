#include "logo.hpp"

logo::logo(const InitData& init) : IScene(init)
{
}

void logo::update()
{
	if (first_time) {
		logo_timer = (int)Time::GetMillisec();
		first_time = false;
	}
	if (Time::GetMillisec()- logo_timer > 1500)changeScene(State::Title,0.8s);
}

void logo::draw() const
{
	fade_img.draw(0, 0);
}

void logo::drawFadeIn(double t) const
{
	fade_img.draw(0, 0, ColorF{ 1.0, t });
}

void logo::drawFadeOut(double t) const
{
	fade_img.draw(0, 0, ColorF{ 1.0, 1.0 - t/0.8 });
}
