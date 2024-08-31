# pragma once
# include "common.hpp"
#include <string>

class Title : public App::Scene
{
private:
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/title/background.png" };
	const Texture button1_img{ U"../images/title/button1.png" };
	const Texture button2_img{ U"../images/title/button2.png" };
	const Texture calc_img{ U"../images/title/calc.png" };
	const Texture connecting_img{ U"../images/common/connecting.png" };

	//音声素材////////////////////////////////////////////////////////
	const Audio bgm{U"../audioes/zun_mgcr.mp3" , Loop::Yes };

	const Audio click_sound{ U"../audioes/click.wav" };
	const Audio click_number_sound{ U"../audioes/click_number.wav" };
	const Audio decision_sound{ U"../audioes/decision.wav" };
	const Audio choice_sound{ U"../audioes/choice.wav" };

	//font////////////////////////////////////////////////////////////
	Font font{ FontMethod::MSDF, 72 };

	//shape////////////////////////////////////////////////////////////
	const RectF button1_shape{ 390,500,340,440 };
	const RectF button2_shape{ 1190,500,340,440 };

	Rect shape_of_number[10];

	const Rect cancel_shape{ 660+55,140+641,150,80 };
	//const RectF shape_of_0{ 225,841,150,80 };
	const Rect decide_shape{ 660+395,140+641,150,80 };
	//内部変数/////////////////////////////////////////////////////////
	//0:通常,1:出現アニメーション,2:表示中,3:消失アニメーション
	int calc_mode = 0;
	int animation_y = 1480;
	int animation_timer = 0;
	double back_alpha = 0.0;
	bool clip_flag = true;

	std::string room_ID;
	int room_ID_digit = 0;

	int key_num();
public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
