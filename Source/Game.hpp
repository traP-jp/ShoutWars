# pragma once
# include "common.hpp"
# include "Voice/CommandRecognizer.hpp"
# include "CommandFeedback.hpp"
# include "ControlsGuide.hpp"
# include "PlayerInput.hpp"
# include "VoiceMonitor.hpp"
# include "CpuBrain.hpp"
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

//#define debug_mode

struct Player {
	//各種変数
	Vec2 pos[2];
	//ステータス(0:待機中,1:左移動,2:右移動,4ジャンプ,8:ガード,16:弱,32:狂,64:必殺,128:ガード破壊,256:特殊攻撃,512～:予備)
	int status = 0;
	//HP(0:実質HP(確定),1:表示HP(未確定),2:表示HP(確定))
	int hp[3] = { 1000,1000,1000 };
	int ap = 0;
	//歩く速さ (100 ミリ秒あたりの px)。声で言っている間に間合いが大きく変わらないよう、遅めにしている
	double speed = 55.0;
	//Playerに関する時間(0:左右移動,1:進捗(0),2:ジャンプ,3:ガード,4:弱,5:狂,6:必殺,7:進捗(1),8:進捗(3),9:進捗(4),10:進捗(5),11:進捗(6),12:ガード破壊,13:進捗(12),14:特殊攻撃,15:進捗(14))
	int timer[16] = {};

	//Playerに関するse(0:左右移動,1:ジャンプ,2:弱,3:狂,4:必殺,5:ガード,6:ガード破壊,7:特殊攻撃)
	bool se[8] = { false };
	//Playerの向き(true:右,false:左)
	bool direction = false;
	//1フレーム限りのイベント (弾が同じフレームに何発も当たらないようにする) (0:なし,1:弱,2:狂,4:必殺,8:ガード破壊)
	int event;
	//今出している技が、もう相手に当たったか (近接攻撃を 1 回の技で 1 回だけ当てるため。ビットは status と同じ)
	int hit_done = 0;
	//ガードを壊された時刻 (この後しばらくガードできない)
	int guard_broken_time = -1000000;
	//ジャンプから着地した時刻 (この後しばらくジャンプと攻撃ができない)
	int landing_time = -1000000;
	//必殺技が使えるか
	bool special_attack = false;
	//キャラ(0:玲（レイ）,1:ユウカ,2:アイリ,3:No.0 (レイ）
	int number = 0;
	//アイリ専用
	int knife_mode = 0;
	int airi_old_timer = 0;
	double wave_pos = 0.0;
	bool walking = true;
	//必殺技の溜めの間の点滅の強さ (0 なら点滅しない)
	double charge_glow = 0.0;
	//相手を引き寄せる必殺技の溜めを始めてからの秒数 (溜めていなければ負)
	double pull_seconds = -1.0;

	int img_number = 0;
	int img_status = 0;
	int img_timer = 0;

	int fire_animation = 0;
	int fire_animation_timer = 0;
};

struct bullet {
	Vec2 pos;
	Vec2 old_pos;
	double angle;
	double old_angle;
	int timer;
	bool exist = false;
	bool disable_disappear = false;
	int direction;
	int mode;
	int character;
	int type = 0;
};

struct torpedo {
	Vec2 pos;
	Vec2 old_pos;
	double angle;
	bool exist = false;
	int timer;
	int mode = 0;
};

struct knife {
	Vec2 pos;
	Vec2 old_pos;
	Vec2 goal_pos;
	int mode = 0;
	int timer[2];
	bool exist = false;
	bool horming = true;
	//0:表示用アングル,1:元のアングル,2:目標アングル
	double angle[3];
	double distance;
	int time;
	int img_number;
};

struct occation_effect {
	Vec2 pos;
	int timer;
	int type = 0;
	bool exist = false;
	double alpha = 1.0;
	double scale = 3.0;
};

struct after_image {
	Vec2 pos;
	double angle;
	int timer;
	bool exist = false;
	double alpha = 1.0;
	int img_number;
};

class Game : public App::Scene
{
private:
	//キャラ性能に関する定数////////////////////////////////////////////
	//玲（レイ）
	//ダメージ量
	const static int rei_weak_atttack = 4;
	const static int rei_strong_attack = 6;
	const static int rei_special_attack = 100;
	const static int rei_strong_attack_bomb = 10;
	const static int rei_uniqe_attack = 8;
	//AP回復量 (必殺技の分は、当てられた側に溜まる)
	const static int rei_weak_atttack_ap = 5;
	const static int rei_strong_attack_ap = 12;
	const static int rei_special_attack_ap = 16;
	const static int rei_uniqe_attack_ap = 8;

