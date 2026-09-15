# include "CpuBrain.hpp"

namespace {
	constexpr int WalkLeft = 1;
	constexpr int WalkRight = 2;
	constexpr int StatusGuard = 8;
	constexpr int StatusSpecial = 64;
	constexpr int StatusUnique = 256;
	//この状態の最中は次の技を言い始めない (弱攻撃とジャンプはすぐ終わるので、言い終える頃には動ける)
	constexpr int BusyStatus = 8 | 32 | 64 | 128 | 256;

	constexpr int Airi = 2;

	struct CommandTiming {
		int action;
		//言い終えるまでの時間 (ミリ秒)
		int say_ms;
		double recognition;
	};

	//言い終えるまでの時間はコーパスのふつうの読み方の平均 (叫ぶときはばらつき、遅くなりがちなので、使うときに伸ばす)、発動率は SN 比 10 dB での値 (パラメーター調整の見積もりと同じ)。ガードは測っていないので仮の値
	constexpr std::array<CommandTiming, 5> YuukaCommands = { { { 1, 530, 0.63 }, { 2, 660, 1.00 }, { 3, 1410, 0.74 }, { 4, 500, 0.90 }, { 5, 480, 0.93 } } };
	constexpr std::array<CommandTiming, 6> AiriCommands = { { { 1, 420, 0.52 }, { 2, 270, 0.96 }, { 3, 1220, 0.44 }, { 4, 500, 0.90 }, { 5, 480, 0.93 }, { 6, 540, 0.89 } } };
	//言い終えてから発動するまで
	constexpr double MinRecognitionLatencyMs = 100.0;
	constexpr double MaxRecognitionLatencyMs = 300.0;

	//相手の様子が見えるまでの遅れ (見てから手を動かすまで)
	constexpr double MinPerceptionDelayMs = 250.0;
	constexpr double MaxPerceptionDelayMs = 450.0;
	//近接攻撃とガード破壊が届く距離
	constexpr double MeleeReach = 230.0;
	constexpr double AiriKnifeReach = 130.0;
	//龍虎水雷撃で引き寄せて当てられると見込む距離
	constexpr double YuukaSpecialReach = 700.0;
	//アイリのナイフの必殺技が、出てから飛び始めるまで
	constexpr int AiriKnifeLaunchMs = 1400;
	constexpr double FrustrationHalfLifeMs = 10000.0;
	//対戦の始めから、序盤の控えめな動きが抜けきるまで
	constexpr double WarmupMs = 60000.0;
	//対戦が始まってから、何もしない時間。人間は画面が切り替わってから状況をつかむまでに時間がかかり、初めて遊ぶならなおさら
	constexpr int StartIdleMs = 4000;
	//動き始めてから、最初の技を考えるまで
	constexpr int FirstThinkAfterIdleMs = 2000;

	const CommandTiming& TimingOf(int character, int action) {
		const auto find = [action](const auto& commands) -> const CommandTiming* {
			for (const auto& command : commands) {
				if (command.action == action) return &command;
			}
			return nullptr;
		};
		//CommandRecognizer と同じく、アイリ以外のキャラはユウカのコマンドを使う
		if (const auto* timing = (character == Airi) ? find(AiriCommands) : find(YuukaCommands)) return *timing;
		throw Error{ U"CPU cannot say action {} as character {}"_fmt(action, character) };
	}

	[[nodiscard]] int RandomMs(double min_ms, double max_ms) {
		return static_cast<int>(Random(min_ms, max_ms));
	}

	/// @brief 見えてから、どうするか決めて言い始めるまでの時間。たいていは短いが、見落としてかなり遅れることもある
	[[nodiscard]] int ReactionMs(double skill) {
		const double tail = -std::log(1.0 - Random(0.0, 0.999));
		return static_cast<int>(Math::Lerp(400.0, 250.0, skill) + Math::Lerp(500.0, 250.0, skill) * tail);
	}
}

CpuBrain::CpuBrain(double base_skill) : base_skill(base_skill) {
	if (!InRange(base_skill, 0.0, 1.0)) throw Error{ U"The CPU base skill must be between 0 and 1, but was {}"_fmt(base_skill) };
}

double LoadCpuBaseSkill(FilePathView configPath) {
	const JSON config = JSON::Load(configPath);
	if (!config.isObject() || !config.contains(U"cpuSkill")) return CpuBrain::DefaultBaseSkill;
	if (!config[U"cpuSkill"].isNumber()) throw Error{ U"cpuSkill in the config file must be a number." };
	return config[U"cpuSkill"].get<double>();
}

