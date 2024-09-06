#include "Game.hpp"
# include "common_function.hpp"

//#define player_number 0
//#define another_player_number 1

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
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/weak_attack.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/powerful_kick.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/destroy_gard.png") });
		player_img.at(1).push_back(Texture{ Unicode::Widen("../images/game/1/special_kick.png") });
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
	if (KeyB.pressed()) return 1;
	if (KeyV.pressed()) return 2;
	if (KeyF.pressed()) return 3;
	if (KeyG.pressed()) return 4;
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
		synchronizate_data();
		//フェードイン
		if (fade_back_alpha > 0.0) {
			fade_back_alpha = 1.0 - ((double)(GameTimer() - fade_back_timer))/ (700-(getData().client->isOwner()) ? ping_time : 0);
			if (fade_back_alpha < 0.0)fade_back_alpha = 0.0;
			return;
		}
		//プレイヤー情報を更新
		update_player();
		//APバーの描画情報を更新
		update_AP_bar_animation();
		//プレイヤーのアニメーションを更新
		update_player_animation();
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
			gard_se.playOneShot();
			player[player_number].se[5] = true;
			player[player_number].status |= 8;
			player[player_number].timer[3] = now_time;
			getData().client->sendAction(U"Guard", player_number);
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
						if ((player[i].event[0] & 1) == 0) {
							if (player[cnt].se[2]) {
								player[cnt].se[2] = false;
								dos_se.playOneShot();
							}
							getData().client->sendAction(U"WeakAttack", Vec2{ cnt,i });
							//player[i].event[0] |= 1;
							//TODO:あとで通信専用の時間に変更する
							//player[i].event[1] = now_time;
							player[i].hp[1] -= 3;
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
						if ((player[i].event[0] & 2) == 0) {
							//狂攻撃
							if (player[cnt].status & 32) {
								if (player[cnt].se[3]) {
									player[cnt].se[3] = false;
									dos_se.playOneShot();
								}
								if (cnt == player_number)
									getData().client->sendAction(U"StrongAttack", i);
								//player[i].event[0] |= 2;
								//TODO:あとで通信専用の時間に変更する
								//player[i].event[1] = now_time;
								player[i].hp[1] -= 5;
								player[cnt].ap += 3;
								//必殺技
							}
							else {
								if (player[cnt].se[4]) {
									player[cnt].se[4] = false;
									dododos_se.playOneShot();
								}
								if (cnt == player_number)
									getData().client->sendAction(U"SpecialAttack", i);
								//player[i].event[0] |= 4;
								//TODO:あとで通信専用の時間に変更する
								//player[i].event[1] = now_time;
								player[i].hp[1] -= 15;
							}
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
		}
		//相互確認が必要な処理
		while (const auto event = getData().client->receiveAction()) {
			if (event->type == U"WeakAttack") {
				//TODO:確認開始！
			}elif(event->type == U"StrongAttack") {

			}elif(event->type == U"SpecialAttack") {

			}
		}

		//0.5秒ごとにpingを取得
		if (GameTimer() - ping_timer > 500) {
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

	//TODO
	for (int i = 0; i < player_sum; i++) {
		if (i == player_number) continue;
		player[i].event[0] = 0;
		player[i].event[1] = 0;
		//TODO:同期後のHPを処理する
		player[i].hp[0] = player[i].hp[1];
	}
}

void Game::Json2ArrayTimer(String str, int(&timer)[12]) {
	str.replace(U"}", U"");
	str.replace(U"{", U"");
	str.replace(U")", U"");
	str.replace(U"(", U"");
	// 文字列から数値を抽出
	Array<int32> parts = str.split(U',').map(Parse<int32>);
	for (int i = 0; i < 12; i++)timer[i] = parts[i];
	timer[0] = GameTimer() - timer[1];
	timer[2] = GameTimer() - timer[7];
	timer[3] = GameTimer() - timer[8];
	timer[4] = GameTimer() - timer[9];
	timer[5] = GameTimer() - timer[10];
	timer[6] = GameTimer() - timer[11];
}

void Game::Json2ArrayPos(String str,Vec2 (& pos)[2]) {
	str.replace(U"}", U"");
	str.replace(U"{", U"");
	str.replace(U")", U"");
	str.replace(U"(", U"");
	// 文字列から数値を抽出
	Array<double> parts = str.split(U',').map(Parse<double>);

	if (parts.size() >= 4) {
		pos[0].x = parts[0];
		pos[0].y = parts[1];
		pos[1].x = parts[2];
		pos[1].y = parts[3];
	}
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
			if (now_time - player[i].timer[5] < 60) {
				player[i].img_number = 0;
			}elif(now_time - player[i].timer[5] < 190) {
				player[i].img_number = 4;
			}elif(now_time - player[i].timer[5] < 250) {
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
			int t = now_time - player[i].img_timer;
			if (t > 160) {
				player[i].img_timer = now_time;
				player[i].img_status = 1+player[i].img_status % 2;
			}
			player[i].img_number = player[i].img_status;
			continue;
		}
		player[i].img_number = 0;
	}
}

void Game::draw() const {
	if (!is_connected) {
		Rect(0, 0, 1920, 1080).draw(ColorF{ Palette::Black });
		connecting_img.drawAt(1500, 950);
	}else {
		background_img.draw(0, 0);
		draw_HP_bar();
		draw_AP_bar();
		draw_ping();

		draw_player();

		if (fade_back_alpha > 0) {
			Rect(0, 0, 1920, 1080).draw(ColorF{ 0,fade_back_alpha });
			connecting_img.drawAt(1500, 950, ColorF{ 1, fade_back_alpha });
		}
	}

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

void Game::draw_ping() const {
	if (ping <= 30) {
		ping_fast_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, Palette::Green);
	}elif(ping <= 100) {
		ping_middle_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, Palette::Orange);
	}else {
		ping_slow_img.draw(1550, 7);
		font(ping).drawAt(1720, 50, Palette::Red);
	}
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
		AP_bar_img(0, 0, 360.0 * ((double)player[1].ap / 100), 42).mirrored().draw(1305, 992);
	}
}