	//ユウカとアイリの値は、言う時間・認識の遅れ・技の時間・SN 比 10 dB での発動率・当たりやすさから見積もった期待ダメージを出発点にしている
	//弱攻撃と強攻撃の 1 秒あたりの期待ダメージをキャラ間でそろえ、SN 比 10 dB で発動しにくいコマンドほど 1 回の威力を上げる。
	//攻撃側に溜まる AP はダメージの 1.5 倍で、ガードされたら溜まらない
	//ユウカ
	//ダメージ量
	const static int yuuka_weak_atttack = 24;
	const static int yuuka_strong_attack = 32;
	const static int yuuka_special_attack = 300;
	//AP回復量 (必殺技の分は、当てられた側に溜まる)
	const static int yuuka_weak_atttack_ap = 36;
	const static int yuuka_strong_attack_ap = 48;
	const static int yuuka_special_attack_ap = 120;
	//強攻撃の後、動けるようになるまでに延ばす時間 (ミリ秒)。「キック」はほぼ確実に発動し「いー」でも出るので、連打しにくくする
	const static int yuuka_strong_attack_recovery_ms = 100;
	//アイリ
	//ダメージ量 (必殺技はナイフ 1 本、特殊攻撃は弾 1 発あたり)
	const static int airi_weak_atttack = 20;
	const static int airi_strong_attack = 30;
	const static int airi_special_attack = 14;
	const static int airi_uniqe_attack = 1;
	//AP回復量 (必殺技の分は、当てられた側に溜まる)
	const static int airi_weak_atttack_ap = 30;
	const static int airi_strong_attack_ap = 45;
	const static int airi_special_attack_ap = 5;
	const static int airi_uniqe_attack_ap = 1;
	//強攻撃の後、動けるようになるまでに延ばす時間 (ミリ秒)。「切れ」は言う時間が短く発動しやすいので、連打しにくくする
	const static int airi_strong_attack_recovery_ms = 150;
	//No.0 (レイ）
	//ダメージ量
	const static int no0_weak_atttack = 5;
	const static int no0_strong_attack = 7;
	const static int no0_special_attack = 8;
	//AP回復量 (必殺技の分は、当てられた側に溜まる)
	const static int no0_weak_atttack_ap = 3;
	const static int no0_strong_attack_ap = 5;
	const static int no0_special_attack_ap = 8;
	//定数////////////////////////////////////////////////////////////
	const static int player_sum = 2;
	//対戦の制限時間 (秒)。慣れた人がスムーズに進めて 2 分、初めて遊ぶ人は 5 分ほどかかる見込みなので余裕を持たせ、サーバーの対戦の期限 (20分) より短く取る
	const static int match_seconds = 600;
	const static int player_min_y = 650;
	//キャラが動ける横の範囲
	const static int stage_min_x = 50;
	const static int stage_max_x = 1850;
	const static int player_max_hp = 1000;
	//技が発動するために必要なAP
	const static int player_max_ap = 500;
	//キャラの食らい判定の縦の範囲 (キャラの位置からのずれ)。足元を外して、ジャンプで攻撃をある程度よけられるようにする
	const static int hurtbox_top = -170;
	const static int hurtbox_bottom = 40;
	//近接攻撃の当たり判定の縦の範囲 (攻撃するキャラの位置からのずれ)
	const static int melee_top = -120;
	const static int melee_bottom = 60;
	//弾やナイフの当たり判定の縦の半径
	const static int projectile_radius = 10;
	//アイリのナイフの狙う高さ (相手の位置からのずれ)。体の中心を狙うと少し跳ぶだけで下を抜けるので、胸の高さを狙う
	const static int knife_aim_y = -100;
	//着地してからジャンプと攻撃ができない時間 (ミリ秒)。跳び続けて攻撃をよけ続けることに代償を付ける (ガードはできる)
	const static int landing_recovery_ms = 300;
	//ガードが続く時間と、壊された後にガードできない時間 (ミリ秒)
	const static int guard_ms = 2000;
	const static int guard_cooldown_ms = 3000;
	//ガード破壊のダメージと、攻撃側に溜まる AP
	const static int destroy_guard_damage = 10;
	const static int destroy_guard_ap = 15;
	//技の最中に、別の行動を始められないようにする状態のビット
	//攻撃: ジャンプ・ガード・攻撃の最中 / ガード: ガード・攻撃の最中 / 左右移動: 移動・ガード・弱攻撃以外の攻撃の最中 / ジャンプ: ジャンプ・ガード・攻撃の最中
	const static int attack_blocking_status = 4 | 8 | 16 | 32 | 64 | 128 | 256;
	const static int guard_blocking_status = 8 | 16 | 32 | 64 | 128 | 256;
	const static int move_blocking_status = 1 | 2 | 8 | 32 | 64 | 128 | 256;
	const static int jump_blocking_status = 4 | 8 | 16 | 32 | 64 | 128 | 256;
	//アイリの連射の段取り (技の開始からのミリ秒)。構えてから撃ち始めるまでと、撃ち終わってから銃を下ろすまでに間を置く
	const static int airi_unique_raise_ms = 130;
	const static int airi_unique_fire_start_ms = airi_unique_raise_ms + 400;
	const static int airi_unique_fire_end_ms = airi_unique_fire_start_ms + 1400;
	const static int airi_unique_lower_ms = airi_unique_fire_end_ms + 400;
	const static int airi_unique_end_ms = airi_unique_lower_ms + 30;
	//必殺技を見てから「ガード」と言えば間に合い、言い直すと間に合わない程度に、出始めから当たるまでの間を空ける
	//(気付いて言い始めるまで約 0.5 秒 + 「ガード」と言い終えるまで 0.4〜0.75 秒 + 認識 0.1〜0.3 秒 + 通信)
	//ユウカは最初の構えの姿勢を延ばし、アイリは出したナイフが浮いている時間を延ばす
	const static int yuuka_special_windup_ms = 1500;
	//ユウカの必殺技の当たり判定 (前は正、後ろは負。縦は攻撃するキャラの位置からのずれ)。通常の近接攻撃 (前 5〜230px、上 120px〜下 60px) より広い
	const static int yuuka_special_front_range = 380;
	const static int yuuka_special_back_range = -80;
	const static int yuuka_special_top = -220;
	const static int yuuka_special_bottom = 60;
	//ユウカの必殺技の溜めの間に、相手を引き寄せる速さ (px/秒)、届く距離、止める距離
	const static int yuuka_special_pull_speed = 250;
	const static int yuuka_special_pull_range = 900;
	const static int yuuka_special_pull_stop = 120;
	const static int airi_knife_hover_ms = 1300;
	//最大同時存在弾丸数は120
	const static int max_bullet = 120;
	//最大同時存在ナイフ数は50本
	const static int max_knife = 50;
	//最大同時存在エフェクト数は10
	const static int max_occation = 10;
	//最大同時存在残像数は20
	const static int max_after_images = 20;
	//最大同時存在魚雷数は2
	const static int max_torpedo = 2;
	//構造体////////////////////////////////////////////////////////////
	struct Player player[player_sum];
	struct bullet bullet[max_bullet];
	struct knife knife[max_knife];
	struct occation_effect occation[max_occation];
	struct after_image after_images[max_after_images];
	struct torpedo torpedo[max_torpedo];
	//font////////////////////////////////////////////////////////////
	Font font{ 40 };
	//画像////////////////////////////////////////////////////////////
	const Texture background_img{ Resource(U"images/game/system/background.png")};
	const Texture HP_bar_flame_img{ Resource(U"images/game/system/HP_bar_flame.png") };
	const Texture HP_bar_gray_img{ Resource(U"images/game/system/HP_bar_gray.png") };
	const Texture HP_bar_red_img{ Resource(U"images/game/system/HP_bar_red.png") };
	const Texture HP_bar_blue_img{ Resource(U"images/game/system/HP_bar_blue.png") };
	const Texture AP_bar_empty_img{ Resource(U"images/game/system/AP_bar_empty.png") };
	const Texture AP_bar_max_img{ Resource(U"images/game/system/AP_bar_max.png") };
	const Texture AP_bar_img{ Resource(U"images/game/system/AP_bar.png") };
	const Texture destroyed_img{ Resource(U"images/game/system/destroyed.png") };
	const Texture timeout_img{ Resource(U"images/game/system/timeout.png") };
	const Texture connecting_img{ Resource(U"images/common/connecting.png") };
	const Texture guard_img{ Resource(U"images/game/system/guard.png") };
	const Texture ping_fast_img{ Resource(U"images/game/system/ping_fast.png") };
	const Texture ping_middle_img{ Resource(U"images/game/system/ping_middle.png") };
	const Texture ping_slow_img{ Resource(U"images/game/system/ping_slow.png") };
	const Texture you_win_img{ Resource(U"images/game/system/you_win.png") };
	const Texture you_lose_img{ Resource(U"images/game/system/you_lose.png") };
	const Texture settle_img{ Resource(U"images/game/system/settle.png") };
	const Texture guns_img{ Resource(U"images/game/system/guns.png") };
	const Texture knives_img{ Resource(U"images/game/system/knives.png") };
	const Texture occation_img{ Resource(U"images/game/system/occation.png") };
	const Texture torpedo_img{ Resource(U"images/game/system/torpedo.png") };
	std::vector<std::vector<Texture>> player_img;
	std::vector<Texture> fire_img;
	std::vector<Texture> command_img;
	//音楽////////////////////////////////////////////////////////////
	const Audio bgm{ Resource(U"audioes/Es-Boss3_loop.ogg") , Arg::loopBegin = 28.848843537415s};
	const Audio dos_se{ Resource(U"audioes/dos.wav") };
	const Audio dododos_se{ Resource(U"audioes/dododos.wav") };
	const Audio jump_se{ Resource(U"audioes/jump.wav") };
	const Audio shot_se{ Resource(U"audioes/shot.wav") };
	const Audio kiran_se{ Resource(U"audioes/kiran.wav") };
	const Audio bom_se{ Resource(U"audioes/bom.wav") };
	const Audio cancel_sound{ Resource(U"audioes/cancel.wav") };
	const Audio guard_se{ Resource(U"audioes/guard.mp3") };
	const Audio void_damage_se{ Resource(U"audioes/void_damage.mp3") };
	const Audio break_guard_se{ Resource(U"audioes/break_guard.wav") };
	const Audio gun_se{ Resource(U"audioes/gun.mp3") };
	const Audio bomber_se{ Resource(U"audioes/bomber.mp3") };
	const Audio gun_reflect1_se{ Resource(U"audioes/gun_reflect1.mp3") };
	const Audio gun_reflect2_se{ Resource(U"audioes/gun_reflect2.mp3") };
	const Audio gun_reflect3_se{ Resource(U"audioes/gun_reflect3.mp3") };
	const Audio torpedo_se{ Resource(U"audioes/torpedo.mp3") };
	//shape////////////////////////////////////////////////////////////
	const Rect OK_shape{ 680,464,240,105 };
	const Rect Yes_shape{ 1010,464,240,105 };
	//特殊変数////////////////////////////////////////////////////////
	CommandRecognizer commandRecognizer;
	//声の届き方と、技が出た・出せなかったことを画面で知らせる
	VoiceMonitor voiceMonitor{ RectF{ 640, 885, 640, 150 } };
	CommandFeedback commandFeedback;
	ControlsGuide controlsGuide;
	//前のフレームの状態 (技が出た瞬間を知るため)
	int previous_status[player_sum] = { 0 };
	//前回の処理から、通信で届いた相手の状態で新しく立ち上がったビット
	//相手の状態は手元のアニメーションでも下ろすため、手元の状態の立ち上がりで見ると、遅れて届いた状態で技が始まり直したように見えてしまう
	int received_started_status = 0;
	//前のフレームでジャンプの入力があったか (押しっぱなしでは続けて跳ばないようにするため)
	bool previous_jump_input = false;
	//自分がガードを壊されてから、再びガードできるまでの残りの割合 (0 ならガードできる)
	double guard_cooldown_ratio = 0.0;

