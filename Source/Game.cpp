#include "Game.hpp"

#define player_number 0

Game::Game(const InitData& init) : IScene(init),
player_img(player_sum), player_flag(player_sum,true)
{
	player_img.at(0).push_back(Texture{Unicode::Widen("../images/player1.png")});
	player_img.at(1).push_back(Texture{ Unicode::Widen("../images/player2.png")});

	internal_timer = Time::GetMillisec();
}

int Game::getkey() {
	int retkey = 0;
	retkey |= 1 * KeyW.pressed();
	retkey |= 2 * KeyA.pressed();
	retkey |= 4 * KeyS.pressed();
	retkey |= 8 * KeyD.pressed();

	retkey |= 1*KeyUp.pressed();
	retkey |= 2*KeyLeft.pressed();
	retkey |= 4*KeyDown.pressed();
	retkey |= 8*KeyRight.pressed();
	return retkey;
}


void Game::update() {
	update_player();
}

void Game::update_player() {
	//プレイヤー(ユーザー操作)の移動処理////////////////////////////////////////////////////////////
	if (int gotkey = getkey()) {
		//TODO:左右の移動も時間で管理する!!
		if (gotkey&2) player_pos[player_number] += go_left * player_speed[player_number];
		if (gotkey&8) player_pos[player_number] += go_right * player_speed[player_number];
		if ((gotkey&1)&&(player_status[player_number] != 3)) {
			player_status[player_number] = 3;
			player_timer[player_number] = Time::GetMillisec();
		}
		player_pos[player_number].x = Clamp(player_pos[player_number].x, 50.0, 1850.0);
	}

	if (player_status[player_number] & 3) {
		if (Time::GetMillisec() - player_timer[player_number] < 500) {
			int t = Time::GetMillisec() - player_timer[player_number];
			player_pos[player_number].y = player_min_y - 2.0 * t + 0.004 * t * t;
		}else {
			player_status[player_number] ^= 3;
			player_pos[player_number].y = player_min_y;
		}
	}
	//プレイヤー(相手)の移動処理////////////////////////////////////////////////////////////////////

	//TODO：いい感じに実装して

	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////

	//TODO:明日起きたらここをかけ

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////

	//TODO:画像来たらやれ

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
		player_img.at(i).at(0).drawAt(player_pos[i]);
	}
}
