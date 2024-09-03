#include "Game.hpp"

#define player_number 0
#define another_player_number 1

using namespace std;

Game::Game(const InitData& init) : IScene(init),
player_img(4),
fire_img(5),
player_flag(player_sum, true)
{
	//システムで使用する画像
	for (int i = 0; i < 5; i++)
		fire_img.at(i) = Texture{ Unicode::Widen("../images/game/system/fire"+to_string(i)+".png") };

	//玲の画像
	if ((getData().player[0] == 0) || (getData().player[1] == 0)) {
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/waiting.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/run1.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/run2.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/1.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/2.png") });
	}
	//ユウカの画像
	if ((getData().player[0] == 1) || (getData().player[1] == 1)) {
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/waiting.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/run1.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/run2.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/weak_kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/powerful_kick.png") });
	}
	//アイリの画像
	if ((getData().player[0] == 2) || (getData().player[1] == 2)) {
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/waiting.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/run1.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/run2.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/1.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/2.png") });
	}
	//No.0の画像
	if ((getData().player[0] == 3) || (getData().player[1] == 3)) {
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/waiting.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/run1.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/run2.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/1.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/2.png") });
	}

	player[0].pos[0] = {600.0,player_min_y};
	player[1].pos[0] = {1200,player_min_y};
	player[1].direction = true;

	internal_timer = (int)Time::GetMillisec();
	init_connection();
}

int Game::getkey() {
	int retkey = 0;
	retkey |= 1 * KeyW.pressed();
	retkey |= 2 * KeyA.pressed();
	retkey |= 4 * KeyS.pressed();
	retkey |= 8 * KeyD.pressed();

	retkey |= 1 * KeyUp.pressed();
	retkey |= 2 * KeyLeft.pressed();
	retkey |= 4 * KeyDown.pressed();
	retkey |= 8 * KeyRight.pressed();
	return retkey;
}

//TODO:音声認識が届いたら実装
int Game::voice_command() {
	if (KeyF.pressed()) return 2;
	if (KeyG.pressed()) return 3;
	return 0;
}

void Game::update() {
	//プレイヤー情報を更新
	update_player();
	//APバーの描画情報を更新
	update_AP_bar_animation();
	//プレイヤーのアニメーションを更新
	update_player_animation();
}

