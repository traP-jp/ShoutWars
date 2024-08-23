# pragma once
# include "common.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

class Game : public App::Scene
{
private:
	//構造体////////////////////////////////////////////////////////////
	struct Player player[2];
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

struct Player{
	//各種変数
	Vec2 pos[2];
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:スタン,16:弱,32:狂,64:特殊,128～:予備)
	int status = 0;
	//HP(0:実質HP,1:表示HP)
	int hp[2] = { 810 };
	int ap = 0;
	double speed = 50.0;
	//Playerに関する時間(0:左右移動,1:予備,2:ジャンプ)
	int timer[10];
	//Playerの向き(true:右,false:左)
	bool direction = false;

	int img_number;
	int img_status;
	int img_timer;
};
