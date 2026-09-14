#include "Game.hpp"
# include "common_function.hpp"

# include <algorithm>
# include <ranges>

//音声コマンド
//共通//////////////////////////////////////

//「ガード」
#define ガード1 U"AO"
//「守れ」
#define ガード2 U"AOE" // ガードに吸われる
//「壊せ」
#define ガード破壊共通 U"OAE"
//玲////////////////////////////////////////

//「殴れ」
#define 玲_弱攻撃 U"AUE"
//「キック」
#define 玲_狂攻撃 U"IU"
//「魚雷」
#define 玲_特殊攻撃 U"OAI"
//「龍虎水雷撃」
#define 玲_必殺技 U"AIEI" //U"UOUIAIEI"
//「刺せ」
#define 玲_ガード破壊 U"AE"

//ユウカ////////////////////////////////////

//「殴れ」
#define ユウカ_弱攻撃 U"AUE"
//「キック」
#define ユウカ_狂攻撃 U"IU"
//「龍虎水雷撃」
#define ユウカ_必殺技 U"AIEI" //U"UOUIAIEI"
//「刺せ」
#define ユウカ_ガード破壊 U"AE"
//アイリ////////////////////////////////////

//「撃て」
#define アイリ_弱攻撃 U"UE"
//「斬れ」
#define アイリ_狂攻撃 U"IE"
//「デッドリーアサルト」
#define アイリ_必殺技 U"IAUO" //U"EOIAUO"
//「連射」
#define アイリ_特殊攻撃 U"EIA"
//「刺せ」
#define アイリ_ガード破壊 U"AE"
//No.0////////////////////////////////////

//「殴れ」
#define No0_弱攻撃 U"AUE"
//「キック」
#define No0_狂攻撃 U"IU"
//「龍虎水雷撃」
#define No0_必殺技 U"AIEI" //U"UOUIAIEI"
//「刺せ」
#define No0_ガード破壊 U"AE"

using namespace std;

//マクロ
#define search(p1) int p1##_number = -1; for (int iter = 0; iter < max_##p1; iter++) { if (!p1[iter].exist) { p1[iter].exist = true; p1##_number = iter;break; } };

