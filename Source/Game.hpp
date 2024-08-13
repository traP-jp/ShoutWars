# pragma once
# include "common.hpp"
#include <vector>
#include <cmath>
#include <string>

class Game : public App::Scene
{
private:
	//定数////////////////////////////////////////////////////////////
	const static int player_sum = 2;
	const static int player_min_y = 700;
	const static int player_status_sum = 10;
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/background.png"};
	std::vector<std::vector<Texture>> player_img;
	std::vector<bool> player_flag;
	//変数////////////////////////////////////////////////////////////
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:スタン,16:弱,32:狂,64:特殊,128～:予備)
	int player_status[player_sum] = {0};
	//HP
	int player_hp[player_sum] = {100};
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
	//内部関数////////////////////////////////////////////////////////
	//1:上,2:左,4:下,8:右
	int getkey();
public:

	Game(const InitData& init);

	void update() override;
	void update_player();

	void drawFadeIn(double t) const override;
	void draw() const override;
	void draw_player() const;
};
