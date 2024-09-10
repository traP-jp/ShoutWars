#include "Game.hpp"
# include "common_function.hpp"

//音声コマンド
//「殴れ」
#define 弱攻撃 U"AUE"
//「キック」
#define 狂攻撃 U"IU"
//「龍虎水雷撃」
#define 必殺技 U"UOUIAIEI"
//「ガード」
#define ガード1 U"AO"
//「守れ」
#define ガード2 U"AOE"
//「壊せ」
#define ガード破壊1 U"OAE"
//「刺せ」
#define ガード破壊2 U"AE" 

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
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/running.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/running.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/kick.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/weak_attack.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/powerful_kick.png") });
		player_img.at(0).push_back(Texture{ Unicode::Widen("../images/game/0/destroy_guard.png") });
	}
	//ユウカの画像
	if ((getData().player[0] == 1) || (getData().player[1] == 1)) {
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/waiting.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/running.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/special_kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/weak_attack.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/powerful_kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/destroy_guard.png") });
	}
	//アイリの画像
	if ((getData().player[0] == 2) || (getData().player[1] == 2)) {
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/waiting.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/running.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/special_gun.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/strong_knife.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/gun.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/beautiful_knife.png") });
		player_img.at(2).push_back(Texture{ Unicode::Widen("../images/game/2/destroy_guard.png") });
	}
	//No.0の画像
	if ((getData().player[0] == 3) || (getData().player[1] == 3)) {
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/waiting.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/running.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/running.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/kick.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/weak_attack.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/powerful_kick.png") });
		player_img.at(3).push_back(Texture{ Unicode::Widen("../images/game/3/destroy_guard.png") });
	}

	//録音開始!
#ifndef debug_mode
	getData().phoneme.start();
#endif

	player[0].pos[0] = {600.0,player_min_y};
	player[1].pos[0] = {1200,player_min_y};
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
#ifndef debug_mode
	wordDetector.addVowel(getData().vowels[getData().phoneme.estimate()]);
	if (wordDetector.detect(弱攻撃))return 1;
	if (wordDetector.detect(狂攻撃))return 2;
	if (wordDetector.detect(必殺技))return 3;
	if (wordDetector.detect(ガード1))return 4;
	if (wordDetector.detect(ガード2))return 4;
	if (wordDetector.detect(ガード破壊1))return 5;
	if (wordDetector.detect(ガード破壊2))return 5;
#else
	if (KeyB.pressed()) return 1;
	if (KeyV.pressed()) return 2;
	if (KeyF.pressed()) return 3;
	if (KeyG.pressed()) return 4;
	if (KeyC.pressed()) return 5;
#endif
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
		try {
			if (getData().client->isOwner()) {
				getData().client->update();
				while (const auto event = getData().client->receiveReport()) {
					if (event->type == U"Start") {
						is_connected = true;
						//Player
						player_number = 0;
						another_player_number = 1;
						//ゲーム開始時刻
						connection_timer = (int)Time::GetMillisec()+700;
						ping_time = (getData().client->api->fetchServerStatus().get().ping).count();
						connection_timer += ping_time;
						fade_back_timer = GameTimer();
						fade_back_alpha = 1.0;
						//BGMを流す
						bgm.play();
					}
				}
			}else {
				getData().client->sendReport(U"Start", true);
				getData().client->update();
				//Player
				player_number = 1;
				another_player_number = 0;
				//時差調整
				System::Sleep(0.1s);
				//ゲーム開始時刻
				connection_timer = (int)Time::GetMillisec() + 700 ;
				is_connected = true;
				fade_back_timer = GameTimer();
				fade_back_alpha = 1.0;
				//BGMを流す
				bgm.play();
			}
		}catch (const APIClient::HTTPError& error) {
			if (FromEnum(error.statusCode) == 404) {
				error_mode = 1;
				error_ID = 1;
			}
			else {
				Print << U"[SERVER ERROR:" << FromEnum(error.statusCode) << U"] " << error.what();
				OutputLogFile("(SERVER ERROR:CODE [" + to_string(FromEnum(error.statusCode)) + "])\n" + error.what().narrow());
			}
		}
		catch (const Error& error) {
			if (error.what() == U"Session not found.") {
				error_mode = 1;
				error_ID = 2;
			}else {
				Print << error.what();
				OutputLogFile("(INTERNAL ERROR)\n" + error.what().narrow());
			}
		}
	}else {
		//同期処理
		if (!is_game_finished)synchronizate_data();
		//フェードイン
		if (fade_back_alpha > 0.0) {
			fade_back_alpha = 1.0 - ((double)(GameTimer() - fade_back_timer))/ (700-(getData().client->isOwner()) ? ping_time : 0);
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
		//決着！
		if (is_game_finished)update_settle();
#ifndef debug_mode
	}
#endif
}