Game::Game(const InitData& init) : IScene(init),
player_img(4),
command_img(4),
fire_img(5),
player_flag(player_sum, true)
{
	//システムで使用する画像
	for (int i = 0; i < 5; i++)
		fire_img.at(i) = Texture{ Resource(Unicode::Widen("images/game/system/fire" + to_string(i) + ".png")) };

	//玲の画像
	if ((getData().player[0] == 0) || (getData().player[1] == 0)) {
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/waiting.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/running.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/special_attack.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/strong_attack.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/weak_attack.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/super_attack.png")) });
		player_img.at(0).push_back(Texture{ Resource(Unicode::Widen("images/game/0/destroy_guard.png")) });
	}
	//ユウカの画像
	if ((getData().player[0] == 1) || (getData().player[1] == 1)) {
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/waiting.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/running.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/special_kick.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/kick.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/weak_attack.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/powerful_kick.png")) });
		player_img.at(1).push_back(Texture{ Resource(Unicode::Widen("images/game/1/destroy_guard.png")) });
	}
	//アイリの画像
	if ((getData().player[0] == 2) || (getData().player[1] == 2)) {
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/waiting.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/running.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/special_gun.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/strong_knife.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/gun.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/beautiful_knife.png")) });
		player_img.at(2).push_back(Texture{ Resource(Unicode::Widen("images/game/2/destroy_guard.png")) });
	}
	//No.0の画像
	if ((getData().player[0] == 3) || (getData().player[1] == 3)) {
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/waiting.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/running.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/special_attack.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/strong_attack.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/weak_attack.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/super_attack.png")) });
		player_img.at(3).push_back(Texture{ Resource(Unicode::Widen("images/game/3/destroy_guard.png")) });
	}
	//コマンド画像
	command_img.at(0) = Texture{ Resource(Unicode::Widen("images/game/system/command_rei.png")) };
	command_img.at(1) = Texture{ Resource(Unicode::Widen("images/game/system/command_yuuka.png")) };
	command_img.at(2) = Texture{ Resource(Unicode::Widen("images/game/system/command_airi.png")) };
	command_img.at(3) = Texture{ Resource(Unicode::Widen("images/game/system/command_no0.png")) };

	//録音開始!
	getData().phoneme.start();

	player[0].pos[0] = { 600.0,player_min_y };
	player[1].pos[0] = { 1200,player_min_y };
	player[1].direction = true;
	player[0].number = getData().player[0];
	player[1].number = getData().player[1];

	internal_timer = (int)Time::GetMillisec();
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

int Game::voice_command() {
# if defined(_DEBUG) || defined(DEBUG)
	//デバッグ用に、声のコマンドをキーでも発動できるようにする
	const int character = getData().player[player_number];
	if (KeyB.down()) return 1;
	if (KeyV.down()) return 2;
	if (KeyF.down()) return 3;
	if (KeyG.down()) return 4;
	if (KeyC.down()) return 5;
	if (KeyE.down() && ((character == 0) || (character == 2))) return 6;
# endif
	HashTable<char32, double> scores;
	for (const auto& [id, score] : getData().phoneme.estimate() | views::enumerate) {
		const auto vowel = getData().vowels[id];
		if (vowel != U' ') scores[vowel] = max(scores[vowel], pow(score, 2) * (score >= 0.0 ? 1.0 : -1.0));
	}
	wordDetector.addScores(scores);
	if (getData().player[player_number] == 0) {
		if (wordDetector.detect(玲_弱攻撃))return 1;
		if (wordDetector.detect(玲_狂攻撃))return 2;
		if (wordDetector.detect(玲_必殺技))return 3;
		if (wordDetector.detect(玲_ガード破壊))return 5;
		if (wordDetector.detect(玲_特殊攻撃))return 6;
	}elif(getData().player[player_number] == 1) {
		if (wordDetector.detect(ユウカ_弱攻撃))return 1;
		if (wordDetector.detect(ユウカ_狂攻撃))return 2;
		if (wordDetector.detect(ユウカ_必殺技))return 3;
		if (wordDetector.detect(ユウカ_ガード破壊))return 5;
	}elif(getData().player[player_number] == 2) {
		if (wordDetector.detect(アイリ_弱攻撃))return 1;
		if (wordDetector.detect(アイリ_狂攻撃))return 2;
		if (wordDetector.detect(アイリ_必殺技))return 3;
		if (wordDetector.detect(アイリ_ガード破壊))return 5;
		if (wordDetector.detect(アイリ_特殊攻撃))return 6;
	}
	else {
		if (wordDetector.detect(No0_弱攻撃))return 1;
		if (wordDetector.detect(No0_狂攻撃))return 2;
		if (wordDetector.detect(No0_必殺技))return 3;
		if (wordDetector.detect(No0_ガード破壊))return 5;
	}
	if (wordDetector.detect(ガード1))return 4;
	if (wordDetector.detect(ガード2))return 4;
	if (wordDetector.detect(ガード破壊共通))return 5;
	return 0;
}

void Game::update_error_screen() {
	//エラーダイアログ
	if (error_mode == 1) {
		error_timer = (int)Time::GetMillisec();
		error_mode = 2;
		return;
	}elif(error_mode == 2) {
		int now_time = (int)Time::GetMillisec();
		if (now_time - error_timer <= 200) {
			error_pos_y = 1400 - 1040 * (now_time - error_timer) / 200;
			back_alpha = 0.8 * (now_time - error_timer) / 200;
		}
		else {
			error_pos_y = 360;
			back_alpha = 0.8;
			error_mode = 3;
		}
		return;
	}elif(error_mode == 3) {
		if (OK_shape.mouseOver())  Cursor::RequestStyle(CursorStyle::Hand);
		if (Yes_shape.mouseOver()) Cursor::RequestStyle(CursorStyle::Hand);
		//タイトルに戻る
		if (OK_shape.leftClicked() || Yes_shape.leftClicked()) {
			cancel_sound.playOneShot();
			getData().before_scene = State::Game;
			changeScene(State::Title, 0.8s);
		}
		return;
	}
}

void Game::showError(const Multiplay::APIError& error) {
	error_mode = 1;
	error_ID = 2;
	OutputLogFile("(" + error.code.narrow() + ")\n" + error.message.narrow());
}

void Game::finish_game(bool won) {
	is_game_finished = true;
	are_you_winnner = won;
	settle_timer = GameTimer();
}

void Game::updateFadeIn(double) {
	getData().room->update();
}

inline int Game::GameTimer() {
	return (int)Time::GetMillisec() - connection_timer;
}

void Game::update() {
	if (error_mode) {
		update_error_screen();
		return;
	}
#ifndef debug_mode
	//ゲーム開始時間の調整
	if (!is_connected) {
		auto& room = *getData().room;
		room.update();
		if (room.error()) {
			showError(*room.error());
		}elif(room.lastTick() && (getData().start_tick <= *room.lastTick())) {
			is_connected = true;
			//Player
			player_number = room.isOwner() ? 0 : 1;
			another_player_number = 1 - player_number;
			//ゲーム開始時刻
			connection_timer = (int)Time::GetMillisec() + 700;
			fade_back_timer = GameTimer();
			fade_back_alpha = 1.0;
			//BGMを流す
			bgm.play();
		}
	}
	else {
		//同期処理
		if (!is_game_finished)synchronizate_data();
		//フェードイン
		if (fade_back_alpha > 0.0) {
			fade_back_alpha = 1.0 - ((double)(GameTimer() - fade_back_timer)) / 700.0;
			if (fade_back_alpha < 0.0)fade_back_alpha = 0.0;
			return;
		}
#else
	if (!bgm.isPlaying()) bgm.play();
	for (int i = 0; i < player_sum; i++) {
		player[i].event = 0;
	}
#endif
	//プレイヤー情報を更新
	if (!is_game_finished)update_player();
	//APバーの描画情報を更新
	update_AP_bar_animation();
	//プレイヤーのアニメーションを更新
	update_player_animation();
	//各種エフェクトの更新
	update_effects();
	//決着！
	if (is_game_finished)update_settle();
#ifndef debug_mode
	}
#endif
}

void Game::update_effects() {
	int now_time = GameTimer();
	//エフェクトの更新
	for (int i = 0; i < max_occation; i++) {
		if (!occation[i].exist)continue;
		int t = now_time - occation[i].timer;
		if (t > 500) {
			occation[i].exist = false;
		}
		else {
			occation[i].alpha = 1.0 - EaseOutExpo((double)t / 500.0);
			occation[i].scale = 3.0 + 7.0 * EaseOutExpo((double)t / 500.0);
		}
	}
}

void Game::update_settle() {
	//決着！
	if (settle_mode == 0) {
		if (GameTimer() - settle_timer <= 1000) {
			settle_fade = EaseOutExpo((double)(GameTimer() - settle_timer) / 1000.0);
		}
		else {
			settle_timer = GameTimer();
			settle_fade = 1.0;
			settle_mode = 1;
		}
		//表示を続ける
	}elif(settle_mode == 1) {
		if (GameTimer() - settle_timer > 500) {
			settle_mode = 2;
			settle_timer = GameTimer();
		}
		//You Win/Loseの表示
	}elif(settle_mode == 2) {
		if (GameTimer() - settle_timer <= 1000) {
			settle_fade = 1.0 - EaseOutExpo((double)(GameTimer() - settle_timer) / 1000.0);
		}
		else {
			settle_fade = 0.0;
			settle_mode = 3;
			settle_timer = GameTimer();

		}
		//待機
	}elif(settle_mode == 3) {
		if (GameTimer() - settle_timer > 2000) {
			settle_mode = 4;
			settle_timer = GameTimer();
			finish_fade_mode = true;
			bgm.pause(2s);
		}
		//フェードアウトしながらタイトル画面へ
	}elif(settle_mode == 4) {
		double t = (double)(GameTimer() - settle_timer);
		finish_fade = Min(t / 2000.0, 1.0);
		if (t >= 2000.0) {
			getData().before_scene = State::Game;
			changeScene(State::Title, 0.8s);
		}
	}
}


void Game::update_player() {
	int now_time = GameTimer();
	Vec2 player_reserved_pos[player_sum];
	for (int i = 0; i < player_sum; i++)player_reserved_pos[i] = player[i].pos[0];
	//プレイヤーの移動処理////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		//左右移動処理
		if (player[i].status & 3) {
			player[i].timer[1] = now_time - player[i].timer[0];
			if (player[i].timer[1] < 100) {
				player_reserved_pos[i].x = player[i].pos[1].x + ((player[i].status & 1) ? -1.0 : 1.0) * player[i].speed * player[i].timer[1] / 100;
			}
			else {
				if (i == player_number) {
					player[i].status ^= (player[i].status & 1) ? 1 : 2;
				}
				else {
					player[i].timer[0] = now_time;
					player[i].timer[1] = 0;
					player[i].pos[1].x = player[i].pos[0].x;
				}
			}
		}
		//ジャンプ処理
		if (player[i].status & 4) {
			player[i].timer[7] = now_time - player[i].timer[2];
			if (player[i].timer[7] < 500) {
				player_reserved_pos[i].y = player_min_y - 2.0 * player[i].timer[7] + 0.004 * player[i].timer[7] * player[i].timer[7];
			}
			else {
				player[i].status ^= 4;
				player_reserved_pos[i].y = player_min_y;
			}
		}
	}
	//プレイヤー(ユーザー操作)のキー入力処理////////////////////////////////////////////////////////////
	if (int gotkey = getkey()) {
		//左右移動
		if ((player[player_number].status & 259) == 0) {
			if (gotkey & 10) {
				player[player_number].status |= (gotkey & 2) ? 1 : 2;
				player[player_number].timer[0] = now_time;
				player[player_number].timer[1] = 0;
				player[player_number].pos[1].x = player[player_number].pos[0].x;
				if (player[player_number].number == 2)
					player[player_number].walking = !player[player_number].walking;
			}
		}
		//ジャンプ
		if ((gotkey & 1) && ((player[player_number].status & 308) == 0)) {
			jump_se.playOneShot();
			player[player_number].status |= 4;
			player[player_number].timer[2] = now_time;
			player[player_number].timer[7] = 0;
			player[player_number].pos[1].y = player[player_number].pos[0].y;
		}
	}
	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////
	//位置を決めるのは本人なので、押し合いでも自分のキャラだけを動かす。押した相手は、相手のクライアントが自分で押し出す
	{
		const double contact = 70.0;
		//押す側がこの深さまでめり込むのを許す。接した位置で止めると、相手の画面で重ならず押せなくなる
		const double push_depth = 20.0;
		Vec2& self = player_reserved_pos[player_number];
		const Vec2& other = player_reserved_pos[another_player_number];
		const double self_x = self.x;
		//ジャンプ中はすれ違えるようにする
		const bool jumping = ((player[player_number].status | player[another_player_number].status) & 4);
		if ((!jumping) && (abs(self.x - other.x) < contact) && (abs(self.y - other.y) < 242.0)) {
			//重なり中の左右の見え方は通信の遅れで食い違うため、向きは動いている側の進行方向で決める
			const auto direction = [](const Player& p) { return (p.status & 1) ? -1.0 : ((p.status & 2) ? 1.0 : 0.0); };
			const double self_direction = direction(player[player_number]);
			const double other_direction = direction(player[another_player_number]);
			const bool pushing = (0.0 < (other.x - self.x) * self_direction);
			const bool pushed = (0.0 < (self.x - other.x) * other_direction);
			if (pushing && pushed) {
				//向かい合ってぶつかったら進めない
				self.x = player[player_number].pos[0].x;
			}elif(pushing) {
				self.x = other.x - self_direction * Max((other.x - self.x) * self_direction, contact - push_depth);
			}elif(pushed) {
				self.x = other.x + other_direction * contact;
			}elif((self_direction == 0.0) && (other_direction == 0.0) && (player_number == 1)) {
				//止まったまま重なったら片方だけが離れる。両方が動くと、見え方の食い違いで同じ向きに逃げ続ける
				self.x = other.x + ((self.x < other.x) ? -contact : contact);
			}
		}
		//相手は歩き始めの位置と進み具合から位置を計算するので、押し合いで動かした分を歩き始めの位置にも反映する
		player[player_number].pos[1].x += (self.x - self_x);
	}

	//プレイヤーの向き
	if (player_reserved_pos[player_number].x < player[another_player_number].pos[0].x) {
		player[player_number].direction = false;
		player[another_player_number].direction = true;
	}
	else {
		player[player_number].direction = true;
		player[another_player_number].direction = false;
	}
	for (int i = 0; i < player_sum; i++) {
		if (player[i].status & 3)player[i].direction = (player[i].status & 1);
	}
	//技とかの起爆/////////////////////////////////////////////////////////////////////////////////
	if (int got_voice = voice_command()) {
		//弱攻撃
		if (got_voice == 1) {
			if ((player[player_number].status & 500) == 0) {
				shot_se.playOneShot();
				player[player_number].se[2] = true;
				player[player_number].status |= 16;
				player[player_number].timer[4] = now_time;
			}
			//狂攻撃
		}elif(got_voice == 2) {
			if ((player[player_number].status & 500) == 0) {
				shot_se.playOneShot();
				player[player_number].se[3] = true;
				player[player_number].status |= 32;
				player[player_number].timer[5] = now_time;
			}
			//必殺技
		}elif((got_voice == 3) && (player[player_number].special_attack)) {
			if ((player[player_number].status & 500) == 0) {
				bom_se.playOneShot();
				player[player_number].se[4] = true;
				player[player_number].status |= 64;
				player[player_number].timer[6] = now_time;
				player[player_number].ap = 0;
				player[player_number].special_attack = false;
			}
			//ガード
		}elif(got_voice == 4) {
			guard_se.playOneShot();
			player[player_number].se[5] = true;
			player[player_number].status |= 8;
			player[player_number].timer[3] = now_time;
#ifndef debug_mode
			getData().room->sendAction(U"Guard", player_number);
#endif
			//ガード破壊
		}elif(got_voice == 5) {
			if ((player[player_number].status & 500) == 0) {
				shot_se.playOneShot();
				player[player_number].se[6] = true;
				player[player_number].status |= 128;
				player[player_number].timer[12] = now_time;
			}
			//特殊攻撃
		}elif(got_voice == 6) {
			if ((player[player_number].status & 500) == 0) {
				player[player_number].se[7] = true;
				player[player_number].status |= 256;
				player[player_number].timer[14] = now_time;
			}
		}

	}

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////
	for (int cnt = 0; cnt < player_sum; cnt++) {
		if (player[cnt].number == 0) {
			rei_attack(cnt, now_time, player_reserved_pos);
		}elif(player[cnt].number == 1) {
			yuuka_attack(cnt, now_time, player_reserved_pos);
		}elif(player[cnt].number == 2) {
			airi_attack(cnt, now_time, player_reserved_pos);
		}elif(player[cnt].number == 3) {
			no0_attack(cnt, now_time, player_reserved_pos);
		}

		//ガード破壊
		if (player[cnt].status & 128) {
			int t = now_time - player[cnt].timer[12];
			if ((100 < t) && (t < 250)) {
				for (int i = 0; i < player_sum; i++) {
					if (i == cnt) continue;
					int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
					if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
						if ((player[i].event & 8) == 0) {
							if (player[cnt].se[6]) {
								player[cnt].se[6] = false;
								dos_se.playOneShot();
							}
							player[i].event |= 8;
							if (player[i].status & 8) {
								break_guard_se.playOneShot();
								player[i].status ^= 8;
							}
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"DestroyGuard", i);
#endif
							player[i].hp[1] -= 1;
							player[cnt].ap += 1;
						}
					}
				}
			}
		}
		//シールド
		if (player[cnt].status & 8) {
			int t = now_time - player[cnt].timer[3];
			//ガード時間は1秒
			if (t > 1000) {
				player[cnt].status ^= 8;
#ifndef debug_mode
				//相手の時計で先に解除されないよう、ガードした本人だけが送る
				if (cnt == player_number)
					getData().room->sendAction(U"VoidGuard", cnt);
#endif
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
		player[i].hp[1] = Max(player[i].hp[1], 0);
		if (player[i].hp[2] > player[i].hp[0])player[i].hp[2]--;
		if (player[i].hp[0] <= 0) {
			player[i].hp[0] = 0;
			player[i].hp[2] = 0;
		}
	}


	for (int i = 0; i < player_sum; i++) {
		//移動範囲制限
		player_reserved_pos[i].x = Clamp(player_reserved_pos[i].x, 50.0, 1850.0);
		//プレイヤーの位置を更新を確定
		player[i].pos[0] = player_reserved_pos[i];
	}
}

