#include "Game.hpp"

#define player_number 0
#define another_player_number 1

using namespace std;

Game::Game(const InitData& init) : IScene(init),
player_img(4),
player_flag(player_sum, true)
{
	//玲の画像
	if ((getData().player[0] == 0) || (getData().player[1] == 0)) {
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/player1.png") });
	}
	//ユウカの画像
	if ((getData().player[0] == 1) || (getData().player[1] == 1)) {
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/1/00.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/1/01.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/1/0.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/1/1.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/1/2.png") });
	}
	//アイリの画像
	if ((getData().player[0] == 2) || (getData().player[1] == 2)) {
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/2/00.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/2/01.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/2/0.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/2/1.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/2/2.png") });
	}
	//No.0の画像
	if ((getData().player[0] == 3) || (getData().player[1] == 3)) {
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/3/00.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/3/01.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/3/0.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/3/1.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/3/2.png") });
	}

	player[0].pos[0] = {100.0,player_min_y};
	player[1].pos[0] = {1200,player_min_y};

	internal_timer = Time::GetMillisec();
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
	return KeyF.pressed();
}

void Game::update() {
	//BGMを流す
	if (!bgm_sw) {
		bgm_sw = true;
		bgm.play();
	}
	//プレイヤー情報を更新
	update_player();
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
		if ((player[player_number].status & 3) == 0){
			if (gotkey & 10) {
				player[player_number].status |= (gotkey & 2)?1:2;
				player[player_number].timer[0] = now_time;
				player[player_number].pos[1].x = player[player_number].pos[0].x;
			}
		}
		if ((gotkey & 1) && ((player[player_number].status & 4) == 0)) {
			player[player_number].status |= 4;
			player[player_number].timer[2] = now_time;
			player[player_number].pos[1].y = player[player_number].pos[0].y;
		}
	}
	//プレイヤー(相手)の移動処理////////////////////////////////////////////////////////////////////

	//TODO：通信関連が届いたら実装

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

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////

	//TODO:画像来たらやれ

	//HP管理///////////////////////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (player[i].hp[1] > player[i].hp[0])player[i].hp[1]--;
		if (player[i].hp[0] <= 0);//TODO:GAMEOVER処理
	}

	//移動範囲制限
	player_reserved_pos.x = Clamp(player_reserved_pos.x, 50.0, 1850.0);

	//プレイヤーの位置を更新を確定
	player[player_number].pos[0] = player_reserved_pos;
}


void Game::update_player_animation() {
	int tmp_time = (int)Time::GetMillisec();
	for (int i = 0; i < player_sum; i++) {
		if (!player_flag[i]) continue;
		//待機中
		if (player[i].status == 0) {
			player[i].img_number = 0;
			continue;
		//ジャンプアニメーション
		}elif(player[i].status & 4) {
			player[i].img_number = 0;
			continue;
		//移動アニメーション
		}elif(player[i].status & 3) {
			if (tmp_time - player[i].img_timer > 150) {
				player[i].img_timer = tmp_time;
				player[i].img_status = (player[i].img_status + 1) % 2;
			}
			player[i].img_number = player[i].img_status;
			continue;
		//スタン
		}elif(player[i].status & 8) {
			player[i].img_number = 0;
			continue;
		//弱攻撃のアニメーション
		}elif(player[i].status & 16) {
			player[i].img_number = 0;
			continue;
		//狂攻撃のアニメーション
		}elif(player[i].status & 32) {
			//NEXT_TODO:ここから！！
			player[i].img_number = 0;
			continue;
		}
		player[i].img_number = 0;
	}
}


void Game::drawFadeIn(double t) const {
	
}

void Game::draw() const {
	Scene::SetBackground(ColorF(0.2, 0.8, 0.6));
	background_img.draw(0,0);
	draw_HP_bar();

	draw_player();
}

//キャラの描画
void Game::draw_player() const {
	for (int i = 0; i < player_sum;i++) {
		if (!player_flag[i]) continue;
		player_img.at(getData().player[i]).at(player[i].img_number).mirrored(player[i].direction).drawAt(player[i].pos[0]);
	}
}

void Game::draw_HP_bar() const {
	//1PのHP
	HP_bar_flame_img.draw(120, 70);
	HP_bar_gray_img.draw(130, 78);
	HP_bar_red_img(0, 0, 680 * (player[player_number].hp[1] / player_max_hp), 25).draw(130, 78);
	HP_bar_blue_img(0, 0, 680 * (player[player_number].hp[0] / player_max_hp), 25).draw(130, 78);

	//2PのHP
	HP_bar_flame_img.draw(1090, 70);
	HP_bar_gray_img.draw(1100, 78);
	HP_bar_red_img (680 * (1.0 - (player[another_player_number].hp[1] / player_max_hp)), 0, 680 * (player[another_player_number].hp[1] / player_max_hp), 25).mirrored().draw(1100, 78);
	HP_bar_blue_img(680 * (1.0 - (player[another_player_number].hp[0] / player_max_hp)), 0, 680 * (player[another_player_number].hp[0] / player_max_hp), 25).mirrored().draw(1100, 78);
}
