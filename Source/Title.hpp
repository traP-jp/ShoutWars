# pragma once
# include "common.hpp"
# include "GlowBorder.hpp"
#include <string>

class Title : public App::Scene
{
private:
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ Resource(U"images/title/background.png") };
	const Texture button_vs_cpu_img{ Resource(U"images/title/button_vs_cpu.png") };
	const Texture button1_img{ Resource(U"images/title/button1.png") };
	const Texture button2_img{ Resource(U"images/title/button2.png") };
	const Texture calc_img{ Resource(U"images/title/calc.png") };
	const Texture connecting_img{ Resource(U"images/common/connecting.png") };
	const Texture setting_img{ Resource(U"images/common/setting.png") };
	const Texture calibration_img{ Resource(U"images/matching/calibration.png") };

	//音声素材////////////////////////////////////////////////////////
	const Audio bgm{ Resource(U"audioes/zun_mgcr.mp3") , Loop::Yes };

	const Audio click_sound{ Resource(U"audioes/click.wav") };
	const Audio click_number_sound{ Resource(U"audioes/click_number.wav") };
	const Audio decision_sound{ Resource(U"audioes/decision.wav") };
	const Audio choice_sound{ Resource(U"audioes/choice.wav") };

	//font////////////////////////////////////////////////////////////
	Font font{ FontMethod::MSDF, 72 };

	//shape////////////////////////////////////////////////////////////
	//左から CPU と対戦、部屋を作る、部屋に入る
	const RectF button_vs_cpu_shape{ 225,500,340,440 };
	const RectF button1_shape{ 790,500,340,440 };
	const RectF button2_shape{ 1355,500,340,440 };

	//境界線の描画用////////////////////////////////////////////////////
	GlowBorder button_vs_cpu_glow;
	GlowBorder button1_glow;
	GlowBorder button2_glow;
	GlowBorder setting_glow;
	bool isButtonVsCpuHovered = false;
	bool isButton1Hovered = false;
	bool isButton2Hovered = false;
	bool isSettingHovered = false;

	Rect shape_of_number[10];

	const Rect cancel_shape{ 660 + 55,140 + 641,150,80 };
	//const RectF shape_of_0{ 225,841,150,80 };
	const Rect decide_shape{ 660 + 395,140 + 641,150,80 };
	const Circle setting_shape{ 1852,68,48 };
	const Rect calibration_OK_shape{ 680,464,240,105 };
	const Rect calibration_Yes_shape{ 1010,464,240,105 };
	//内部変数/////////////////////////////////////////////////////////
	//0:通常,1:出現アニメーション,2:表示中,3:消失アニメーション
	int calc_mode = 0;
	int animation_y = 1480;
	int animation_timer = 0;
	double back_alpha = 0.0;
	bool setting_flag = false;
	//キャリブレーションを促すダイアログ 0:非表示,1:出現アニメーション,2:表示中
	int calibration_dialog_mode = 0;
	int calibration_dialog_timer = 0;
	int calibration_dialog_y = 1400;
	double calibration_back_alpha = 0.0;
	bool clip_flag = true;

	std::string room_ID;
	int room_ID_digit = 0;

	//部屋数の表示とサーバーを起こすため、数秒おきに状況を問い合わせる
	Multiplay::APICall<Multiplay::ServerStatus> status_call;
	Timer status_timer{ 5s };
	String status_text = U"サーバーに接続中…";
	//サーバーに繋がるか (繋がらなければ部屋を作る・入るボタンを押せない)
	bool server_available = false;

	int key_num();
	void updateServerStatus();
	bool requireCalibration();
	void updateCalibrationDialog();
public:
	Title(const InitData& init);

	void update() override;
	void draw() const override;
	void drawFadeIn(double t) const override;
	void drawFadeOut(double t) const override;
};