void Game::call_bullet(int cnt, int now_time, Vec2 player_reserved_pos[], int type) {
	//銃攻撃
	if (player[cnt].status & (16 * (int)pow(2, type))) {
		player[cnt].timer[9 + type] = now_time - player[cnt].timer[4 + type];
		if ((180 < player[cnt].timer[9 + type]) && player[cnt].se[2 + type]) {
			player[cnt].se[2 + type] = false;
			gun_se.playOneShot();
			//銃弾の発生
			search(bullet);
			if (bullet_number != -1) {
				bullet[bullet_number].pos = player_reserved_pos[cnt] + Vec2{ sign(!player[cnt].direction) * 140 + ((type == 1) ? 55 : 0),what<int>(type,-115,-66,-90) };
				bullet[bullet_number].old_pos = bullet[bullet_number].pos;
				bullet[bullet_number].direction = !player[cnt].direction;
				bullet[bullet_number].angle = (bullet[bullet_number].direction ? 0.0 : M_PI);
				bullet[bullet_number].timer = now_time;
				bullet[bullet_number].mode = 0;
				bullet[bullet_number].type = (type == 2) ? 3 : type;
				bullet[bullet_number].character = player[cnt].number;
				bullet[bullet_number].disable_disappear = (type == 2);
			}
		}
	}
}