void Game::update_player() {
	int now_time = (int)Time::GetMillisec();
	Vec2 player_reserved_pos = player[player_number].pos[0];
	//プレイヤー(ユーザー操作)の移動処理////////////////////////////////////////////////////////////
	//左右移動処理
	if (player[player_number].status & 3) {
		int t = now_time - player[player_number].timer[0];
		if (t < 50) {
			player_reserved_pos.x = player[player_number].pos[1].x + ((player[player_number].status & 1) ? -1.0 : 1.0) * player[player_number].speed * t / 50;
		}else {
			player[player_number].status ^= (player[player_number].status&1)?1:2;
		}
	}
	//ジャンプ処理
	if (player[player_number].status & 4) {
		int t = now_time - player[player_number].timer[2];
		if (t < 500) {
			player_reserved_pos.y = player_min_y - 2.0 * t + 0.004 * t * t;
		}
		else {
			player[player_number].status ^= 4;
			player_reserved_pos.y = player_min_y;
		}
	}
	//キー入力処理
	if (int gotkey = getkey()) {
		//左右移動
		if ((player[player_number].status & 3) == 0){
			if (gotkey & 10) {
				player[player_number].status |= (gotkey & 2)?1:2;
				player[player_number].timer[0] = now_time;
				player[player_number].pos[1].x = player[player_number].pos[0].x;
			}
		}
		//ジャンプ
		if ((gotkey & 1) && ((player[player_number].status & 52) == 0)) {
			jump_se.playOneShot();
			player[player_number].status |= 4;
			player[player_number].timer[2] = now_time;
			player[player_number].pos[1].y = player[player_number].pos[0].y;
		}
	}
	//プレイヤー(相手)の移動処理////////////////////////////////////////////////////////////////////

	//TODO：通信関連が届いたら実装
	synchronizate_data();

	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (i == player_number) continue;
		if ((abs(player_reserved_pos.x - player[i].pos[0].x) < 50.0)&&(abs(player_reserved_pos.y - player[i].pos[0].y) < 242.0)) {
			//相手が動いている場合
			if (player[i].status&3) {
				//TODO:通信関連が届いたら実装
			}else {
				player[i].pos[0].x += (player_reserved_pos.x < player[i].pos[0].x) ? 16.0 : -16.0;
			}
		}
	}

	//プレイヤーの向き
	if (player_reserved_pos.x < player[another_player_number].pos[0].x) {
		player[player_number].direction = false;
		player[another_player_number].direction = true;
	}else {
		player[player_number].direction = true;
		player[another_player_number].direction = false;
	}
	if (player[player_number].status & 3)player[player_number].direction = (player[player_number].status & 1);

	//技とかの起爆/////////////////////////////////////////////////////////////////////////////////
	if (int got_voice = voice_command()) {
		//弱攻撃
		if (got_voice == 1) {

		//狂攻撃
		}elif(got_voice == 2) {
			if ((player[player_number].status & 116) == 0) {
				shot_se.playOneShot();
				player[player_number].se[3] = true;
				player[player_number].status |= 32;
				player[player_number].timer[5] = now_time;
			}
		//必殺技
		}elif ((got_voice == 3)&&(player[player_number].special_attack)) {
			if ((player[player_number].status & 116) == 0) {
				bom_se.playOneShot();
				player[player_number].se[4] = true;
				player[player_number].status |= 64;
				player[player_number].timer[6] = now_time;
				player[player_number].ap = 0;
				player[player_number].special_attack = false;
			}
		}

	}

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////
	//弱攻撃
	if (player[player_number].status & 16) {

	}
	//狂攻撃+必殺技
	if (player[player_number].status & 96) {
		int tmp = now_time - player[player_number].timer[(player[player_number].status & 32)?5:6];
		if ((200 < tmp) && (tmp < 400)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == player_number) continue;
				int tmp_pos_x = sign(player[player_number].direction)*(player_reserved_pos.x - player[i].pos[0].x);
				if ((5.0 < tmp_pos_x) &&(tmp_pos_x < 250.0) && (abs(player_reserved_pos.y - player[i].pos[0].y) < 242.0)) {
					if ((player[i].event[0] & 2) == 0) {
						//狂攻撃
						if (player[player_number].status & 32) {
							if (player[player_number].se[3]) {
								player[player_number].se[3] = false;
								dos_se.playOneShot();
							}
							player[i].event[0] |= 2;
							//TODO:あとで通信専用の時間に変更する
							player[i].event[1] = now_time;
							player[i].hp[1] -= 5;
							player[player_number].ap += 3;
						//必殺技
						}else {
							if (player[player_number].se[4]) {
								player[player_number].se[4] = false;
								dododos_se.playOneShot();
							}
							player[i].event[0] |= 4;
							//TODO:あとで通信専用の時間に変更する
							player[i].event[1] = now_time;
							player[i].hp[1] -= 15;
						}
					}
				}
			}
		}
	}

	//AP管理///////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (player[i].ap >= player_max_ap) {
			player[i].ap = player_max_ap;
			if (!player[i].special_attack) {
				kiran_se.playOneShot();
				player[i].special_attack = true;
			}
		}
	}
	//HP管理///////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (player[i].hp[2] > player[i].hp[0])player[i].hp[2]--;
		if (player[i].hp[0] <= 0);//TODO:GAMEOVER処理
	}

	//移動範囲制限
	player_reserved_pos.x = Clamp(player_reserved_pos.x, 50.0, 1850.0);

	//プレイヤーの位置を更新を確定
	player[player_number].pos[0] = player_reserved_pos;
}

//APバーのアニメーションを更新
void Game::update_AP_bar_animation() {
	int now_time = (int)Time::GetMillisec();
	//APバーが満タンの時、炎のアニメーションを描く
	for (int i = 0; i < player_sum; i++) {
		if (!player_flag[i]) continue;
		if (player[i].special_attack) {
			if (now_time - player[i].fire_animation_timer > 150) {
				player[i].fire_animation_timer = now_time;
				player[i].fire_animation = (1 + player[i].fire_animation) % 5;
			}
		}
	}
}

//TODO:通信関連が届いたら実装
void Game::init_connection() {
	//同期
	getData().client->update();
	connection_timer = (int)Time::GetMillisec();
	return;
}

//TODO:通信関連が届いたら実装
void Game::synchronizate_data() {
	//同期
	getData().client->update();
	if ((int)Time::GetMillisec() - connection_timer > 50) {
		//TODO:通信処理

		//いろいろ初期化
		connection_timer = (int)Time::GetMillisec();
		for (int i = 0; i < player_sum; i++) {
			if (i == player_number) continue;
			player[i].event[0] = 0;
			player[i].event[1] = 0;
			//TODO:同期後のHPを処理する
			player[i].hp[0] = player[i].hp[1];
		}
	}
}


