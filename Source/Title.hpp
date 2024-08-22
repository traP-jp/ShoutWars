# pragma once
# include "common.hpp"

class Title : public App::Scene
{
private:
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/background.png" };
	const Texture button1_img{ U"../images/button1.png" };
	const Texture button2_img{ U"../images/button2.png" };


public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
