#include "Game.hpp"

#define player_number 0
#define another_player_number 1

using namespace std;

Game::Game(const InitData& init) : IScene(init),
player_img(player_sum),
player_flag(player_sum, true)
{
	player_img.at(0).push_back(Texture{ Unicode::Widen("../images/player1.png") });
	player_img.at(1).push_back(Texture{ Unicode::Widen("../images/player2.png") });

	player_pos[0][0] = { 100.0,player_min_y };
	player_pos[1][0] = { 1200,player_min_y };
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


void Game::update() {
	update_player();
}

void Game::update_player() {
	int now_time = Time::GetMillisec();
	//プレイヤー(ユーザー操作)の移動処理////////////////////////////////////////////////////////////
	//左右移動処理
	if (player_status[player_number] & 3) {
		int t = now_time - player_timer[player_number][0];
		if (t < 50) {
			player_pos[player_number][0].x = player_pos[player_number][1].x + ((player_status[player_number] & 1) ? -1.0 : 1.0) * player_speed[player_number] * t / 50;
		}else {
			player_status[player_number] ^= (player_status[player_number]&1)?1:2;
		}
	}
	//ジャンプ処理
	if (player_status[player_number] & 4) {
		int t = now_time - player_timer[player_number][2];
		if (t < 500) {
			player_pos[player_number][0].y = player_min_y - 2.0 * t + 0.004 * t * t;
		}
		else {
			player_status[player_number] ^= 4;
			player_pos[player_number][0].y = player_min_y;
		}
	}
	//キー入力処理
	if (int gotkey = getkey()) {
		if ((player_status[player_number] & 3) == 0){
			if (gotkey & 10) {
				player_status[player_number] |= (gotkey & 2)?1:2;
				player_timer[player_number][0] = now_time;
				player_pos[player_number][1].x = player_pos[player_number][0].x;
			}
		}
		if ((gotkey & 1) && ((player_status[player_number] & 4) == 0)) {
			player_status[player_number] |= 4;
			player_timer[player_number][2] = now_time;
			player_pos[player_number][1].y = player_pos[player_number][0].y;
		}
	}
	//プレイヤー(相手)の移動処理////////////////////////////////////////////////////////////////////

	//TODO：いい感じに実装して

	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (i == player_number) continue;
		if ((abs(player_pos[player_number][0].x - player_pos[i][0].x) < 40.0)&&(abs(player_pos[player_number][0].y - player_pos[i][0].y) < 242.0)) {
			if (player_status[i]&3) {
				//TODO
			}else {
				player_pos[i][0].x += (player_pos[player_number][0].x < player_pos[i][0].x) ? 16.0 : -16.0;
			}
		}
	}

	//プレイヤーの向き
	if (player_pos[player_number][0].x < player_pos[another_player_number][0].x) {
		player_direction[player_number] = false;
		player_direction[another_player_number] = true;
	}else {
		player_direction[player_number] = true;
		player_direction[another_player_number] = false;
	}

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////

	//TODO:画像来たらやれ

	//移動範囲制限
	player_pos[player_number][0].x = Clamp(player_pos[player_number][0].x, 50.0, 1850.0);
}

void Game::drawFadeIn(double t) const {

}

void Game::draw() const {
	Scene::SetBackground(ColorF(0.2, 0.8, 0.6));
	background_img.draw(0,0);
	draw_player();
}

void Game::draw_player() const {
	for (int i = 0; i < player_sum;i++) {
		if (!player_flag[i]) continue;
		player_img.at(i).at(0).mirrored(player_direction[i]).drawAt(player_pos[i][0]);
	}
}