void Game::rei_attack(int cnt, int now_time, Vec2 player_reserved_pos[]) {
	//銃弾の移動+当たり判定
	for (int i = 0; i < max_bullet; i++) {
		if (!bullet[i].exist)continue;
		if (bullet[i].character != player[cnt].number)continue;
		//銃弾の移動
		//弱
		if (bullet[i].type == 0) {
			bullet[i].pos.x = bullet[i].old_pos.x + sign(bullet[i].direction) * (now_time - bullet[i].timer) * 3.0;
			//狂
		}elif(bullet[i].type == 1) {
			if (now_time - bullet[i].timer < 250) {
				bullet[i].pos.x = bullet[i].old_pos.x + sign(bullet[i].direction) * (now_time - bullet[i].timer) * 1.5;
			}
			else {
				//爆発
				bomber_se.playOneShot();
				//エフェクトの発生
				search(occation);
				if (occation_number != -1) {
					occation[occation_number].pos = bullet[i].pos;
					occation[occation_number].timer = now_time;
					occation[occation_number].alpha = 1.0;
					occation[occation_number].scale = 3.0;
					occation[occation_number].type = 0;
				}

				//当たり判定
				if (abs(player_reserved_pos[another_player_number].x - bullet[i].pos.x) < 60.0) {
					if (player[another_player_number].status & 8) {
						void_damage_se.playOneShot();
					}
					else {
#ifndef debug_mode
						if (cnt == player_number)
							getData().room->sendAction(U"StrongAttackBomb", another_player_number);
#endif
						player[another_player_number].hp[1] -= rei_strong_attack_bomb;
						player[cnt].ap += rei_strong_attack_ap;
					}
				}
				else {
					for (int j = 0; j < 6; j++) {
						search(bullet);
						if (bullet_number == -1)break;
						bullet[bullet_number].pos = bullet[i].pos;
						bullet[bullet_number].old_pos = bullet[bullet_number].pos;
						bullet[bullet_number].angle = (M_PI / 3.0) * j;
						bullet[bullet_number].old_angle = bullet[bullet_number].angle;
						bullet[bullet_number].direction = bullet[i].direction;
						bullet[bullet_number].timer = now_time;
						bullet[bullet_number].mode = 0;
						bullet[bullet_number].type = 2;
						bullet[bullet_number].character = player[cnt].number;
					}
				}
				bullet[i].exist = false;
			}
			//狂(爆発後)
		}elif(bullet[i].type == 2) {
			bullet[i].pos.x = bullet[i].old_pos.x + cos(bullet[i].angle) * (now_time - bullet[i].timer) * 2.0;
			bullet[i].pos.y = bullet[i].old_pos.y + sin(bullet[i].angle) * (now_time - bullet[i].timer) * 2.0;
			bullet[i].angle = bullet[i].old_angle - (bullet[i].direction ? -1.0 : 1.0) * (now_time - bullet[i].timer) * (M_PI / 3000);
			//必殺(第一段階)
		}elif(bullet[i].type == 3) {
			bullet[i].angle = -(bullet[i].direction ? M_PI / 15.0 : M_PI * 14.0 / 15.0);
			bullet[i].pos.x = bullet[i].old_pos.x + 2.5 * (now_time - bullet[i].timer) * cos(bullet[i].angle);
			bullet[i].pos.y = bullet[i].old_pos.y + 2.5 * (now_time - bullet[i].timer) * sin(bullet[i].angle);
			//第二段階(反射)
			if ((bullet[i].pos.x < 10) || (bullet[i].pos.x > 1910)) {
				gun_reflect1_se.playOneShot();
				bullet[i].type = 4;
				bullet[i].angle = M_PI - bullet[i].angle;
				bullet[i].old_pos = bullet[i].pos;
				bullet[i].timer = now_time;
			}
			//必殺(第二、三段階)
		}elif((bullet[i].type == 4) || (bullet[i].type == 5)) {
			bullet[i].pos.x = bullet[i].old_pos.x + 4.2 * (now_time - bullet[i].timer) * cos(bullet[i].angle);
			bullet[i].pos.y = bullet[i].old_pos.y + 4.2 * (now_time - bullet[i].timer) * sin(bullet[i].angle);
			//反射
			if ((bullet[i].pos.y < 0) || (bullet[i].pos.x < 0) || (bullet[i].pos.x > 1920)) {
				if (bullet[i].type == 4) {
					gun_reflect2_se.playOneShot();
					bullet[i].angle = 2.0 * M_PI - bullet[i].angle;
					bullet[i].old_pos = bullet[i].pos;
					bullet[i].timer = now_time;
					//第三段階(反射して相手へ)
				}
				else {
					gun_reflect3_se.playOneShot();
					bullet[i].angle = atan2(player_reserved_pos[another_player_number].y - 80.0 - bullet[i].pos.y, player_reserved_pos[another_player_number].x - bullet[i].pos.x);
					bullet[i].old_pos = bullet[i].pos;
					bullet[i].timer = now_time;
				}
				bullet[i].type++;
			}
			//必殺(最終段階:高速で相手へ)
		}elif(bullet[i].type == 6) {
			bullet[i].pos.x = bullet[i].old_pos.x + 6.0 * (now_time - bullet[i].timer) * cos(bullet[i].angle);
			bullet[i].pos.y = bullet[i].old_pos.y + 6.0 * (now_time - bullet[i].timer) * sin(bullet[i].angle);
		}

		//残像の発生
		if ((3 <= bullet[i].type) && (bullet[i].type <= 6)) {
			search(after_images);
			if (after_images_number != -1) {
				after_images[after_images_number].pos = bullet[i].pos;
				after_images[after_images_number].timer = now_time;
				after_images[after_images_number].alpha = 1.0;
				after_images[after_images_number].angle = bullet[i].angle;
			}
		}

		//場外退場
		if ((!bullet[i].disable_disappear) && ((bullet[i].pos.x < 0) || (bullet[i].pos.x > 1920) || (bullet[i].pos.y < 0) || (bullet[i].pos.y > 1080)))
			bullet[i].exist = false;

		//当たり判定
		for (int j = 0; j < player_sum; j++) {
			if (j == cnt)continue;
			//弱攻撃
			if (bullet[i].type <= 1) {
				//頭を下げればぶつかりません～♪
				if ((abs(player_reserved_pos[j].x - bullet[i].pos.x) < 40.0) && ((player[j].status & 3) == 0)) {
					if ((player[j].event & 1) == 0) {
						player[j].event |= 1;
						if (player[j].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"WeakAttack", j);
#endif
							player[j].hp[1] -= rei_weak_atttack;
							player[cnt].ap += rei_weak_atttack_ap;
						}
						bullet[i].exist = false;
						break;
					}
				}
			}elif(bullet[i].type == 2) {
				if ((abs(player_reserved_pos[j].x - bullet[i].pos.x) < 40.0) && (abs(player_reserved_pos[j].y - bullet[i].pos.y) < 195.0)) {
					if (player[j].status & 8) {
						void_damage_se.playOneShot();
					}
					else {
#ifndef debug_mode
						if (cnt == player_number)
							getData().room->sendAction(U"StrongAttack", j);
#endif
						player[j].hp[1] -= rei_strong_attack;
						player[cnt].ap += rei_strong_attack_ap;
					}
					bullet[i].exist = false;
					break;
				}
				//必殺技(最終段階)
			}elif(bullet[i].type == 6) {
				if ((abs(player_reserved_pos[j].x - bullet[i].pos.x) < 50.0) && (abs(player_reserved_pos[j].y - bullet[i].pos.y) < 195.0)) {
					//爆発
					bomber_se.playOneShot();
					//エフェクトの発生
					search(occation);
					if (occation_number != -1) {
						occation[occation_number].pos = bullet[i].pos;
						occation[occation_number].timer = now_time;
						occation[occation_number].alpha = 1.0;
						occation[occation_number].scale = 3.0;
						occation[occation_number].type = 1;
					}

					if (player[j].status & 64) {
						void_damage_se.playOneShot();
					}
					else {
#ifndef debug_mode
						if (cnt == player_number)
							getData().room->sendAction(U"SpecialAttack", j);
#endif
						player[j].hp[1] -= rei_special_attack;
						player[cnt].ap += rei_special_attack_ap;
					}
					bullet[i].exist = false;
					break;
				}
			}
		}
	}

	//魚雷の移動+当たり判定
	for (int i = 0; i < max_torpedo; i++) {
		if (!torpedo[i].exist)continue;
		//魚雷の移動
		if (torpedo[i].mode == 0) {
			int t = now_time - torpedo[i].timer;
			if (t < 200) {
				torpedo[i].pos.x = torpedo[i].old_pos.x + 0.8 * sign(torpedo[i].angle == 0.0) * t;
				torpedo[i].pos.y = torpedo[i].old_pos.y - 0.44 * t + 0.008 * t * t;
			}
			else {
				torpedo[i].mode = 1;
				torpedo[i].timer = now_time;
				torpedo[i].pos = Vec2{ torpedo[i].old_pos.x + sign(torpedo[i].angle == 0.0) * 160,torpedo[i].old_pos.y + 232 };
				torpedo[i].old_pos = torpedo[i].pos;
				torpedo[i].angle = (torpedo[i].angle == 0.0) ? atan2(-0.07, 4.0) : M_PI - atan2(-0.07, 4.0);
			}
		}
		else {
			torpedo[i].pos.x = torpedo[i].old_pos.x + sign(!player[cnt].direction) * 4.0 * (now_time - torpedo[i].timer);
			torpedo[i].pos.y = torpedo[i].old_pos.y - 0.07 * (now_time - torpedo[i].timer);
			//場外退場
			if ((torpedo[i].pos.x < -80) || (torpedo[i].pos.x > 2000))torpedo[i].exist = false;
		}
		//当たり判定
		for (int j = 0; j < player_sum; j++) {
			if (j == cnt)continue;
			if ((abs(player_reserved_pos[j].x - torpedo[i].pos.x) < 60.0) && ((player[j].status & 4) == 0)) {
				if (player[j].status & 8) {
					void_damage_se.playOneShot();
				}
				else {
#ifndef debug_mode
					if (cnt == player_number)
						getData().room->sendAction(U"UniqueAttack", j);
#endif
					player[j].hp[1] -= rei_uniqe_attack;
					player[cnt].ap += rei_uniqe_attack_ap;

					//爆発
					bomber_se.playOneShot();
					//エフェクトの発生
					search(occation);
					if (occation_number != -1) {
						occation[occation_number].pos = torpedo[i].pos;
						occation[occation_number].timer = now_time;
						occation[occation_number].alpha = 1.0;
						occation[occation_number].scale = 3.0;
						occation[occation_number].type = 2;
					}
				}
				torpedo[i].exist = false;
				break;
			}
		}
	}


	//残像の更新
	for (int i = 0; i < max_after_images; i++) {
		if (!after_images[i].exist)continue;
		int t = now_time - after_images[i].timer;
		if (t > 500) {
			after_images[i].exist = false;
		}
		else {
			after_images[i].alpha = 1.0 - EaseOutExpo((double)t / 500.0);
		}
	}

	//弱攻撃
	call_bullet(cnt, now_time, player_reserved_pos, 0);
	//狂攻撃(爆発する銃)
	call_bullet(cnt, now_time, player_reserved_pos, 1);
	//必殺技(反射する銃)
	call_bullet(cnt, now_time, player_reserved_pos, 2);

	//独自技(魚雷)
	if (player[cnt].status & 256) {
		player[cnt].timer[15] = now_time - player[cnt].timer[14];
		if ((180 < player[cnt].timer[15]) && player[cnt].se[7]) {
			player[cnt].se[7] = false;
			torpedo_se.playOneShot();
			//魚雷の発生
			search(torpedo);
			if (torpedo_number != -1) {
				torpedo[torpedo_number].pos = player_reserved_pos[cnt] + Vec2{ sign(!player[cnt].direction) * 130,-75 };
				torpedo[torpedo_number].old_pos = torpedo[torpedo_number].pos;
				torpedo[torpedo_number].angle = (player[cnt].direction ? M_PI : 0.0);
				torpedo[torpedo_number].timer = now_time;
				torpedo[torpedo_number].mode = 0;
			}
		}
	}
}

