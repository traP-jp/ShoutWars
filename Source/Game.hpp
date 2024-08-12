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
	const Vec2 go_left = {-1.0,0.0};
	const Vec2 go_right = {1.0,0.0};
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/background.png"};
	std::vector<std::vector<Texture>> player_img;
	std::vector<bool> player_flag;
	//変数////////////////////////////////////////////////////////////
	//ステータス(0:待機中,1:左移動,2:右移動,3ジャンプ,4:スタン,5:弱,6:狂,7:特殊,8～:予備)
	int player_status[player_sum] = {0};
	//HP
	int player_hp[player_sum] = {100};
	//AP
	int player_ap[player_sum] = { 10 };
	Vec2 player_pos[player_sum] = { {100,player_min_y},{1200,player_min_y} };
	//移動速度
	double player_speed[player_sum] = {10.0};
	//Playerに関する時間
	int player_timer[player_sum] = {0};
	//時間
	int internal_timer = 0;
	//内部関数////////////////////////////////////////////////////////
	int getkey();
public:

	Game(const InitData& init);

	void update() override;
	void update_player();

	void drawFadeIn(double t) const override;
	void draw() const override;
	void draw_player() const;
};
