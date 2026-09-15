#include "Game.hpp"
# include "common_function.hpp"

# include <algorithm>
# include <ranges>

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
	cpu_room = dynamic_cast<Multiplay::LocalRoom*>(getData().room.get());
	return_glow.init(return_img);
	//CPU 戦は通信を待たないので、シーンのフェードインと同時に始める
	if (cpu_room) start_match(0);
	//対戦ごとに読むので、設定ファイルを書き換えれば次の対戦から変わる
	if (cpu_room) cpu_brain.emplace(LoadCpuBaseSkill(U"config.json"));
}

CpuView Game::make_cpu_view(int now_time) const {
	const auto fighter = [&](int i) {
		const Player& p = player[i];
		return CpuFighter{ .pos = p.pos[0], .status = p.status, .number = p.number, .hp = p.hp[0], .special_ready = p.special_attack, .guard_cooling_down = is_guard_cooling_down(i, now_time) };
	};
	return CpuView{
		.now_ms = now_time,
		.self = fighter(another_player_number),
		.opponent = fighter(player_number),
		.max_hp = player_max_hp,
		.walk_speed = player[another_player_number].speed * 10.0,
		.stage_min_x = stage_min_x,
		.stage_max_x = stage_max_x,
		.opponent_unmatched_utterances = commandRecognizer.unmatchedUtterances(),
	};
}

int Game::getkey() {
	const Directions directions = PressedDirections();
	//ノートパソコンでは上キーが小さいことがあるので、スペースキーでもジャンプできる。コントローラーでは右側のボタンでもジャンプできる
	const bool jump = directions.up || KeySpace.pressed() || ControllerFaceButtonPressed();
	return (jump ? 1 : 0) | (directions.left ? 2 : 0) | (directions.down ? 4 : 0) | (directions.right ? 8 : 0);
}

int Game::voice_command() {
	const auto phoneme_scores = getData().phoneme.estimate();
	voiceMonitor.update(getData().phoneme, phoneme_scores);
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
	return commandRecognizer.update(phoneme_scores, getData().player[player_number]);
}

void Game::handle_started_moves() {
	//状態のビットと行動の番号の対応 (16:弱攻撃, 32:強攻撃, 64:必殺技, 8:ガード, 128:ガード破壊, 256:特殊攻撃)
	constexpr std::array<std::pair<int, int32>, 6> moves = { { { 16, 1 }, { 32, 2 }, { 64, 3 }, { 8, 4 }, { 128, 5 }, { 256, 6 } } };
	for (int i = 0; i < player_sum; i++) {
#ifndef debug_mode
		const int started = is_local_player(i) ? (player[i].status & ~previous_status[i]) : received_started_status;
#else
		const int started = player[i].status & ~previous_status[i];
#endif
		previous_status[i] = player[i].status;
		//新しく出した技は、まだ当たっていない
		player[i].hit_done &= ~started;
		for (const auto& [bit, action] : moves) {
			if (started & bit) commandFeedback.started(i, action, getData().player[i], i == player_number);
		}
	}
	received_started_status = 0;
}

bool Game::is_guard_cooling_down(int cnt, int now_time) const {
	return now_time - player[cnt].guard_broken_time < guard_cooldown_ms;
}

bool Game::is_ducking(int cnt) const {
	return (player[cnt].status & 3) && ((player[cnt].number == 0) || (player[cnt].number == 1));
}

bool Game::is_landing_recovery(int cnt, int now_time) const {
	return now_time - player[cnt].landing_time < landing_recovery_ms;
}

bool Game::can_start_attack(int cnt, int now_time) const {
	return ((player[cnt].status & attack_blocking_status) == 0) && !is_landing_recovery(cnt, now_time);
}

bool Game::is_local_player(int cnt) const {
	return (cnt == player_number) || (cpu_room && (cnt == another_player_number));
}

void Game::send_action(int sender, StringView type, int target) {
	if (sender == player_number) {
		getData().room->sendAction(type, target);
	}elif(cpu_room && (sender == another_player_number)) {
		cpu_room->sendCpuAction(type, target);
	}
}