void Game::yuuka_attack(int cnt, int now_time, Vec2 player_reserved_pos[]) {
	//弱攻撃
	if (player[cnt].status & 16) {
		player[cnt].timer[9] = now_time - player[cnt].timer[4];
		if ((100 < player[cnt].timer[9]) && (player[cnt].timer[9] < 250)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
					if ((player[i].event & 1) == 0) {
						if (player[cnt].se[2]) {
							player[cnt].se[2] = false;
							dos_se.playOneShot();
						}
						player[i].event |= 1;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"WeakAttack", i);
#endif
							player[i].hp[1] -= yuuka_weak_atttack;
						}
						player[cnt].ap += yuuka_weak_atttack_ap;
					}
				}
			}
		}
	}
	//狂攻撃+必殺技
	if (player[cnt].status & 96) {
		player[cnt].timer[(player[cnt].status & 32) ? 10 : 11] = now_time - player[cnt].timer[(player[cnt].status & 32) ? 5 : 6];
		int tmp = player[cnt].timer[(player[cnt].status & 32) ? 10 : 11];
		if ((200 < tmp) && (tmp < 400)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
					if ((player[i].event & 2) == 0) {
						//狂攻撃
						if (player[cnt].status & 32) {
							if (player[cnt].se[3]) {
								player[cnt].se[3] = false;
								dos_se.playOneShot();
							}
							player[i].event |= 2;
							if (player[i].status & 8) {
								void_damage_se.playOneShot();
							}
							else {
#ifndef debug_mode
								if (cnt == player_number)
									getData().room->sendAction(U"StrongAttack", i);
#endif
								player[i].hp[1] -= yuuka_strong_attack;
							}
							player[cnt].ap += yuuka_strong_attack_ap;
							//必殺技
						}
						else {
							if (player[cnt].se[4]) {
								player[cnt].se[4] = false;
								dododos_se.playOneShot();
							}
							player[i].event |= 4;
							if (player[i].status & 8) {
								void_damage_se.playOneShot();
							}
							else {
#ifndef debug_mode
								if (cnt == player_number)
									getData().room->sendAction(U"SpecialAttack", i);
#endif
								player[i].hp[1] -= yuuka_special_attack;
							}
						}
					}
				}
			}
		}
	}
	//TODO:特殊攻撃
}

void Game::airi_attack(int cnt, int now_time, Vec2 player_reserved_pos[]) {
	//銃弾の移動・当たり判定処理
	for (int i = 0; i < max_bullet; i++) {
		if (!bullet[i].exist)continue;
		if (bullet[i].character != player[cnt].number)continue;
		bullet[i].pos.x = bullet[i].old_pos.x + sign(bullet[i].direction) * (now_time - bullet[i].timer) * 3.0;
		if ((bullet[i].pos.x < 0) || (bullet[i].pos.x > 1920))bullet[i].exist = false;
		for (int j = 0; j < player_sum; j++) {
			if (j == cnt)continue;
			if ((abs(player_reserved_pos[j].x - bullet[i].pos.x) < 40.0) && ((player[j].status & 3) == 0)) {
				if ((player[j].event & 1) == 0) {
					player[j].event |= 1;
					if (player[j].status & 8) {
						void_damage_se.playOneShot();
					}
					else {
						//弱
						if (bullet[i].mode == 0) {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"WeakAttack", j);
#endif
							player[j].hp[1] -= airi_weak_atttack;
							player[cnt].ap += airi_weak_atttack_ap;
							//特殊
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"UniqueAttack", j);
#endif
							player[j].hp[1] -= airi_uniqe_attack;
							player[cnt].ap += airi_uniqe_attack_ap;
						}

					}
					bullet[i].exist = false;
					break;
				}
			}
		}
	}
	//ナイフの移動・当たり判定処理
	for (int i = 0; i < max_knife; i++) {
		if (!knife[i].exist)continue;
		//待機
		if (knife[i].mode == 0) {
			if (now_time - knife[i].timer[0] > 500) {
				knife[i].mode = 1;
				knife[i].timer[1] = now_time;
				knife[i].angle[2] = atan2(knife[i].goal_pos.y - knife[i].pos.y, knife[i].goal_pos.x - knife[i].pos.x);
			}
			//発射
		}
		else {
			double t = now_time - knife[i].timer[1];
			double distance = sqrt(pow(knife[i].goal_pos.x - knife[i].pos.x, 2) + pow(knife[i].goal_pos.y - knife[i].pos.y, 2));
			if (distance < 32.0) {
				knife[i].horming = false;
			}elif(knife[i].horming) {
				knife[i].angle[2] = atan2(knife[i].goal_pos.y - knife[i].pos.y, knife[i].goal_pos.x - knife[i].pos.x);
				double tmp_angle = knife[i].angle[2] - knife[i].angle[1];
				// 角度差を-π～πの間に収める
				if (tmp_angle > M_PI) tmp_angle -= 2.0 * M_PI;
				if (tmp_angle < -M_PI) tmp_angle += 2.0 * M_PI;
				knife[i].angle[0] = knife[i].angle[1] + tmp_angle * EaseOutExpo(Min(t / knife[i].time, 1.0));
			}
			//ナイフの移動
			knife[i].pos += (Scene::DeltaTime()) * 1800.0 * Vec2{ cos(knife[i].angle[0]), sin(knife[i].angle[0]) };
			//角度を-π～πの間に収める
			if (knife[i].angle[0] > M_PI) knife[i].angle[0] -= 2.0 * M_PI;
			if (knife[i].angle[0] < -M_PI) knife[i].angle[0] += 2.0 * M_PI;
			//画面外に出たら退場
			if ((t > knife[i].time) && ((knife[i].pos.x < 0) || (knife[i].pos.x > 1920) || (knife[i].pos.y < 0) || (knife[i].pos.y > 1080)))
				knife[i].exist = false;
			//当たり判定処理
			int distance_x = abs(player_reserved_pos[another_player_number].x - knife[i].pos.x);
			int distance_y = abs(player_reserved_pos[another_player_number].y - knife[i].pos.y);
			if ((distance_x < 20.0) && (distance_y < (((player[another_player_number].status & 3) == 0) ? 180.0 : 90.0))) {
				if (player[another_player_number].status & 8) {
					void_damage_se.playOneShot();
				}
				else {
#ifndef debug_mode
					if (cnt == player_number)
						getData().room->sendAction(U"SpecialAttack", another_player_number);
#endif
					player[another_player_number].hp[1] -= airi_special_attack;
					player[another_player_number].ap += airi_special_attack_ap;
				}
				knife[i].exist = false;
			}

		}
	}

	//弱攻撃(銃)
	call_bullet(cnt, now_time, player_reserved_pos, 0);

	//狂攻撃(ナイフ)
	if (player[cnt].status & 32) {
		player[cnt].timer[10] = now_time - player[cnt].timer[5];
		if ((200 < player[cnt].timer[10]) && (player[cnt].timer[10] < 400)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 130.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
					if ((player[i].event & 2) == 0) {
						if (player[cnt].se[3]) {
							player[cnt].se[3] = false;
							dos_se.playOneShot();
						}
						player[i].event |= 2;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"StrongAttack", i);
#endif
							player[i].hp[1] -= airi_strong_attack;
						}
						player[cnt].ap += airi_strong_attack_ap;
					}
				}
			}
		}
	}
	//必殺技(大量(25本)のナイフ)
	if (player[cnt].status & 64) {
		player[cnt].timer[11] = now_time - player[cnt].timer[6];
		if ((140 < player[cnt].timer[11]) && player[cnt].se[4]) {
			player[cnt].se[4] = false;
			setting_knife(cnt, now_time, player_reserved_pos, 0);
		}
		if ((180 < player[cnt].timer[11]) && (player[cnt].knife_mode <= 1)) {
			setting_knife(cnt, now_time, player_reserved_pos, 1);
		}
		if ((220 < player[cnt].timer[11]) && (player[cnt].knife_mode <= 2)) {
			setting_knife(cnt, now_time, player_reserved_pos, 2);
		}
		if ((260 < player[cnt].timer[11]) && (player[cnt].knife_mode <= 3)) {
			setting_knife(cnt, now_time, player_reserved_pos, 3);
		}
		if ((300 < player[cnt].timer[11]) && (player[cnt].knife_mode <= 4)) {
			setting_knife(cnt, now_time, player_reserved_pos, 4);
		}
	}
	//特殊攻撃(連射)
	if (player[cnt].status & 256) {
		player[cnt].timer[15] = now_time - player[cnt].timer[14];
		if (player[cnt].timer[15] > 150) {
			int tmp = now_time - player[cnt].airi_old_timer;
			if (tmp > 20) {
				player[cnt].airi_old_timer = now_time;
				//銃弾の発生
				search(bullet);
				if (bullet_number != -1) {
					bullet[bullet_number].pos = player_reserved_pos[cnt] + Vec2{ sign(!player[cnt].direction) * 60,-77 };
					bullet[bullet_number].old_pos = bullet[bullet_number].pos;
					bullet[bullet_number].direction = !player[cnt].direction;
					bullet[bullet_number].angle = (bullet[bullet_number].direction ? 0.0 : M_PI);
					bullet[bullet_number].timer = now_time;
					bullet[bullet_number].mode = 1;
					bullet[bullet_number].character = player[cnt].number;
				}
			}
		}
	}
}

