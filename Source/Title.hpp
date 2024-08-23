# pragma once
# include "common.hpp"

class Title : public App::Scene
{
private:
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/title/background.png" };
	const Texture button1_img{ U"../images/title/button1.png" };
	const Texture button2_img{ U"../images/title/button2.png" };

	//shape////////////////////////////////////////////////////////////
	const RectF button1_shape{ 390,500,340,440 };
	const RectF button2_shape{ 1190,500,340,440 };
public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