void Game::update_settle() {
	//決着！
	if (settle_mode == 0) {
		if (GameTimer() - settle_timer <= 1000) {
			settle_fade = EaseOutExpo((double)(GameTimer() - settle_timer) / 1000.0);
		}else {
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
		finish_fade = Min(t / 2000.0,1.0);
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
			if (player[i].timer[1]  < 100) {
				player_reserved_pos[i].x = player[i].pos[1].x + ((player[i].status & 1) ? -1.0 : 1.0) * player[i].speed * player[i].timer[1] / 100;
			}
			else {
				if (i == player_number) {
					player[i].status ^= (player[i].status & 1) ? 1 : 2;
				}else {
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
		if ((player[player_number].status & 3) == 0){
			if (gotkey & 10) {
				player[player_number].status |= (gotkey & 2)?1:2;
				player[player_number].timer[0] = now_time;
				player[player_number].timer[1] = 0;
				player[player_number].pos[1].x = player[player_number].pos[0].x;
			}
		}
		//ジャンプ
		if ((gotkey & 1) && ((player[player_number].status & 52) == 0)) {
			jump_se.playOneShot();
			player[player_number].status |= 4;
			player[player_number].timer[2] = now_time;
			player[player_number].timer[7] = 0;
			player[player_number].pos[1].y = player[player_number].pos[0].y;
		}
	}
	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////
	for (int i = 0; i < player_sum; i++) {
		if (i == player_number) continue;
		if ((abs(player_reserved_pos[player_number].x - player_reserved_pos[i].x) < 70.0) && (abs(player_reserved_pos[player_number].y - player_reserved_pos[i].y) < 242.0)) {
			//相手が動いている場合
			if (player[i].status&3) {
				if (player[i].status == player[player_number].status) {
					player_reserved_pos[i].x += (player_reserved_pos[player_number].x < player_reserved_pos[i].x) ? 16.0 : -16.0;
				}elif(player[player_number].status == 0) {
					player_reserved_pos[player_number].x += (player_reserved_pos[player_number].x < player_reserved_pos[i].x) ? -16.0 : 16.0;
				}
				else {
					player_reserved_pos[player_number].x = player[player_number].pos[0].x;
					player_reserved_pos[i].x = player[i].pos[0].x;
				}
			//自分だけが動いている場合
			}else {
				player_reserved_pos[i].x += (player_reserved_pos[player_number].x < player_reserved_pos[i].x) ? 16.0 : -16.0;
			}
		}
	}

	//プレイヤーの向き
	if (player_reserved_pos[player_number].x < player[another_player_number].pos[0].x) {
		player[player_number].direction = false;
		player[another_player_number].direction = true;
	}else {
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
			if ((player[player_number].status & 116) == 0) {
				shot_se.playOneShot();
				player[player_number].se[2] = true;
				player[player_number].status |= 16;
				player[player_number].timer[4] = now_time;
			}
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
		//ガード
		}elif(got_voice == 4) {
			guard_se.playOneShot();
			player[player_number].se[5] = true;
			player[player_number].status |= 8;
			player[player_number].timer[3] = now_time;
#ifndef debug_mode
			getData().client->sendAction(U"Guard", player_numbefr);
#endif
		//ガード破壊
		}elif(got_voice == 5) {
			if ((player[player_number].status & 116) == 0) {
				shot_se.playOneShot();
				player[player_number].se[6] = true;
				player[player_number].status |= 128;
				player[player_number].timer[12] = now_time;
			}
		}

	}

	//技とかの処理/////////////////////////////////////////////////////////////////////////////////
	for (int cnt = 0; cnt < player_sum; cnt++) {
		//弱攻撃
		if (player[cnt].status & 16) {
			int t = now_time - player[cnt].timer[4];
			if ((100 < t) && (t < 250)) {
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
							}else {
#ifndef debug_mode
								if (cnt == player_number)
									getData().client->sendAction(U"WeakAttack", i);
#endif
								player[i].hp[1] -= 3;
							}
							player[cnt].ap += 1;
						}
					}
				}
			}
		}
		//狂攻撃+必殺技
		if (player[cnt].status & 96) {
			int tmp = now_time - player[cnt].timer[(player[cnt].status & 32) ? 5 : 6];
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
								}else {
#ifndef debug_mode
									if (cnt == player_number)
										getData().client->sendAction(U"StrongAttack", i);
#endif
									player[i].hp[1] -= 5;
								}
								player[cnt].ap += 3;
								//必殺技
							}else {
								if (player[cnt].se[4]) {
									player[cnt].se[4] = false;
									dododos_se.playOneShot();
								}
								player[i].event |= 4;
								if (player[i].status & 8) {
									void_damage_se.playOneShot();
								}else {
#ifndef debug_mode
									if (cnt == player_number)
										getData().client->sendAction(U"SpecialAttack", i);
#endif
									player[i].hp[1] -= 15;
								}
							}
						}
					}
				}
			}
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
								getData().client->sendAction(U"DestroyGuard", i);