void Game::setting_knife(int cnt, int now_time, Vec2 player_reserved_pos[], int now_number) {
	player[cnt].knife_mode = 1 + now_number;
	shot_se.playOneShot();
	double set_angle = (M_PI / 7.0) * (-6.0);
	double knife_angle = (M_PI * 2.0 / 5.0) * now_number;
	//ナイフ召喚x5
	for (int i = 0; i < 5; i++) {
		search(knife);
		if (knife_number == -1) break;
		knife[knife_number].pos = player_reserved_pos[cnt] + Vec2{ cos(set_angle) * 180.0, sin(set_angle) * 180.0 };
		knife[knife_number].old_pos = knife[knife_number].pos;
		knife[knife_number].angle[0] = knife_angle + (M_PI * 2 / 5.0) * i;
		//角度を-π～πの間に収める
		if (knife[knife_number].angle[0] > M_PI)knife[knife_number].angle[0] -= M_PI * 2.0;
		if (knife[knife_number].angle[0] < -M_PI)knife[knife_number].angle[0] += M_PI * 2.0;
		knife[knife_number].angle[1] = knife[knife_number].angle[0];
		knife[knife_number].timer[0] = now_time;
		knife[knife_number].mode = 0;
		knife[knife_number].horming = true;
		knife[knife_number].img_number = i;
		knife[knife_number].goal_pos = player_reserved_pos[(cnt == player_number) ? another_player_number : player_number];
		knife[knife_number].distance = sqrt(pow(knife[knife_number].goal_pos.x - knife[knife_number].pos.x, 2) + pow(knife[knife_number].goal_pos.y - knife[knife_number].pos.y, 2));
		knife[knife_number].time = knife[knife_number].distance / 1.8;
		set_angle += M_PI / 7.0;
	}
}


void Game::no0_attack(int cnt, int now_time, Vec2 player_reserved_pos[]) {
	//弱攻撃
	if (player[cnt].status & 16) {
		player[cnt].timer[9] = now_time - player[cnt].timer[4];
		if ((100 < player[cnt].timer[9]) && (player[cnt].timer[9] < 250)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
					if ((player[i].event & 1) == 0) {
						if (player[cnt].se[2]) {
							player[cnt].se[2] = false;
							dos_se.playOneShot();
						}
						player[i].event |= 1;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"WeakAttack", i);
#endif
							player[i].hp[1] -= no0_weak_atttack;
						}
						player[cnt].ap += no0_weak_atttack_ap;
					}
				}
			}
		}
	}

	//狂攻撃
	if (player[cnt].status & 32) {
		player[cnt].timer[10] = now_time - player[cnt].timer[5];
		if ((200 < player[cnt].timer[10]) && (player[cnt].timer[10] < 400)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 130.0) && (abs(player_reserved_pos[cnt].y - player_reserved_pos[i].y) < 242.0)) {
					if ((player[i].event & 2) == 0) {
						if (player[cnt].se[3]) {
							player[cnt].se[3] = false;
							dos_se.playOneShot();
						}
						player[i].event |= 2;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							if (cnt == player_number)
								getData().room->sendAction(U"StrongAttack", i);
#endif
							player[i].hp[1] -= no0_strong_attack;
						}
						player[cnt].ap += no0_strong_attack_ap;
					}
				}
			}
		}
	}
}