void Game::update_player_animation() {
	int now_time = (int)Time::GetMillisec();
	for (int i = 0; i < player_sum; i++) {
		if (!player_flag[i]) continue;
		//待機中
		if (player[i].status == 0) {
			player[i].img_number = 0;
			continue;
		//スタン
		}elif(player[i].status == 8) {
			player[i].img_number = 0;
			continue;
		//弱攻撃のアニメーション
		}elif(player[i].status & 16) {
			player[i].img_number = 0;
			continue;
		//狂攻撃のアニメーション
		}elif(player[i].status & 32) {
			if (now_time - player[i].timer[5] < 130) {
				player[i].img_number = 0;
			}elif (now_time - player[i].timer[5] < 320) {
				player[i].img_number = 3;
			}elif (now_time - player[i].timer[5] < 450) {
				player[i].img_number = 0;
			}
			else {
				player[i].status ^= 32;
				player[i].img_number = 0;
			}
			continue;
		//必殺技のアニメーション
		}elif(player[i].status & 64) {
			if (now_time - player[i].timer[6] < 120) {
				player[i].img_number = 3;
			}elif (now_time - player[i].timer[6] < 330) {
				player[i].img_number = 5;
			}elif (now_time - player[i].timer[6] < 450) {
				player[i].img_number = 3;
			}else{
				player[i].status ^= 64;
				player[i].img_number = 0;
			}
			continue;
		//ジャンプアニメーション
		}elif(player[i].status & 4) {
			player[i].img_number = 0;
			continue;
		//移動アニメーション
		}elif(player[i].status & 3) {
			if (now_time - player[i].img_timer > 160) {
				player[i].img_timer = now_time;
				player[i].img_status = 1+player[i].img_status % 2;
			}
			player[i].img_number = player[i].img_status;
			continue;
		}
		player[i].img_number = 0;
	}
}


void Game::drawFadeIn(double t) const {
	//BGMを流す
	if (!bgm.isPlaying())bgm.play();
	draw();
	Rect(0, 0, 1920, 1080).draw(ColorF{ 0,1.0 - t });
}

void Game::draw() const {
	Scene::SetBackground(ColorF(0.2, 0.8, 0.6));
	background_img.draw(0,0);
	draw_HP_bar();
	draw_AP_bar();

	draw_player();
}

//キャラの描画
void Game::draw_player() const {
	for (int i = 0; i < player_sum;i++) {
		if (!player_flag[i]) continue;
		player_img.at(getData().player[i]).at(player[i].img_number).mirrored(player[i].direction).drawAt(player[i].pos[0]);
	}
}

//HPバーの描画
void Game::draw_HP_bar() const {
	//1PのHP
	HP_bar_flame_img.draw(120, 70);
	HP_bar_gray_img.draw(130, 78);
	HP_bar_red_img(0, 0, 680.0 * ((double)player[0].hp[2] / player_max_hp), 25).draw(130, 78);
	HP_bar_blue_img(0, 0, 680.0 * ((double)player[0].hp[1] / player_max_hp), 25).draw(130, 78);

	//2PのHP
	HP_bar_flame_img.draw(1090, 70);
	HP_bar_gray_img.draw(1100, 78);
	HP_bar_red_img (680.0 * (1.0 - ((double)player[1].hp[2] / player_max_hp)), 0, 680.0 * ((double)player[1].hp[2] / player_max_hp), 25).mirrored().draw(1100, 78);
	HP_bar_blue_img(680.0 * (1.0 - ((double)player[1].hp[1] / player_max_hp)), 0, 680.0 * ((double)player[1].hp[1] / player_max_hp), 25).mirrored().draw(1100, 78);
}

//APバーの描画
void Game::draw_AP_bar() const {
	//1PのAP
	AP_bar_empty_img.mirrored().draw(120, 880);
	if (player[0].special_attack) {
		AP_bar_max_img.mirrored().draw(120, 880);
		{
			const ScopedRenderStates2D blend{ BlendState::Additive };
			fire_img.at(player[0].fire_animation).draw(90, 800);
		}
	}else {
		AP_bar_img(0, 0, 360.0 * ((double)player[0].ap / player_max_ap), 42).mirrored().draw(612.0- 360.0 * ((double)player[0].ap / player_max_ap), 992);
	}


	//2PのAP
	AP_bar_empty_img.draw(1300, 880);
	if (player[1].special_attack) {
		AP_bar_max_img.draw(1300, 880);
		{
			const ScopedRenderStates2D blend{ BlendState::Additive };
			fire_img.at(player[1].fire_animation).draw(1610, 800);
		}
	}else {
		AP_bar_img(0, 0, 360.0 * ((double)player[1].ap / 100), 42).mirrored().draw(1305, 992);
	}
}
