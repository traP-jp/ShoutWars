# pragma once
# include "common.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

struct Player {
	//各種変数
	Vec2 pos[2];
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:スタン,16:弱,32:狂,64:特殊,128～:予備)
	int status = 0;
	//HP(0:実質HP(確定),1:表示HP(未確定),2:表示HP(確定))
	int hp[3] = { 1000,1000,1000 };
	int ap = 0;
	double speed = 50.0;
	//Playerに関する時間(0:左右移動,1:予備,2:ジャンプ,3:スタン,4:弱,5:狂)
	int timer[10];
	//Playerの向き(true:右,false:左)
	bool direction = false;
	//通信が必要なイベント,[0]:イベント(0:なし,1:弱,2:狂,4:),[1]:時間
	int event[2];

	int img_number;
	int img_status;
	int img_timer;
};

class Game : public App::Scene
{
private:
	//構造体////////////////////////////////////////////////////////////
	struct Player player[2];
	//定数////////////////////////////////////////////////////////////
	const static int player_sum = 2;
	const static int player_min_y = 700;
	const static int player_status_sum = 10;
	const static int player_max_hp = 1000;
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/game/background.png"};
	const Texture HP_bar_flame_img{ U"../images/game/HP_bar_flame.png" };
	const Texture HP_bar_gray_img{ U"../images/game/HP_bar_gray.png" };
	const Texture HP_bar_red_img{ U"../images/game/HP_bar_red.png" };
	const Texture HP_bar_blue_img{ U"../images/game/HP_bar_blue.png" };
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

	//通信用の変数////////////////////////////////////////////////////
	int connection_timer = 0;

	//内部関数////////////////////////////////////////////////////////

	//1:上,2:左,4:下,8:右
	int getkey();
	void draw_player() const;
	void draw_HP_bar() const;
	void update_player();
	void update_player_animation();
	void init_connection();
	void synchronizate_data();
	int voice_command();
	inline int sign(bool plus_or_minus) {return plus_or_minus ? 1 : -1;}
public:

	Game(const InitData& init);

	void update() override;

	void drawFadeIn(double t) const override;
	void draw() const override;
};