//APバーのアニメーションを更新
void Game::update_AP_bar_animation() {
	int now_time = GameTimer();
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

void Game::synchronizate_data() {
	//同期
	auto& room = *getData().room;
	try {
		const auto& me = player[player_number];
		room.sendReport(U"PlayerInfoPos", Array<double>{ me.pos[0].x, me.pos[0].y, me.pos[1].x, me.pos[1].y });
		//取りこぼしても次で上書きされるよう、変化の有無にかかわらず毎フレーム送る
		room.sendReport(U"PlayerStatus", me.status);
		//届くまでに経った分を相手が補えるよう、送った時点のゲーム内時刻を添える
		room.sendReport(U"PlayerInfoTimer", JSON{ { U"sent", GameTimer() }, { U"timer", Array<int32>(std::begin(me.timer), std::end(me.timer)) } });
		room.sendReport(U"PlayerInfoAP", me.ap);
		room.sendReport(U"PlayerInfoSpecialAttack", me.special_attack);
		room.update();
		if (room.error()) {
			showError(*room.error());
			return;
		}
		//相手が脱落した (v3 のサーバーは残った人がいる限り部屋を消さない)
		if (room.users().size() < player_sum) {
			error_mode = 1;
			error_ID = 1;
			return;
		}
		//一方的な報告の処理
		for (const auto& event : room.receiveReports()) {
			if (event.type == U"PlayerInfoPos") {
				Json2ArrayPos(event.data, player[another_player_number].pos);
			}
			if (event.type == U"PlayerStatus") {
				const int status = event.data.get<int32>();
				//状態は毎フレーム届くため、前回届いた状態から立ち上がったビットでだけ効果音を鳴らす
				const int started = (status & ~received_status);
				received_status = status;
				player[another_player_number].status = status;
				//各種効果音の設定
				if (started & 3) {
					player[another_player_number].se[0] = true;
				}
				if (started & 4) {
					player[another_player_number].se[1] = true;
				}
				if (started & 8) {
					player[another_player_number].se[5] = true;
				}
				if (started & 16) {
					player[another_player_number].se[2] = true;
				}
				if (started & 32) {
					player[another_player_number].se[3] = true;
				}
				if (started & 64) {
					player[another_player_number].se[4] = true;
				}
				if (started & 128) {
					player[another_player_number].se[6] = true;
				}
				if (started & 256) {
					player[another_player_number].se[7] = true;
				}
			}
			if (event.type == U"PlayerInfoTimer") {
				Json2ArrayTimer(event.data, player[another_player_number].timer);
			}
			if (event.type == U"PlayerInfoAP") {
				player[another_player_number].ap = (event.data).get<int32>();
			}
			if (event.type == U"PlayerInfoSpecialAttack") {
				player[another_player_number].special_attack = (event.data).get<bool>();
			}
		}
		//相互確認が必要な処理
		//全員が同じ順番で適用するので、HP とガードはここでだけ確定させる
		const uint64 end_tick = getData().start_tick + static_cast<uint64>(match_seconds / room.joined().tickDuration.count());
		for (const auto& event : room.receiveActions()) {
			//時間切れより後の確認イベントは適用しない
			if (end_tick <= event.tick) break;
			const int target = event.data.get<int32>();
			const int attacker = (event.from == room.joined().userId) ? player_number : another_player_number;
			int damage = 0;
			if (event.type == U"WeakAttack") {
				damage = get_character_power(player[attacker].number, 0);
			}elif(event.type == U"StrongAttack") {
				damage = get_character_power(player[attacker].number, 1);
				//玲限定技
			}elif(event.type == U"StrongAttackBomb") {
				damage = rei_strong_attack_bomb;
			}elif(event.type == U"SpecialAttack") {
				damage = get_character_power(player[attacker].number, 2);
			}elif(event.type == U"UniqueAttack") {
				damage = get_character_power(player[attacker].number, 3);
			}elif(event.type == U"Guard") {
				void_attack[target] = true;
				continue;
			}elif(event.type == U"VoidGuard") {
				void_attack[target] = false;
				continue;
			}elif(event.type == U"DestroyGuard") {
				void_attack[target] = false;
				damage = 1;
			}
			else {
				continue;
			}
			//ガード中
			if (void_attack[target]) {
				//暫定HPを元に戻す
				player[target].hp[1] += damage;
				//ガードしていない
			}
			else {
				//実質HPを確定
				player[target].hp[0] -= damage;
			}
			if (player[target].hp[0] <= 0) {
				finish_game(target != player_number);
				//笹食ってる場合じゃねぇ！！
				break;
			}
		}
		if (const auto last_tick = room.lastTick()) {
			remaining_seconds = Max(0, match_seconds - static_cast<int>((*last_tick - getData().start_tick) * room.joined().tickDuration.count()));
			//時間切れは残り HP が多い方の勝ち。同じなら両者とも負け
			if (!is_game_finished && (end_tick <= *last_tick)) {
				finish_game(player[another_player_number].hp[0] < player[player_number].hp[0]);
			}
		}

		//1秒ごとに、その間の往復時間の中央値を表示する
		rtt_samples.append(room.receiveRTTs());
		if (GameTimer() - ping_timer > 1000) {
			ping_timer = GameTimer();
			//応答が1つも無ければ、送ってからの経過時間で重さを示す
			Duration rtt = room.timeSinceLastSync();
			if (!rtt_samples.isEmpty()) {
				rtt_samples.sort();
				rtt = rtt_samples[rtt_samples.size() / 2];
			}
			ping = static_cast<int>(rtt.count() * 1000);
			rtt_samples.clear();
		}
	}
	catch (const Error& error) {
		Print << error.what();
		OutputLogFile("(INTERNAL ERROR)\n" + error.what().narrow());
	}

	for (int i = 0; i < player_sum; i++) {
		player[i].event = 0;
		//同期後のHPを確定する
		player[i].hp[1] = player[i].hp[0];
	}
}

void Game::Json2ArrayTimer(const JSON& json, int(&timer)[16]) {
	const JSON values = json[U"timer"];
	for (int i = 0; i < 16; i++)timer[i] = values[i].get<int32>();
	//開始を tick で揃えているので、ゲーム内時刻は相手と比べられる。先の時刻にはならないよう手元の時刻で打ち切る
	const int sent = Min(json[U"sent"].get<int32>(), GameTimer());
	timer[0] = sent - timer[1];
	timer[2] = sent - timer[7];
	timer[3] = sent - timer[8];
	timer[4] = sent - timer[9];
	timer[5] = sent - timer[10];
	timer[6] = sent - timer[11];
	timer[12] = sent - timer[13];
	timer[14] = sent - timer[15];
}

void Game::Json2ArrayPos(const JSON& json, Vec2(&pos)[2]) {
	pos[0] = { json[0].get<double>(), json[1].get<double>() };
	pos[1] = { json[2].get<double>(), json[3].get<double>() };
}

void Game::update_player_animation() {
	int now_time = GameTimer();
	for (int i = 0; i < player_sum; i++) {
		//if (!player_flag[i]) continue;
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
			if (now_time - player[i].timer[4] < 60) {
				player[i].img_number = 0;
			}elif(now_time - player[i].timer[4] < 180) {
				player[i].img_number = 4;
			}elif(now_time - player[i].timer[4] < 270) {
				player[i].img_number = 0;
			}
			else {
				player[i].status ^= 16;
				player[i].img_number = 0;
			}
			continue;
			//狂攻撃のアニメーション
		}elif(player[i].status & 32) {
			if (now_time - player[i].timer[5] < 130) {
				player[i].img_number = 0;
			}elif(now_time - player[i].timer[5] < 320) {
				player[i].img_number = 3;
			}elif(now_time - player[i].timer[5] < 450) {
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
			}elif(now_time - player[i].timer[6] < 330) {
				player[i].img_number = 5;
			}elif(now_time - player[i].timer[6] < 460) {
				player[i].img_number = 3;
			}
			else {
				player[i].status ^= 64;
				player[i].img_number = 0;
			}
			continue;
			//ガード破壊のアニメーション
		}elif(player[i].status & 128) {
			if (now_time - player[i].timer[12] < 60) {
				player[i].img_number = 0;
			}elif(now_time - player[i].timer[12] < 190) {
				player[i].img_number = 6;
			}elif(now_time - player[i].timer[12] < 250) {
				player[i].img_number = 0;
			}
			else {
				player[i].status ^= 128;
				player[i].img_number = 0;
			}
			continue;
			//特殊攻撃のアニメーション
		}elif(player[i].status & 256) {
			//玲
			if (player[i].number == 0) {
				if (now_time - player[i].timer[14] < 150) {
					player[i].img_number = 0;
				}elif(now_time - player[i].timer[14] < 400) {
					player[i].img_number = 2;
				}elif(now_time - player[i].timer[14] < 550) {
					player[i].img_number = 0;
				}
				else {
					player[i].status ^= 256;
					player[i].img_number = 0;
				}
				//アイリ
			}elif(player[i].number == 2) {
				if (now_time - player[i].timer[14] < 130) {
					player[i].img_number = 0;
				}elif(now_time - player[i].timer[14] < 1530) {
					player[i].img_number = 2;
				}elif(now_time - player[i].timer[14] < 1560) {
					player[i].img_number = 0;
				}
				else {
					player[i].status ^= 256;
					player[i].img_number = 0;
				}
				player[i].wave_pos = 3.0 * sin(0.1 * GameTimer());
			}
			continue;
			//ジャンプアニメーション
		}elif(player[i].status & 4) {
			player[i].img_number = 0;
			continue;
			//移動アニメーション
		}elif(player[i].status & 3) {
			player[i].img_number = 1;
			// アイリのみ移動アニメーション
			if (player[i].number == 2) {
				player[i].img_number = player[i].walking;
			}
			continue;
		}
		player[i].img_number = 0;
	}
}

