# pragma once
# include "common.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

class Game : public App::Scene
{
private:
	//定数////////////////////////////////////////////////////////////
	const static int player_sum = 2;
	const static int player_min_y = 700;
	const static int player_status_sum = 10;
	const static int player_max_hp = 810;
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/background.png"};
	const Texture HP_bar_flame_img{ U"../images/HP_bar_flame.png" };
	const Texture HP_bar_gray_img{ U"../images/HP_bar_gray.png" };
	const Texture HP_bar_red_img{ U"../images/HP_bar_red.png" };
	const Texture HP_bar_blue_img{ U"../images/HP_bar_blue.png" };
	std::vector<std::vector<Texture>> player_img;
	//音楽////////////////////////////////////////////////////////////
	const Audio bgm{ U"../audioes/Es-Boss3_loop.ogg" , Arg::loopBegin = 28.848843537415s};

	//変数////////////////////////////////////////////////////////////
	//プレイヤーが存在するか
	std::vector<bool> player_flag;
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:スタン,16:弱,32:狂,64:特殊,128～:予備)
	int player_status[player_sum] = {0};
	//プレイヤーの画像のアニメーション用
	int player_img_number[player_sum] = { 0 };
	int player_img_status[player_sum] = { 0 };
	int player_img_timer[player_sum] = { 0 };

	//HP(0:実質HP,1:表示HP)
	std::vector<std::vector<int>> player_hp;
	//AP
	int player_ap[player_sum] = { 10 };
	Vec2 player_pos[player_sum][2];
	//移動速度
	double player_speed[player_sum] = {50.0};
	//Playerに関する時間(0:左右移動,1:予備,2:ジャンプ)
	int player_timer[player_sum][player_status_sum] = {0};
	//Playerの向き(true:右,false:左)
	bool player_direction[player_sum] = {false};
	//時間
	int internal_timer = 0;
	//bgmが流れているか
	bool bgm_sw = false;

	//内部関数////////////////////////////////////////////////////////

	//1:上,2:左,4:下,8:右
	int getkey();


	void draw_player() const;
	void draw_HP_bar() const;
	void update_player();
	void update_player_animation();
	int voice_command();
public:

	Game(const InitData& init);

	void update() override;

	void drawFadeIn(double t) const override;
	void draw() const override;
};