CpuIntent CpuBrain::update(const CpuView& view) {
	if (!match_start_ms) {
		match_start_ms = view.now_ms;
		next_think_ms = view.now_ms + StartIdleMs + FirstThinkAfterIdleMs;
	}
	frustration *= std::pow(0.5, (view.now_ms - last_ms) / FrustrationHalfLifeMs);
	frustration += static_cast<double>(view.opponent_unmatched_utterances - seen_unmatched_utterances);
	seen_unmatched_utterances = view.opponent_unmatched_utterances;
	last_ms = view.now_ms;

	const double current_skill = skill(view);
	const CpuView& seen = perceive(view, current_skill);
	notice(seen, view.now_ms, current_skill);
	if (view.now_ms < *match_start_ms + StartIdleMs) return {};

	CpuIntent intent;
	intent.move = fireSpeech(view, seen, current_skill);
	if (!speech && (next_think_ms <= view.now_ms)) think(view, seen, current_skill);
	intent.walk = walk(view, seen, current_skill);
	if (jump_ms && (*jump_ms <= view.now_ms)) {
		jump_ms.reset();
		intent.jump = true;
	}
	return intent;
}

double CpuBrain::skill(const CpuView& view) const {
	const double hp_lead = static_cast<double>(view.self.hp - view.opponent.hp) / view.max_hp;
	return Clamp(base_skill - 0.8 * hp_lead - 0.1 * frustration, 0.1, 1.0);
}

/// @return 対戦の始めは 0、WarmupMs 経つと 1
double CpuBrain::warmup(int now_ms) const {
	return Clamp((now_ms - *match_start_ms) / WarmupMs, 0.0, 1.0);
}

/// @brief 技を出してから次の技を考えるまでの時間。この間が、相手が声で攻める隙になる
int CpuBrain::thinkDelayMs(int now_ms, double skill) const {
	return static_cast<int>(Math::Lerp(3000.0, 1200.0, skill) * Math::Lerp(2.5, 1.0, warmup(now_ms)) * Random(0.7, 1.3));
}

const CpuView& CpuBrain::perceive(const CpuView& view, double skill) {
	history << view;
	while ((2 <= history.size()) && (history[1].now_ms <= view.now_ms - static_cast<int>(MaxPerceptionDelayMs))) history.pop_front();
	const int seen_ms = view.now_ms - static_cast<int>(Math::Lerp(MaxPerceptionDelayMs, MinPerceptionDelayMs, skill));
	for (auto it = history.rbegin(); it != history.rend(); ++it) {
		if (it->now_ms <= seen_ms) return *it;
	}
	return history.front();
}

void CpuBrain::notice(const CpuView& seen, int now_ms, double skill) {
	const int started = seen.opponent.status & ~seen_opponent_status;
	seen_opponent_status = seen.opponent.status;
	if (started & StatusSpecial) {
		special_threat_pending = true;
		next_think_ms = Min(next_think_ms, now_ms + ReactionMs(skill));
	}
	//連射は撃ち始めるまで間があるので、見てから跳んでよけられることがある
	if ((started & StatusUnique) && (seen.opponent.number == Airi) && RandomBool(0.3)) {
		jump_ms = now_ms + RandomMs(300, 600);
	}
}

void CpuBrain::think(const CpuView& view, const CpuView& seen, double skill) {
	next_think_ms = view.now_ms + thinkDelayMs(view.now_ms, skill);
	//間合いは目で見積もるので、ずれる
	const double distance = Abs(seen.opponent.pos.x - view.self.pos.x) + Random(-1.0, 1.0) * Math::Lerp(120.0, 40.0, skill);

	if (special_threat_pending) {
		if (!(seen.opponent.status & StatusSpecial)) {
			special_threat_pending = false;
		} else if (view.self.status & BusyStatus) {
			//自分の技の最中はガードも回避もできないので、終わるのを待つ
			next_think_ms = view.now_ms + 100;
			return;
		} else {
			special_threat_pending = false;
			if (!view.self.guard_cooling_down && RandomBool((0.35 + 0.55 * skill) * Math::Lerp(0.5, 1.0, warmup(view.now_ms)))) {
				say(view.self.number, 4, view.now_ms);
			} else if ((seen.opponent.number != Airi) && RandomBool(0.3 + 0.4 * skill)) {
				//引き寄せより歩きの方が速いので、気付くのが早ければ歩いて逃げきれる
				escape_until_ms = view.now_ms + 1500;
				stroke.end_ms = view.now_ms;
			} else if ((seen.opponent.number == Airi) && RandomBool(0.5)) {
				jump_ms = seen.now_ms + AiriKnifeLaunchMs;
			}
			return;
		}
	}
	if (view.self.status & BusyStatus) {
		next_think_ms = view.now_ms + 100;
		return;
	}
	if ((seen.opponent.status & StatusGuard) && (distance < 450.0) && RandomBool(0.3 + 0.4 * skill)) {
		say(view.self.number, 5, view.now_ms);
		return;
	}
	const bool special_reaches = (view.self.number == Airi) || (distance < YuukaSpecialReach);
	if (view.self.special_ready && special_reaches && RandomBool(0.3 + 0.4 * skill)) {
		say(view.self.number, 3, view.now_ms);
		return;
	}
	//相手の攻撃が届きそうな距離では、先読みしてガードすることがある。相手が「壊せ」を言う機会にもなる
	if ((distance < 300.0) && !view.self.guard_cooling_down && RandomBool(0.05 + 0.25 * skill)) {
		say(view.self.number, 4, view.now_ms);
		return;
	}
	thinkOffense(view, distance, skill);
}