void Game::draw() const {
#ifndef debug_mode
	if (!is_connected) {
		//通信中...
		Rect(0, 0, 1920, 1080).draw(ColorF{ Palette::Black });
		connecting_img.drawAt(1500, 950);
	}
	else {
#endif
		background_img.draw(0, 0);
		command_img.at(getData().player[player_number]).draw(120, 150);
		draw_HP_bar();
		draw_AP_bar();
		draw_ping();
		//残り時間
		font(U"{:02}:{:02}"_fmt(remaining_seconds / 60, remaining_seconds % 60)).drawAt(960, 50, Palette::White);

		draw_bullet();
		draw_knife();
		draw_torpedo();
		draw_player();
		draw_after_images();
		draw_effects();

		// WordDetector のパラメータ調整用
		//double coolTime = wordDetector.coolTime;
		//SimpleGUI::Slider(U"coolTime={}us"_fmt(coolTime), coolTime, 0.0, 1'000'000.0, Vec2(600, 20), 300, 500);
		//wordDetector.coolTime = coolTime;
		//double wordTimeout = wordDetector.wordTimeout;
		//SimpleGUI::Slider(U"wordTimeout={}us"_fmt(wordTimeout), wordTimeout, 0.0, 1'000'000.0, Vec2(600, 60), 300, 500);
		//wordDetector.wordTimeout = wordTimeout;
		//double wordTimeLimit = wordDetector.wordTimeLimit;
		//SimpleGUI::Slider(U"wordTimeLimit={}us"_fmt(wordTimeLimit), wordTimeLimit, 0.0, 10'000'000.0, Vec2(600, 100), 300, 500);
		//wordDetector.wordTimeLimit = wordTimeLimit;
		//double scoresHistoryLife = wordDetector.scoresHistoryLife;
		//SimpleGUI::Slider(U"scoresHistoryLife={}us"_fmt(scoresHistoryLife), scoresHistoryLife, 0.0, 1'000'000.0, Vec2(600, 140), 300, 500);
		//wordDetector.scoresHistoryLife = scoresHistoryLife;
		//SimpleGUI::Slider(U"scoreThreshold={}"_fmt(wordDetector.scoreThreshold), wordDetector.scoreThreshold, -1.0, 1.0, Vec2(600, 180), 300, 500);

		draw_settle();

		if (finish_fade_mode)Rect(0, 0, 1920, 1080).draw(ColorF{ 0, finish_fade });

#ifndef debug_mode
		//通信中...
		if (fade_back_alpha > 0) {
			Rect(0, 0, 1920, 1080).draw(ColorF{ 0,fade_back_alpha });
			connecting_img.drawAt(1500, 950, ColorF{ 1, fade_back_alpha });
		}
	}
#endif

	//エラーダイアログ
	if (error_mode) {
		Rect(0, 0, 1920, 1080).draw(ColorF{ 0, back_alpha });
		if (error_ID == 1) {
			destroyed_img.drawAt(960, error_pos_y);
		}elif(error_ID == 2) {
			timeout_img.drawAt(960, error_pos_y);
		}
	}
}

void Game::draw_settle() const {
	settle_img.scaled(1.0 / ((settle_fade == 0.0) ? 0.0000001 : settle_fade)).drawAt(960, 540, ColorF{ 1,settle_fade });
	if (settle_mode >= 2) {
		if (are_you_winnner) {
			you_win_img.drawAt(960, 540 + 1080 * settle_fade, ColorF{ 1, 1.0 - settle_fade });
		}
		else {
			you_lose_img.drawAt(960, 540 + 1080 * settle_fade, ColorF{ 1, 1.0 - settle_fade });
		}
	}
}

void Game::draw_ping() const {
	if (ping <= 30) {
		ping_fast_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, Palette::Green);
	}elif(ping <= 100) {
		ping_middle_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, ColorF{ 1.0,0.729,0.0 });
	}
	else {
		ping_slow_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, ColorF{ 0.812,0.137,0.137 });
	}
}

//弾の描画
void Game::draw_bullet() const {
	for (int i = 0; i < max_bullet; i++) {
		if (!bullet[i].exist)continue;
		guns_img(0, 7 * min(bullet[i].type, 3), 14, 7).rotated(bullet[i].angle).drawAt(bullet[i].pos);
	}
}

//ナイフの描画
void Game::draw_knife() const {
	for (int i = 0; i < max_knife; i++) {
		if (!knife[i].exist)continue;
		knives_img(0, 28 * knife[i].img_number, 54, 28).rotated(knife[i].angle[0]).drawAt(knife[i].pos);
	}
}

//魚雷の描画
void Game::draw_torpedo() const {
	for (int i = 0; i < max_torpedo; i++) {
		if (!torpedo[i].exist)continue;
		torpedo_img.rotated(torpedo[i].angle).drawAt(torpedo[i].pos);
	}
}

//エフェクトの描画
void Game::draw_effects() const {
	for (int i = 0; i < max_occation; i++) {
		if (!occation[i].exist)continue;
		const ScopedRenderStates2D blend{ BlendState::Additive };
		occation_img(occation[i].type * 20, 0, 20, 20).scaled(occation[i].scale).drawAt(occation[i].pos, ColorF{ 1.0,occation[i].alpha });
	}
}

//銃弾の残像の描画
void Game::draw_after_images() const {
	for (int i = 0; i < max_after_images; i++) {
		if (!after_images[i].exist)continue;
		const ScopedRenderStates2D blend{ BlendState::Additive };
		guns_img(0, 21, 14, 7).rotated(after_images[i].angle).drawAt(after_images[i].pos, ColorF{ 1.0,after_images[i].alpha });
	}
}

//キャラの描画
void Game::draw_player() const {
	for (int i = 0; i < player_sum; i++) {
		if (!player_flag[i]) continue;
		player_img.at(getData().player[i]).at(player[i].img_number).mirrored(player[i].direction).drawAt(draw_player_pos(player[i].pos[0], i));
		//シールドの表示
		if (player[i].status & 8)guard_img.drawAt(player[i].pos[0]);
	}
}

Vec2 Game::draw_player_pos(Vec2 player_pos, int i) const {
	if ((player[i].number == 2) && ((player[i].status & 256) == 256)) {
		return player_pos + Vec2{ player[i].wave_pos,0.0 };
	}
	return player_pos;
}

//HPバーの描画
void Game::draw_HP_bar() const {
	//1PのHP
	HP_bar_flame_img.draw(120, 100);
	HP_bar_gray_img.draw(130, 108);
	HP_bar_red_img(0, 0, 680.0 * ((double)player[0].hp[2] / player_max_hp), 25).draw(130, 108);
	HP_bar_blue_img(0, 0, 680.0 * ((double)player[0].hp[1] / player_max_hp), 25).draw(130, 108);

	//2PのHP
	HP_bar_flame_img.draw(1090, 100);
	HP_bar_gray_img.draw(1100, 108);
	HP_bar_red_img(680.0 * (1.0 - ((double)player[1].hp[2] / player_max_hp)), 0, 680.0 * ((double)player[1].hp[2] / player_max_hp), 25).mirrored().draw(1100, 108);
	HP_bar_blue_img(680.0 * (1.0 - ((double)player[1].hp[1] / player_max_hp)), 0, 680.0 * ((double)player[1].hp[1] / player_max_hp), 25).mirrored().draw(1100, 108);
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
	}
	else {
		AP_bar_img(0, 0, 360.0 * ((double)player[0].ap / player_max_ap), 42).mirrored().draw(612.0 - 360.0 * ((double)player[0].ap / player_max_ap), 992);
	}


	//2PのAP
	AP_bar_empty_img.draw(1300, 880);
	if (player[1].special_attack) {
		AP_bar_max_img.draw(1300, 880);
		{
			const ScopedRenderStates2D blend{ BlendState::Additive };
			fire_img.at(player[1].fire_animation).draw(1610, 800);
		}
	}
	else {
		AP_bar_img(0, 0, 360.0 * ((double)player[1].ap / player_max_ap), 42).mirrored().draw(1305, 992);
	}
}

int Game::get_character_power(int character_number, int attack_sort) {
	if (character_number == 0) {
		switch (attack_sort)
		{
		case 0:
			return rei_weak_atttack;
		case 1:
			return rei_strong_attack;
		case 2:
			return rei_special_attack;
		case 3:
			return rei_uniqe_attack;
		default:
			return 0;
		}

	}elif(character_number == 1) {
		switch (attack_sort)
		{
		case 0:
			return yuuka_weak_atttack;
		case 1:
			return yuuka_strong_attack;
		case 2:
			return yuuka_special_attack;
		default:
			return 0;
		}
	}elif(character_number == 2) {
		switch (attack_sort)
		{
		case 0:
			return airi_weak_atttack;
		case 1:
			return airi_strong_attack;
		case 2:
			return airi_special_attack;
		case 3:
			return airi_uniqe_attack;
		default:
			return 0;
		}
	}
	else {
		switch (attack_sort)
		{
		case 0:
			return no0_weak_atttack;
		case 1:
			return no0_strong_attack;
		case 2:
			return no0_special_attack;
		default:
			return 0;
		}

	}
}
