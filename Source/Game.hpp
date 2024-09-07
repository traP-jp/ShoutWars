# pragma once
# include "common.hpp"
# include "Voice/WordDetector.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

struct Player {
	//各種変数
	Vec2 pos[2];
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:ガード,16:弱,32:狂,64:必殺,128:ガード破壊,256:特殊攻撃,512～:予備)
	int status = 0;
	int old_status = 0;
	//HP(0:実質HP(確定),1:表示HP(未確定),2:表示HP(確定))
	int hp[3] = { 1000,1000,1000 };
	int ap = 0;
	double speed = 80.0;
	//Playerに関する時間(0:左右移動,1:進捗(0),2:ジャンプ,3:ガード,4:弱,5:狂,6:必殺,7:進捗(1),8:進捗(3),9:進捗(4),10:進捗(5),11:進捗(6),12:ガード破壊,13:進捗(12),14:特殊攻撃,15:進捗(14))
	int timer[16];

	//Playerに関するse(0:左右移動,1:ジャンプ,2:弱,3:狂,4:必殺,5:ガード,6:ガード破壊)
	bool se[7] = { false };
	//Playerの向き(true:右,false:左)
	bool direction = false;
	//1回限りのイベント(0:なし,1:弱,2:狂,4:必殺,8:ガード破壊)
	int event;
	//必殺技が使えるか
	bool special_attack = false;

	int img_number = 0;
	int img_status = 0;
	int img_timer = 0;

	int fire_animation = 0;
	int fire_animation_timer = 0;
};

class Game : public App::Scene
{
private:
	//構造体////////////////////////////////////////////////////////////
	struct Player player[2];
	//定数////////////////////////////////////////////////////////////
	const static int player_sum = 2;
	const static int player_min_y = 650;
	const static int player_max_hp = 1000;
	//技が発動するために必要なAP
	const static int player_max_ap = 500;
	//font////////////////////////////////////////////////////////////
	Font font{ 40 };
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ U"../images/game/system/background.png"};
	const Texture HP_bar_flame_img{ U"../images/game/system/HP_bar_flame.png" };
	const Texture HP_bar_gray_img{ U"../images/game/system/HP_bar_gray.png" };
	const Texture HP_bar_red_img{ U"../images/game/system/HP_bar_red.png" };
	const Texture HP_bar_blue_img{ U"../images/game/system/HP_bar_blue.png" };
	const Texture AP_bar_empty_img{ U"../images/game/system/AP_bar_empty.png" };
	const Texture AP_bar_max_img{ U"../images/game/system/AP_bar_max.png" };
	const Texture AP_bar_img{ U"../images/game/system/AP_bar.png" };
	const Texture destroyed_img{ U"../images/game/system/destroyed.png" };
	const Texture timeout_img{ U"../images/game/system/timeout.png" };
	const Texture connecting_img{ U"../images/common/connecting.png" };
	const Texture guard_img{ U"../images/game/system/guard.png" };
	const Texture ping_fast_img{ U"../images/game/system/ping_fast.png" };
	const Texture ping_middle_img{ U"../images/game/system/ping_middle.png" };
	const Texture ping_slow_img{ U"../images/game/system/ping_slow.png" };
	const Texture you_win_img{ U"../images/game/system/you_win.png" };
	const Texture you_lose_img{ U"../images/game/system/you_lose.png" };
	const Texture settle_img{ U"../images/game/system/settle.png" };
	const Texture command_img{ U"../images/game/system/command.png" };
	std::vector<std::vector<Texture>> player_img;
	std::vector<Texture> fire_img;
	//音楽////////////////////////////////////////////////////////////
	const Audio bgm{ U"../audioes/Es-Boss3_loop.ogg" , Arg::loopBegin = 28.848843537415s};
	const Audio dos_se{ U"../audioes/dos.wav" };
	const Audio dododos_se{ U"../audioes/dododos.wav" };
	const Audio jump_se{ U"../audioes/jump.wav" };
	const Audio shot_se{ U"../audioes/shot.wav" };
	const Audio kiran_se{ U"../audioes/kiran.wav" };
	const Audio bom_se{ U"../audioes/bom.wav" };
	const Audio cancel_sound{ U"../audioes/cancel.wav" };
	const Audio guard_se{ U"../audioes/guard.mp3" };
	const Audio void_damage_se{ U"../audioes/void_damage.mp3" };
	const Audio break_guard_se{ U"../audioes/break_guard.wav" };
	//shape////////////////////////////////////////////////////////////
	const Rect OK_shape{ 680,464,240,105 };
	const Rect Yes_shape{ 1010,464,240,105 };
	//特殊変数////////////////////////////////////////////////////////
	WordDetector wordDetector{};

	//変数////////////////////////////////////////////////////////////
	//プレイヤーが存在するか
	std::vector<bool> player_flag;
	//プレイヤー関連
	int player_number = 0;
	int another_player_number = 1;
	//時間
	int internal_timer = 0;
	//描画用変数
	double fade_back_alpha = 1.0;
	int fade_back_timer = 0;
	double settle_fade = 0.0;
	bool finish_fade_mode = false;
	double finish_fade = 0.0;
	//エラー関連
	int error_mode = 0;
	//1:404,2:タイムアウト
	int error_ID = 0;
	int error_timer = 0;
	int error_pos_y = 1400;
	double back_alpha = 0.0;
	//ゲームシステム用
	bool is_game_finished = false;
	bool are_you_winnner = true;
	int settle_timer = 0;
	int settle_mode = 0;

	//通信用の変数////////////////////////////////////////////////////
	int connection_timer = 0;
	double ping_time = 0.0;
	bool is_connected = false;
	int ping = 0;
	int ping_timer = 0;

	//内部関数////////////////////////////////////////////////////////

	//1:上,2:左,4:下,8:右
	int getkey();
	void draw_player() const;
	void draw_HP_bar() const;
	void draw_AP_bar() const;
	void draw_ping() const;
	void draw_settle() const;
	void update_player();
	void update_player_animation();
	void update_AP_bar_animation();
	void update_settle();
	void synchronizate_data();
	void update_error_screen();
	int voice_command();
	inline int sign(bool plus_or_minus) {return plus_or_minus ? 1 : -1;}
	void Json2ArrayPos(String str,Vec2 (& pos)[2]);
	void Json2ArrayTimer(String str, int(&timer)[16]);
	void Json2ArrayHP(String str, int(&hp)[3]);
	inline int GameTimer();
public:

	Game(const InitData& init);

	void update() override;

	void draw() const override;
};