#endif
							player[i].hp[1] -= 1;
							player[cnt].ap += 1;
						}
					}
				}
			}
		}

		//TODO:特殊攻撃

		//シールド
		if (player[cnt].status & 8) {
			int t = now_time - player[cnt].timer[3];
			//ガード時間は1秒
			if (t > 1000) {
				player[cnt].status ^= 8;
#ifndef debug_mode
				getData().client->sendAction(U"VoidGuard", cnt);
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
			//敗北を通知
#ifndef debug_mode
			getData().client->sendAction(U"Loser", i);
#endif
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
	try {
		getData().client->sendReport(U"PlayerInfoPos", player[player_number].pos);
		if (player[player_number].status != player[player_number].old_status) {
			getData().client->sendReport(U"PlayerStatus", player[player_number].status);
			player[player_number].old_status = player[player_number].status;
		}
		getData().client->sendReport(U"PlayerInfoTimer", player[player_number].timer);
		//自分のHPは基本的に相手が管理する
		getData().client->sendReport(U"PlayerInfoHP", player[another_player_number].hp);
		getData().client->sendReport(U"PlayerInfoAP", player[player_number].ap);
		getData().client->update();
		//一方的な報告の処理
		while (const auto event = getData().client->receiveReport()) {
			if (event->type == U"PlayerInfoPos") {
				Json2ArrayPos((event->data).getString(), player[another_player_number].pos);
			}
			if (event->type == U"PlayerStatus") {
				player[another_player_number].status = (event->data).get<int32>();
			}
			if (event->type == U"PlayerInfoTimer") {
				Json2ArrayTimer((event->data).getString(), player[another_player_number].timer);
			}
			//自分のHPは基本的に相手が管理する
			//したがって相手に自分のHPを聞かなくてはならない
			if (event->type == U"PlayerInfoHP") {
				Json2ArrayHP((event->data).getString(), player[player_number].hp);
			}
			if (event->type == U"PlayerInfoAP") {
				player[another_player_number].ap = (event->data).get<int32>();
			}
		}
		//相互確認が必要な処理
		bool void_attack[player_sum] = { false };
		while (const auto event = getData().client->receiveAction()) {
			if (event->type == U"WeakAttack") {
				//自分のHPは基本的に相手が管理する
				if (event->data.get<int32>() != player_number) {
					//ガード中
					if (void_attack[event->data.get<int32>()]) {
						//暫定HPを元に戻す
						player[event->data.get<int32>()].hp[1] += 3;
						//APを増やす
						if (event->data.get<int32>() == player_number)player[event->data.get<int32>()].ap += 3;
						//ガードしていない
					}
					else {
						//実質HPを確定
						player[event->data.get<int32>()].hp[0] -= 3;
					}
				}
			}elif(event->type == U"StrongAttack") {
				if (event->data.get<int32>() != player_number) {
					//ガード中
					if (void_attack[event->data.get<int32>()]) {
						//暫定HPを元に戻す
						player[event->data.get<int32>()].hp[1] += 5;
						//APを増やす
						if (event->data.get<int32>() == player_number)player[event->data.get<int32>()].ap += 5;
						//ガードしていない
					}
					else {
						//実質HPを確定
						player[event->data.get<int32>()].hp[0] -= 5;
					}
				}
			}elif(event->type == U"SpecialAttack") {
				if (event->data.get<int32>() != player_number) {
					//ガード中
					if (void_attack[event->data.get<int32>()]) {
						//暫定HPを元に戻す
						player[event->data.get<int32>()].hp[1] += 15;
						//APを増やす
						if (event->data.get<int32>() == player_number)player[event->data.get<int32>()].ap += 15;
						//ガードしていない
					}
					else {
						//実質HPを確定
						player[event->data.get<int32>()].hp[0] -= 15;
					}
				}
			}elif(event->type == U"Guard") {
				void_attack[event->data.get<int32>()] = true;
			}elif(event->type == U"VoidGuard") {
				void_attack[event->data.get<int32>()] = false;
			}elif(event->type == U"DestroyGuard") {
				void_attack[event->data.get<int32>()] = false;
				player[event->data.get<int32>()].hp[0] -= 1;
			}elif(event->type == U"Loser") {
				is_game_finished = true;
				are_you_winnner = (event->data.get<int32>() != player_number);
				settle_timer = GameTimer();
				//笹食ってる場合じゃねぇ！！
				break;
			}
		}

		//2秒ごとにpingを取得
		if (GameTimer() - ping_timer > 2000) {
			ping_timer = GameTimer();
			ping = (int)getData().client->api->fetchServerStatus().get().ping.count();
		}
	}catch (const APIClient::HTTPError& error) {
		Print << U"[SERVER ERROR:" << FromEnum(error.statusCode) << U"] " << error.what();
		OutputLogFile("(SERVER ERROR:CODE [" + to_string(FromEnum(error.statusCode)) + "])\n" + error.what().narrow());
	}
	catch (const Error& error) {
		if (error.what() == U"Session not found.") {
			error_mode = 1;
			error_ID = 2;
		}else {
			Print << error.what();
			OutputLogFile("(INTERNAL ERROR)\n" + error.what().narrow());
		}
	}

	for (int i = 0; i < player_sum; i++) {
		player[i].event = 0;
		//同期後のHPを確定する
		player[i].hp[1] = player[i].hp[0];
	}
}

#define remove_trash(str)\
	str.replace(U"}", U"");\
	str.replace(U"{", U"");\
	str.replace(U")", U"");\
	str.replace(U"(", U"");

void Game::Json2ArrayTimer(String str, int(&timer)[16]) {
	remove_trash(str);
	// 文字列から数値を抽出
	Array<int32> parts = str.split(U',').map(Parse<int32>);
	for (int i = 0; i < 16; i++)timer[i] = parts[i];
	timer[0] = GameTimer() - timer[1];
	timer[2] = GameTimer() - timer[7];
	timer[3] = GameTimer() - timer[8];
	timer[4] = GameTimer() - timer[9];
	timer[5] = GameTimer() - timer[10];
	timer[6] = GameTimer() - timer[11];
	timer[12] = GameTimer() - timer[13];
	timer[14] = GameTimer() - timer[15];
}

void Game::Json2ArrayPos(String str,Vec2 (& pos)[2]) {
	remove_trash(str);
	// 文字列から数値を抽出
	Array<double> parts = str.split(U',').map(Parse<double>);

	if (parts.size() >= 4) {
		pos[0].x = parts[0];
		pos[0].y = parts[1];
		pos[1].x = parts[2];
		pos[1].y = parts[3];
	}
}

void Game::Json2ArrayHP(String str, int(&hp)[3]) {
	remove_trash(str);
	// 文字列から数値を抽出
	Array<int32> parts = str.split(U',').map(Parse<int32>);
	for (int i = 0; i < 3; i++)hp[i] = parts[i];
}

#undef remove_trash

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
			}elif(now_time - player[i].timer[4] < 190) {
				player[i].img_number = 4;
			}elif(now_time - player[i].timer[4] < 250) {
				player[i].img_number = 0;
			}else {
				player[i].status ^= 16;
				player[i].img_number = 0;
			}
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
			}elif(now_time - player[i].timer[6] < 330) {
				player[i].img_number = 5;
			}elif(now_time - player[i].timer[6] < 450) {
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
		//ジャンプアニメーション
		}elif(player[i].status & 4) {
			player[i].img_number = 0;
			continue;
		//移動アニメーション
		}elif(player[i].status & 3) {
			player[i].img_number = 1;
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
	}else {
#endif
		background_img.draw(0, 0);
		command_img.draw(120, 150);
		draw_HP_bar();
		draw_AP_bar();
		draw_ping();

		draw_player();

		String inputVowels = U"";
		for (const auto& vowel : wordDetector.getVowelBuffer(10)) {
			if (vowel == U'A') inputVowels += U"ア";
			if (vowel == U'I') inputVowels += U"イ";
			if (vowel == U'U') inputVowels += U"ウ";
			if (vowel == U'E') inputVowels += U"エ";
			if (vowel == U'O') inputVowels += U"オ";
		}
		font(U"入力: {}"_fmt(inputVowels)).draw(60, 10, 10, Palette::Lightgray);
		//double vowelHistoryLife = wordDetector.vowelHistoryLife;
		//SimpleGUI::Slider(U"vowelHistoryLife={}us"_fmt(vowelHistoryLife), vowelHistoryLife, 0.0, 1000000.0, Vec2(600, 30), 300, 500);
		//wordDetector.vowelHistoryLife = vowelHistoryLife;

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
	settle_img.scaled(1.0/((settle_fade == 0.0)?0.0000001: settle_fade)).drawAt(960, 540, ColorF{1,settle_fade});
	if (settle_mode >= 2) {
		if (are_you_winnner) {
			you_win_img .drawAt(960, 540 + 1080 * settle_fade, ColorF{ 1, 1.0 - settle_fade});
		}else {
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
		font(ping).drawAt(1720, 50, ColorF{1.0,0.729,0.0});
	}else {
		ping_slow_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, ColorF{ 0.812,0.137,0.137 });
	}
}

//キャラの描画
void Game::draw_player() const {
	for (int i = 0; i < player_sum;i++) {
		if (!player_flag[i]) continue;
		player_img.at(getData().player[i]).at(player[i].img_number).mirrored(player[i].direction).drawAt(player[i].pos[0]);
		//シールドの表示
		if (player[i].status & 8)guard_img.drawAt(player[i].pos[0]);
	}
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
	HP_bar_red_img (680.0 * (1.0 - ((double)player[1].hp[2] / player_max_hp)), 0, 680.0 * ((double)player[1].hp[2] / player_max_hp), 25).mirrored().draw(1100, 108);
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
		AP_bar_img(0, 0, 360.0 * ((double)player[1].ap / player_max_ap), 42).mirrored().draw(1305, 992);
	}
}
