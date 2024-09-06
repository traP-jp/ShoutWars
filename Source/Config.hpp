# pragma once
# include "common.hpp"

class Config : public App::Scene
{
	//画像の読み込み////////////////////////////////////////
	const Texture background_img = Texture(U"../images/config/background.png");
	const Texture return_img = Texture(U"../images/common/return.png");

	//shape////////////////////////////////////////////////
	const Rect return_shape{ 20,20,80,80 };

	//音声データの読み込み/////////////////////////////////
	const Audio cancel_sound{ U"../audioes/cancel.wav" };
public:
	Config(const InitData& init);
	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