	//変数////////////////////////////////////////////////////////////
	//プレイヤーが存在するか
	std::vector<bool> player_flag;
	//プレイヤー関連
	int player_number = 0;
	int another_player_number = 1;
	//CPU と対戦するときの部屋 (相手は CPU で、手元で動かす)。通信対戦では nullptr
	Multiplay::LocalRoom* cpu_room = nullptr;
	Optional<CpuBrain> cpu_brain;
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
#ifndef debug_mode
	bool is_connected = false;
#else
	bool is_connected = true;
#endif
	//相手から最後に届いた状態 (効果音を立ち上がりでだけ鳴らすため)
	int received_status = 0;
	//確認イベントで確定したガード状態
	bool void_attack[player_sum] = { false };
	//対戦の残り時間 (秒)。開始の tick から数えるので全員で一致する
	int remaining_seconds = match_seconds;
	int ping = 0;
	int ping_timer = 0;
	Array<Duration> rtt_samples;

	//内部関数////////////////////////////////////////////////////////

	//1:上,2:左,4:下,8:右
	int getkey();
	void draw_player() const;
	void draw_bullet() const;
	void draw_knife() const;
	void draw_torpedo() const;
	void draw_effects() const;
	void draw_special_pull() const;
	void draw_after_images() const;
	void draw_HP_bar() const;
	void draw_AP_bar() const;
	void draw_ping() const;
	void draw_settle() const;
	void update_player();
	void update_player_animation();
	void update_AP_bar_animation();
	void update_settle();
	void update_effects();
	void synchronizate_data();
	void update_error_screen();
	void showError(const Multiplay::APIError& error);
	void finish_game(bool won);
	int voice_command();
	void handle_started_moves();
	[[nodiscard]] bool is_guard_cooling_down(int cnt, int now_time) const;
	[[nodiscard]] bool is_landing_recovery(int cnt, int now_time) const;
	/// @brief 走ってかがんでいて、弾やナイフが頭の上を抜けるか (走りの姿勢で頭が下がるのは玲とユウカだけで、アイリと No.0 はかがまない)
	[[nodiscard]] bool is_ducking(int cnt) const;
	[[nodiscard]] bool can_start_attack(int cnt, int now_time) const;
	/// @brief 位置や状態を手元で決めるプレイヤーか (自分と CPU。通信相手は相手のクライアントが決める)
	[[nodiscard]] bool is_local_player(int cnt) const;
	/// @brief 手元で動かすプレイヤーからの確認イベントを送る
	void send_action(int sender, StringView type, int target);
	/// @brief 左右移動を始める (押している間、毎フレーム呼ぶ)。direction は 1:左, 2:右
	void start_walk(int cnt, int direction, int now_time);
	[[nodiscard]] CpuView make_cpu_view(int now_time) const;
	/// @brief ジャンプを始める。跳べなければ false
	bool start_jump(int cnt, int now_time);
	/// @brief 技を始める (action は CommandRecognizer の番号 1:弱攻撃, 2:強攻撃, 3:必殺技, 4:ガード, 5:ガード破壊, 6:特殊攻撃)。出せなければ false
	bool start_move(int cnt, int action, int now_time);
	inline int sign(bool plus_or_minus) {return plus_or_minus ? 1 : -1;}
	void Json2ArrayPos(const JSON& json, Vec2 (& pos)[2]);
	void Json2ArrayTimer(const JSON& json, int(&timer)[16]);
	inline int GameTimer();
	Vec2 draw_player_pos(Vec2 player_pos,int i) const;
	//各キャラ専用関数
	void rei_attack(int cnt,int now_time,Vec2 player_reserved_pos[]);
	void yuuka_attack(int cnt, int now_time, Vec2 player_reserved_pos[]);
	void airi_attack(int cnt, int now_time, Vec2 player_reserved_pos[]);
	void setting_knife(int cnt,int now_time,Vec2 player_reserved_pos[], int now_number);
	void no0_attack(int cnt, int now_time, Vec2 player_reserved_pos[]);

	//type=0:弱,1:狂,2:必殺
	void call_bullet(int cnt, int now_time, Vec2 player_reserved_pos[],int type);

	//get_character_power_ap(番号,攻撃の種類)
	//攻撃の種類(0:弱,1:狂,2:必殺,3:特殊)
	int get_character_power(int character_number, int attack_sort);
	[[nodiscard]] int strong_attack_recovery_ms(int character_number) const;
	/// @brief 縦の範囲 [top, bottom] が、target_y にいるキャラの食らい判定に重なるか
	[[nodiscard]] static bool overlaps_hurtbox(double target_y, double top, double bottom) {
		return (target_y + hurtbox_top < bottom) && (top < target_y + hurtbox_bottom);
	}
	/// @brief attacker_y にいるキャラの近接攻撃が、縦方向で target_y にいるキャラに届くか
	[[nodiscard]] static bool melee_reaches(double attacker_y, double target_y) {
		return overlaps_hurtbox(target_y, attacker_y + melee_top, attacker_y + melee_bottom);
	}

public:

	Game(const InitData& init);
	~Game() { getData().phoneme.stop(); }

	void update() override;
	void updateFadeIn(double t) override;

	void draw() const override;
};