void CpuBrain::thinkOffense(const CpuView& view, double distance, double skill) {
	if (!RandomBool(Math::Lerp(0.3, 0.8, warmup(view.now_ms)))) return;
	const int character = view.self.number;
	if (character == Airi) {
		if (distance < AiriKnifeReach) {
			say(character, 2, view.now_ms);
		} else if (250.0 < distance) {
			say(character, RandomBool(0.3) ? 6 : 1, view.now_ms);
		}
		return;
	}
	const int action = RandomBool(0.5) ? 1 : 2;
	//言っている間に歩いて詰めれば、言い終える頃に届く。この読みができるかは強さ次第
	const double reach_while_saying = MeleeReach + view.walk_speed * TimingOf(character, action).say_ms / 1000.0;
	if ((distance < MeleeReach) || ((distance < reach_while_saying) && RandomBool(0.5 * skill))) {
		say(character, action, view.now_ms);
	}
}

void CpuBrain::say(int character, int action, int start_ms) {
	const CommandTiming& timing = TimingOf(character, action);
	const int say_ms = static_cast<int>(timing.say_ms * Random(0.9, 1.4));
	speech = Speech{ .action = action, .fire_ms = start_ms + say_ms + RandomMs(MinRecognitionLatencyMs, MaxRecognitionLatencyMs), .recognized = RandomBool(timing.recognition) };
	//言い始めたら、歩き方を決め直す (攻撃を言っている間は相手から離れない)
	stroke.end_ms = start_ms;
}

int CpuBrain::fireSpeech(const CpuView& view, const CpuView& seen, double skill) {
	if (!speech || (view.now_ms < speech->fire_ms)) return 0;
	const Speech fired = *speech;
	speech.reset();
	if (fired.recognized) {
		//相手の必殺技に気付いていれば、技の後の間を置かずに対応を考える
		if (!special_threat_pending) next_think_ms = view.now_ms + thinkDelayMs(view.now_ms, skill);
		return fired.action;
	}
	//認識されなかったら、まだ意味があれば言い直す
	const bool still_useful = (fired.action == 4) ? (seen.opponent.status & StatusSpecial) : ((fired.action == 5) ? (seen.opponent.status & StatusGuard) : true);
	if (still_useful && RandomBool(0.3 + 0.5 * skill)) {
		say(view.self.number, fired.action, view.now_ms + RandomMs(150, 300));
	}
	return 0;
}

int CpuBrain::walk(const CpuView& view, const CpuView& seen, double skill) {
	if (view.now_ms < stroke.end_ms) return stroke.walk;

	const double self_x = view.self.pos.x;
	const double opponent_x = seen.opponent.pos.x;
	const double distance = Abs(opponent_x - self_x);
	const int toward = (opponent_x < self_x) ? WalkLeft : WalkRight;
	const int away = (toward == WalkLeft) ? WalkRight : WalkLeft;
	const double room_behind = (away == WalkLeft) ? (self_x - view.stage_min_x) : (view.stage_max_x - self_x);
	const bool saying_attack = speech && (speech->action != 4);
	const bool saying_melee = saying_attack && ((speech->action == 5) || (speech->action == 2) || ((view.self.number != Airi) && (speech->action == 1)));

	//いたい距離の範囲
	const auto [near, far] = saying_melee ? std::pair{ 60.0, 180.0 } : ((view.self.number == Airi) ? std::pair{ 400.0, 700.0 } : std::pair{ 100.0, 220.0 });
	const double target = (near + far) / 2.0;
	const bool escaping = (view.now_ms < escape_until_ms);
	int direction = 0;
	if (escaping) {
		direction = away;
	} else if (far < distance) {
		direction = toward;
	//アイリは近づかれると離れるが、逃げ続けると追いつけないので、たまにしか離れず、端の近くでは離れない
	} else if ((view.self.number == Airi) && (distance < 250.0) && (300.0 < room_behind) && RandomBool(0.3)) {
		direction = away;
	}
	//迷って止まる
	if (!escaping && RandomBool(Math::Lerp(0.7, 0.45, warmup(view.now_ms)) - 0.2 * skill)) direction = 0;
	if (saying_attack && (direction == away)) direction = 0;
	if (((direction == WalkLeft) && (self_x < view.stage_min_x + 30.0)) || ((direction == WalkRight) && (view.stage_max_x - 30.0 < self_x))) direction = 0;

	const int duration_ms = escaping
		? (escape_until_ms - view.now_ms)
		: direction
		? Clamp(static_cast<int>(Abs(distance - target) / view.walk_speed * 1000.0 * Random(0.6, 1.4)), 150, 900)
		: RandomMs(500, 1500);
	stroke = Stroke{ .walk = direction, .end_ms = view.now_ms + duration_ms };
	return direction;
}