Optional<double> Game::pull_direction(int cnt, double x, int now_time) const {
	const Player& other = player[1 - cnt];
	const int t = now_time - other.timer[6];
	if ((other.number != 1) || !(other.status & 64) || (200 + yuuka_special_windup_ms <= t)) return none;
	const double distance = other.pos[0].x - x;
	if ((abs(distance) <= yuuka_special_pull_stop) || (yuuka_special_pull_range <= abs(distance))) return none;
	return (distance < 0.0) ? -1.0 : 1.0;
}

void Game::start_walk(int cnt, int direction, int now_time) {
	if (player[cnt].status & move_blocking_status) return;
	player[cnt].status |= direction;
	player[cnt].timer[0] = now_time;
	player[cnt].timer[1] = 0;
	player[cnt].pos[1].x = player[cnt].pos[0].x;
	if (player[cnt].number == 2)
		player[cnt].walking = !player[cnt].walking;
}

bool Game::start_jump(int cnt, int now_time) {
	if ((player[cnt].status & jump_blocking_status) || is_landing_recovery(cnt, now_time)) return false;
	jump_se.playOneShot();
	player[cnt].status |= 4;
	player[cnt].timer[2] = now_time;
	player[cnt].timer[7] = 0;
	player[cnt].pos[1].y = player[cnt].pos[0].y;
	return true;
}

