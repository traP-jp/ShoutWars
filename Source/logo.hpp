# pragma once
# include "common.hpp"

class logo : public App::Scene
{
	const Texture fade_img{ U"../images/Siv3D_logo.png" };
	int logo_timer = 0;
	bool first_time = true;
public:
	logo(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