bool Game::start_move(int cnt, int action, int now_time) {
	Player& p = player[cnt];
	if (action == 4) {
		if ((p.status & guard_blocking_status) || is_guard_cooling_down(cnt, now_time)) return false;
		guard_se.playOneShot();
		p.se[5] = true;
		p.status |= 8;
		p.timer[3] = now_time;
#ifndef debug_mode
		send_action(cnt, U"Guard", cnt);
#endif
		return true;
	}
	//技ごとの状態のビット・開始時刻を入れるタイマー・効果音のフラグ
	struct Move { int bit; int timer; int se; };
	static constexpr std::array<Move, 7> moves = { { {}, { 16, 4, 2 }, { 32, 5, 3 }, { 64, 6, 4 }, {}, { 128, 12, 6 }, { 256, 14, 7 } } };
	if ((action < 1) || (6 < action)) throw Error{ U"Unknown move: {}"_fmt(action) };
	//特殊攻撃があるのは玲とアイリだけ (他のキャラは状態を下ろすアニメーションが無く、動けなくなる)
	const bool has_move = (action != 6) || (p.number == 0) || (p.number == 2);
	if (!has_move || !can_start_attack(cnt, now_time) || ((action == 3) && !p.special_attack)) return false;
	const Move& move = moves[action];
	if (action == 3) {
		bom_se.playOneShot();
		p.ap = 0;
		p.special_attack = false;
	}elif(action != 6) {
		shot_se.playOneShot();
	}
	p.se[move.se] = true;
	p.status |= move.bit;
	p.timer[move.timer] = now_time;
	return true;
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

void Game::start_match(int fade_ms) {
	is_connected = true;
	player_number = getData().room->isOwner() ? 0 : 1;
	another_player_number = 1 - player_number;
	//ゲーム開始時刻
	connection_timer = (int)Time::GetMillisec() + fade_ms;
	fade_back_timer = GameTimer();
	fade_back_alpha = (0 < fade_ms) ? 1.0 : 0.0;
	bgm.play();
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
			start_match(700);
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
	//CPU 戦は待たせる相手がいないので、途中でやめてタイトルに戻れる
	if (cpu_room && !is_game_finished) {
		is_return_hovered = return_shape.mouseOver();
		if (is_return_hovered) Cursor::RequestStyle(CursorStyle::Hand);
		if (return_shape.leftClicked()) {
			cancel_sound.playOneShot();
			bgm.stop(0.8s);
			getData().before_scene = State::Game;
			changeScene(State::Title, 0.8s);
			return;
		}
	}
	//プレイヤー情報を更新
	if (!is_game_finished)update_player();
	handle_started_moves();
	guard_cooldown_ratio = Clamp(1.0 - static_cast<double>(GameTimer() - player[player_number].guard_broken_time) / guard_cooldown_ms, 0.0, 1.0);
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
				if (is_local_player(i)) {
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
				player[i].landing_time = now_time;
				player_reserved_pos[i].y = player_min_y;
			}
		}
	}
	//プレイヤー(ユーザー操作)のキー入力処理////////////////////////////////////////////////////////////
	const int gotkey = getkey();
	//ジャンプは押した瞬間だけ受け付ける
	const bool jump_pressed = (gotkey & 1) && !previous_jump_input;
	previous_jump_input = (gotkey & 1);
	if (gotkey & 10) start_walk(player_number, (gotkey & 2) ? 1 : 2, now_time);
	if (jump_pressed) start_jump(player_number, now_time);
	//CPU の操作は、押し合いと向きを決めるより前に行う。歩きは 100 ミリ秒ごとに止まって入力で続くので、後にすると止まった瞬間に相手の方を向いてしまう
	if (cpu_brain) {
		const int cpu = another_player_number;
		const CpuIntent intent = cpu_brain->update(make_cpu_view(now_time));
		if (intent.walk) start_walk(cpu, intent.walk, now_time);
		if (intent.jump) start_jump(cpu, now_time);
		if (intent.move) start_move(cpu, intent.move, now_time);
		if (intent.unmatched) commandFeedback.unmatched(cpu, false);
	}
	//プレイヤー同士の相互作用/////////////////////////////////////////////////////////////////////
	//位置を決めるのは本人なので、引き寄せや押し合いでは手元で動かすプレイヤー (自分と CPU) だけを動かす。通信相手は、相手のクライアントが自分で動く
	for (int i = 0; i < player_sum; i++) {
		if (!is_local_player(i)) continue;
		const int other_number = 1 - i;
		Vec2& self = player_reserved_pos[i];
		const double self_x = self.x;
		if (player[i].knockback != 0.0) {
			const double max_step = knockback_speed * Scene::DeltaTime();
			const double step = Clamp(player[i].knockback, -max_step, max_step);
			self.x += step;
			player[i].knockback -= step;
		}
		//相手のユウカの必殺技の溜めの間は、相手に引き寄せられる
		if (const auto toward = pull_direction(i, self.x, now_time)) {
			self.x += *toward * Min(abs(player[other_number].pos[0].x - self.x) - yuuka_special_pull_stop, yuuka_special_pull_speed * Scene::DeltaTime());
		}
		//押し合い
		{
			const double contact = 70.0;
			//押す側がこの深さまでめり込むのを許す。接した位置で止めると、相手の画面で重ならず押せなくなる
			const double push_depth = 20.0;
			const Vec2& other = player_reserved_pos[other_number];
			//ジャンプ中はすれ違えるようにする
			const bool jumping = ((player[i].status | player[other_number].status) & 4);
			if ((!jumping) && (abs(self.x - other.x) < contact) && (abs(self.y - other.y) < 242.0)) {
				//重なり中の左右の見え方は通信の遅れで食い違うため、向きは動いている側の進行方向で決める
				const auto direction = [](const Player& p) { return (p.status & 1) ? -1.0 : ((p.status & 2) ? 1.0 : 0.0); };
				const double self_direction = direction(player[i]);
				const double other_direction = direction(player[other_number]);
				const bool pushing = (0.0 < (other.x - self.x) * self_direction);
				const bool pushed = (0.0 < (self.x - other.x) * other_direction);
				if (pushing && pushed) {
					//向かい合ってぶつかったら進めない
					self.x = player[i].pos[0].x;
				}elif(pushing) {
					self.x = other.x - self_direction * Max((other.x - self.x) * self_direction, contact - push_depth);
				}elif(pushed) {
					self.x = other.x + other_direction * contact;
				}elif((self_direction == 0.0) && (other_direction == 0.0) && (i == 1)) {
					//止まったまま重なったら片方だけが離れる。両方が動くと、見え方の食い違いで同じ向きに逃げ続ける
					self.x = other.x + ((self.x < other.x) ? -contact : contact);
				}
			}
		}
		//相手は歩き始めの位置と進み具合から位置を計算するので、動かした分を歩き始めの位置にも反映する
		player[i].pos[1].x += (self.x - self_x);
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
	const int got_voice = voice_command();
	if (shown_unmatched_utterances[0] < static_cast<int64>(commandRecognizer.unmatchedUtterances())) {
		shown_unmatched_utterances[0] = static_cast<int64>(commandRecognizer.unmatchedUtterances());
		commandFeedback.unmatched(player_number, true);
	}
	if (got_voice) {
		if ((got_voice == 3) && !player[player_number].special_attack) {
			commandFeedback.blocked(3, true);
		}elif(!start_move(player_number, got_voice, now_time)) {
			commandFeedback.blocked(got_voice);
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
					if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y)) {
						if ((player[cnt].hit_done & 128) == 0) {
							if (player[cnt].se[6]) {
								player[cnt].se[6] = false;
								dos_se.playOneShot();
							}
							player[cnt].hit_done |= 128;
							if (player[i].status & 8) {
								break_guard_se.playOneShot();
								player[i].status ^= 8;
								player[i].guard_broken_time = now_time;
							}
#ifndef debug_mode
							send_action(cnt, U"DestroyGuard", i);
#endif
							player[i].hp[1] -= destroy_guard_damage;
							player[cnt].ap += destroy_guard_ap;
						}
					}
				}
			}
		}
		//シールド
		if (player[cnt].status & 8) {
			int t = now_time - player[cnt].timer[3];
			if (t > guard_ms) {
				player[cnt].status ^= 8;
#ifndef debug_mode
				//相手の時計で先に解除されないよう、ガードした本人だけが送る
				send_action(cnt, U"VoidGuard", cnt);
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
		player_reserved_pos[i].x = Clamp(player_reserved_pos[i].x, static_cast<double>(stage_min_x), static_cast<double>(stage_max_x));
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
				bullet[bullet_number].swept_from_x = player_reserved_pos[cnt].x;
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
						send_action(cnt, U"StrongAttackBomb", another_player_number);
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
				if (bullet_passes(bullet[i], player_reserved_pos[j].x) && !is_ducking(j) && overlaps_hurtbox(player_reserved_pos[j].y, bullet[i].pos.y - projectile_radius, bullet[i].pos.y + projectile_radius)) {
					if ((player[j].event & 1) == 0) {
						player[j].event |= 1;
						if (player[j].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"WeakAttack", j);
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
						send_action(cnt, U"StrongAttack", j);
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
						send_action(cnt, U"SpecialAttack", j);
#endif
						player[j].hp[1] -= rei_special_attack;
						player[j].ap += rei_special_attack_ap;
					}
					bullet[i].exist = false;
					break;
				}
			}
		}
		bullet[i].swept_from_x = bullet[i].pos.x;
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
					send_action(cnt, U"UniqueAttack", j);
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
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y)) {
					if ((player[cnt].hit_done & 16) == 0) {
						if (player[cnt].se[2]) {
							player[cnt].se[2] = false;
							dos_se.playOneShot();
						}
						player[cnt].hit_done |= 16;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"WeakAttack", i);
#endif
							player[i].hp[1] -= yuuka_weak_atttack;
							player[cnt].ap += yuuka_weak_atttack_ap;
						}
					}
				}
			}
		}
	}
	//狂攻撃+必殺技
	if (player[cnt].status & 96) {
		player[cnt].timer[(player[cnt].status & 32) ? 10 : 11] = now_time - player[cnt].timer[(player[cnt].status & 32) ? 5 : 6];
		int tmp = player[cnt].timer[(player[cnt].status & 32) ? 10 : 11];
		const bool special = (player[cnt].status & 64);
		const int windup = special ? yuuka_special_windup_ms : 0;
		if ((200 + windup < tmp) && (tmp < 400 + windup)) {
			for (int i = 0; i < player_sum; i++) {
				if (i == cnt) continue;
				int tmp_pos_x = sign(player[cnt].direction) * (player_reserved_pos[cnt].x - player_reserved_pos[i].x);
				const bool reaches = special
					? ((yuuka_special_back_range < tmp_pos_x) && (tmp_pos_x < yuuka_special_front_range) && overlaps_hurtbox(player_reserved_pos[i].y, player_reserved_pos[cnt].y + yuuka_special_top, player_reserved_pos[cnt].y + yuuka_special_bottom))
					: ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y));
				if (reaches) {
					if ((player[cnt].hit_done & (player[cnt].status & 96)) == 0) {
						//狂攻撃
						if (player[cnt].status & 32) {
							if (player[cnt].se[3]) {
								player[cnt].se[3] = false;
								dos_se.playOneShot();
							}
							player[cnt].hit_done |= 32;
							if (player[i].status & 8) {
								void_damage_se.playOneShot();
							}
							else {
#ifndef debug_mode
								send_action(cnt, U"StrongAttack", i);
#endif
								player[i].hp[1] -= yuuka_strong_attack;
								player[cnt].ap += yuuka_strong_attack_ap;
							}
							//必殺技
						}
						else {
							if (player[cnt].se[4]) {
								player[cnt].se[4] = false;
								dododos_se.playOneShot();
							}
							player[cnt].hit_done |= 64;
							if (player[i].status & 8) {
								void_damage_se.playOneShot();
							}
							else {
#ifndef debug_mode
								send_action(cnt, U"SpecialAttack", i);
#endif
								player[i].hp[1] -= yuuka_special_attack;
								player[i].ap += yuuka_special_attack_ap;
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
			if (bullet_passes(bullet[i], player_reserved_pos[j].x) && !is_ducking(j) && overlaps_hurtbox(player_reserved_pos[j].y, bullet[i].pos.y - projectile_radius, bullet[i].pos.y + projectile_radius)) {
				if ((player[j].event & 1) == 0) {
					player[j].event |= 1;
					if (player[j].status & 8) {
						void_damage_se.playOneShot();
					}
					else {
						//弱
						if (bullet[i].mode == 0) {
#ifndef debug_mode
							send_action(cnt, U"WeakAttack", j);
#endif
							player[j].hp[1] -= airi_weak_atttack;
							player[cnt].ap += airi_weak_atttack_ap;
							//特殊
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"UniqueAttack", j);
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
		bullet[i].swept_from_x = bullet[i].pos.x;
	}
	//ナイフの移動・当たり判定処理
	for (int i = 0; i < max_knife; i++) {
		if (!knife[i].exist)continue;
		//ナイフを投げた側の相手を狙う
		const int target = (cnt == player_number) ? another_player_number : player_number;
		//待機
		if (knife[i].mode == 0) {
			if (now_time - knife[i].timer[0] > airi_knife_hover_ms) {
				knife[i].mode = 1;
				knife[i].timer[1] = now_time;
				//飛び始めるときに、相手の今の位置を狙い直す。浮いている間に動いても逃げられないが、飛んでくる間に動けばよけられる
				//(遠くにいるほど届くまでに間があるので、位置によっては操作でよけられる。必殺技は基本はガードで防ぎ、うまく操作すればよけられることもある、という立ち位置)
				knife[i].goal_pos = player_reserved_pos[target] + Vec2{ 0, knife_aim_y };
				knife[i].distance = knife[i].goal_pos.distanceFrom(knife[i].pos);
				knife[i].time = knife[i].distance / 1.8;
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
			int distance_x = abs(player_reserved_pos[target].x - knife[i].pos.x);
			int distance_y = abs(player_reserved_pos[target].y + knife_aim_y - knife[i].pos.y);
			const bool knife_in_hurtbox = overlaps_hurtbox(player_reserved_pos[target].y, knife[i].pos.y - projectile_radius, knife[i].pos.y + projectile_radius);
			if ((distance_x < 20.0) && knife_in_hurtbox && (!is_ducking(target) || (distance_y < 90.0))) {
				if (player[target].status & 8) {
					void_damage_se.playOneShot();
				}
				else {
#ifndef debug_mode
					send_action(cnt, U"SpecialAttack", target);
#endif
					player[target].hp[1] -= airi_special_attack;
					player[target].ap += airi_special_attack_ap;
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
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 130.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y)) {
					if ((player[cnt].hit_done & 32) == 0) {
						if (player[cnt].se[3]) {
							player[cnt].se[3] = false;
							dos_se.playOneShot();
						}
						player[cnt].hit_done |= 32;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"StrongAttack", i);
#endif
							player[i].hp[1] -= airi_strong_attack;
							player[cnt].ap += airi_strong_attack_ap;
						}
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
		if ((airi_unique_fire_start_ms < player[cnt].timer[15]) && (player[cnt].timer[15] < airi_unique_fire_end_ms)) {
			int tmp = now_time - player[cnt].airi_old_timer;
			if (tmp > 20) {
				player[cnt].airi_old_timer = now_time;
				//銃弾の発生
				search(bullet);
				if (bullet_number != -1) {
					bullet[bullet_number].pos = player_reserved_pos[cnt] + Vec2{ sign(!player[cnt].direction) * 60,-77 };
					bullet[bullet_number].old_pos = bullet[bullet_number].pos;
					bullet[bullet_number].swept_from_x = player_reserved_pos[cnt].x;
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
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 230.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y)) {
					if ((player[cnt].hit_done & 16) == 0) {
						if (player[cnt].se[2]) {
							player[cnt].se[2] = false;
							dos_se.playOneShot();
						}
						player[cnt].hit_done |= 16;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"WeakAttack", i);
#endif
							player[i].hp[1] -= no0_weak_atttack;
							player[cnt].ap += no0_weak_atttack_ap;
						}
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
				if ((5.0 < tmp_pos_x) && (tmp_pos_x < 130.0) && melee_reaches(player_reserved_pos[cnt].y, player_reserved_pos[i].y)) {
					if ((player[cnt].hit_done & 32) == 0) {
						if (player[cnt].se[3]) {
							player[cnt].se[3] = false;
							dos_se.playOneShot();
						}
						player[cnt].hit_done |= 32;
						if (player[i].status & 8) {
							void_damage_se.playOneShot();
						}
						else {
#ifndef debug_mode
							send_action(cnt, U"StrongAttack", i);
#endif
							player[i].hp[1] -= no0_strong_attack;
							player[cnt].ap += no0_strong_attack_ap;
						}
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
		//「？」は見た目だけなので確認は要らないが、取りこぼしても次で分かるよう、回数を毎フレーム送る
		room.sendReport(U"PlayerUnmatched", shown_unmatched_utterances[0]);
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
				received_started_status |= started;
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
			if (event.type == U"PlayerUnmatched") {
				const int64 count = event.data.get<int64>();
				if (shown_unmatched_utterances[1] < count) commandFeedback.unmatched(another_player_number, false);
				shown_unmatched_utterances[1] = count;
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
			int knockback = 0;
			if (event.type == U"WeakAttack") {
				damage = get_character_power(player[attacker].number, 0);
				if (player[attacker].number == 2) knockback = airi_weak_attack_knockback;
			}elif(event.type == U"StrongAttack") {
				damage = get_character_power(player[attacker].number, 1);
				//玲限定技
			}elif(event.type == U"StrongAttackBomb") {
				damage = rei_strong_attack_bomb;
			}elif(event.type == U"SpecialAttack") {
				damage = get_character_power(player[attacker].number, 2);
			}elif(event.type == U"UniqueAttack") {
				damage = get_character_power(player[attacker].number, 3);
				if (player[attacker].number == 2) knockback = airi_unique_attack_knockback;
			}elif(event.type == U"Guard") {
				void_attack[target] = true;
				continue;
			}elif(event.type == U"VoidGuard") {
				void_attack[target] = false;
				continue;
			}elif(event.type == U"DestroyGuard") {
				//壊された側の画面で当たりを検出できなかった場合も、確定したガード破壊に合わせて解除し、しばらくガードできなくする
				if (void_attack[target]) {
					if (!is_guard_cooling_down(target, GameTimer())) player[target].guard_broken_time = GameTimer();
					if (player[target].status & 8) player[target].status ^= 8;
				}
				void_attack[target] = false;
				damage = destroy_guard_damage;
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
				//位置を決めるのは本人なので、押し戻しも当たった側の画面で動かす
				if (is_local_player(target)) player[target].knockback += ((player[target].pos[0].x < player[attacker].pos[0].x) ? -1.0 : 1.0) * knockback;
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
		player[i].pull_seconds = -1.0;
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
			}elif(now_time - player[i].timer[5] < 450 + strong_attack_recovery_ms(player[i].number)) {
				player[i].img_number = 0;
			}
			else {
				player[i].status ^= 32;
				player[i].img_number = 0;
			}
			continue;
			//必殺技のアニメーション
		}elif(player[i].status & 64) {
			const int windup = (player[i].number == 1) ? yuuka_special_windup_ms : 0;
			const int t = now_time - player[i].timer[6];
			player[i].charge_glow = ((0 < windup) && (t < 120 + windup)) ? Min(1.0, t / 300.0) * (0.5 + 0.5 * sin(t * 0.025)) : 0.0;
			if ((player[i].number == 1) && (t < 200 + windup)) player[i].pull_seconds = t / 1000.0;
			if (t < 120 + windup) {
				player[i].img_number = 3;
			}elif(t < 330 + windup) {
				player[i].img_number = 5;
			}elif(t < 460 + windup) {
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
				if (now_time - player[i].timer[14] < airi_unique_raise_ms) {
					player[i].img_number = 0;
				}elif(now_time - player[i].timer[14] < airi_unique_lower_ms) {
					player[i].img_number = 2;
				}elif(now_time - player[i].timer[14] < airi_unique_end_ms) {
					player[i].img_number = 0;
				}
				else {
					player[i].status ^= 256;
					player[i].img_number = 0;
				}
				//撃っている間だけ、反動で震える
				const int t = now_time - player[i].timer[14];
				player[i].wave_pos = ((airi_unique_fire_start_ms <= t) && (t < airi_unique_fire_end_ms)) ? 3.0 * sin(0.1 * GameTimer()) : 0.0;
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
		commandFeedback.drawCommandList(command_img.at(getData().player[player_number]), Vec2{ 120, 145 }, guard_cooldown_ratio);
		voiceMonitor.draw();
		controlsGuide.draw(Vec2{ 1790, 150 });
		draw_HP_bar();
		draw_AP_bar();
		if (!cpu_room) draw_ping();
		//残り時間
		font(U"{:02}:{:02}"_fmt(remaining_seconds / 60, remaining_seconds % 60)).drawAt(960, 120, Palette::White);
		commandFeedback.drawVoiceHint(Vec2{ 960, 50 });

		draw_bullet();
		draw_knife();
		draw_torpedo();
		draw_special_pull();
		draw_player();
		draw_after_images();
		draw_effects();
		commandFeedback.drawMoveNames(Array<Vec2>{ player[0].pos[0], player[1].pos[0] });
		commandFeedback.drawUnmatchedMarks(Array<Vec2>{ player[0].pos[0], player[1].pos[0] });


		if (cpu_room && !is_game_finished) return_glow.draw(is_return_hovered, return_shape.pos);

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
//ユウカの必殺技の溜めの間、相手が引き寄せられていると分かるように、ユウカに向かって縮む輪と、相手の側から吸い込まれる風の筋を描く
void Game::draw_special_pull() const {
	const ScopedRenderStates2D additive{ BlendState::Additive };
	for (int i = 0; i < player_sum; i++) {
		const double t = player[i].pull_seconds;
		if (t < 0.0) continue;
		const double fade_in = Min(1.0, t / 0.3);
		const Vec2 center = player[i].pos[0] + Vec2{ 0.0, -60.0 };
		const Vec2 other = player[1 - i].pos[0] + Vec2{ 0.0, -60.0 };
		const double side = (other.x < center.x) ? -1.0 : 1.0;

		//縮みながら集まる輪
		constexpr int rings = 4;
		constexpr double ring_period = 0.7;
		for (int k = 0; k < rings; k++) {
			const double p = Math::Fraction(t / ring_period + static_cast<double>(k) / rings);
			const double radius = 60.0 + 460.0 * (1.0 - EaseInQuad(p));
			Circle{ center, radius }.drawFrame(4.0 + 6.0 * p, ColorF{ 0.6, 0.85, 1.0, 0.7 * fade_in * p });
		}

		//相手のいる側から吸い込まれる風の筋 (位置と速さは筋ごとに決まった値でばらつかせる)
		constexpr int streaks = 24;
		const double reach = Min(static_cast<double>(yuuka_special_pull_range), Max(abs(other.x - center.x) + 200.0, 500.0));
		for (int k = 0; k < streaks; k++) {
			const double speed = 1.3 + 0.9 * Math::Fraction(k * 0.618);
			const double p = Math::Fraction(t * speed + k * 0.37);
			const double spread = (Math::Fraction(k * 0.4142) - 0.5) * 360.0;
			const double eased = EaseInQuad(p);
			const Vec2 head{ center.x + side * reach * (1.0 - eased), center.y + spread * (1.0 - eased) };
			const Vec2 tail = head + Vec2{ side * (40.0 + 120.0 * eased), spread * 0.25 * eased };
			const double alpha = 0.9 * fade_in * Math::Sin(Math::Pi * p);
			Line{ tail, head }.draw(LineStyle::RoundCap, 6.0, ColorF{ 0.7, 0.9, 1.0, 0.0 }, ColorF{ 0.8, 0.95, 1.0, alpha });
		}
	}
}

void Game::draw_player() const {
	for (int i = 0; i < player_sum; i++) {
		if (!player_flag[i]) continue;
		const auto& player_texture = player_img.at(getData().player[i]).at(player[i].img_number);
		player_texture.mirrored(player[i].direction).drawAt(draw_player_pos(player[i].pos[0], i));
		//必殺技の溜めの点滅
		if (0.0 < player[i].charge_glow) {
			const ScopedRenderStates2D additive{ BlendState::Additive };
			player_texture.mirrored(player[i].direction).drawAt(draw_player_pos(player[i].pos[0], i), ColorF{ 1.0, 0.6 * player[i].charge_glow });
		}
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
	//ゲージ不足で必殺技を出せなかったら、自分のゲージを揺らす
	const double shake[player_sum] = { (player_number == 0) ? commandFeedback.gaugeShake() : 0.0, (player_number == 1) ? commandFeedback.gaugeShake() : 0.0 };

	//1PのAP
	AP_bar_empty_img.mirrored().draw(120 + shake[0], 880);
	if (player[0].special_attack) {
		AP_bar_max_img.mirrored().draw(120 + shake[0], 880);
		{
			const ScopedRenderStates2D blend{ BlendState::Additive };
			fire_img.at(player[0].fire_animation).draw(90 + shake[0], 800);
		}
	}
	else {
		AP_bar_img(0, 0, 360.0 * ((double)player[0].ap / player_max_ap), 42).mirrored().draw(612.0 - 360.0 * ((double)player[0].ap / player_max_ap) + shake[0], 992);
	}


	//2PのAP
	AP_bar_empty_img.draw(1300 + shake[1], 880);
	if (player[1].special_attack) {
		AP_bar_max_img.draw(1300 + shake[1], 880);
		{
			const ScopedRenderStates2D blend{ BlendState::Additive };
			fire_img.at(player[1].fire_animation).draw(1610 + shake[1], 800);
		}
	}
	else {
		AP_bar_img(0, 0, 360.0 * ((double)player[1].ap / player_max_ap), 42).mirrored().draw(1305 + shake[1], 992);
	}
}

int Game::strong_attack_recovery_ms(int character_number) const {
	if (character_number == 1) return yuuka_strong_attack_recovery_ms;
	if (character_number == 2) return airi_strong_attack_recovery_ms;
	return 0;
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
