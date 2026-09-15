# pragma once
# include <Siv3D.hpp>

/// @brief CPU の頭脳が見る、1 人のキャラの様子
struct CpuFighter {
	Vec2 pos;
	/// @brief Player::status と同じビット
	int status = 0;
	/// @brief キャラ (0:玲, 1:ユウカ, 2:アイリ, 3:No.0)
	int number = 0;
	int hp = 0;
	bool special_ready = false;
	bool guard_cooling_down = false;
};

struct CpuView {
	int now_ms = 0;
	CpuFighter self;
	CpuFighter opponent;
	int max_hp = 1;
	/// @brief 歩く速さ (px/秒)
	double walk_speed = 0.0;
	double stage_min_x = 0.0;
	double stage_max_x = 0.0;
	/// @brief 相手が言い終えたのに、どのコマンドにも当てはまらなかった発話の累計
	size_t opponent_unmatched_utterances = 0;
};

struct CpuIntent {
	/// @brief 歩く向き (0:止まる, 1:左, 2:右)。押している間と同じく、毎フレーム返す
	int walk = 0;
	bool jump = false;
	/// @brief 始める技 (0:無し。番号は CommandRecognizer の行動と同じ)
	int move = 0;
};

/// @brief 声で操作する人間と同じ制約で、CPU の行動を決める
/// @remark 技はコマンドを言い終えて認識されるまで出ず、発動率に応じて不発になる。相手の様子は少し前のものしか見えない。
/// 強さは、HP で CPU が勝っているときや、相手の声がコマンドに当てはまらないことが続くときに、迷う時間やミスを増やして目立たないように下げる。
/// 初めて遊ぶ人は操作がおぼつかないので、対戦の序盤はさらにゆっくり動き、技も控えめにする
class CpuBrain {
public:
	/// @brief 設定ファイルに cpuSkill が無いときの基準の強さ
	static constexpr double DefaultBaseSkill = 0.35;

	/// @param base_skill 基準の強さ (0〜1)。HP の差や相手の声の当てはまらなさで、ここから上下する
	[[nodiscard]] explicit CpuBrain(double base_skill);

	[[nodiscard]] CpuIntent update(const CpuView& view);

private:
	struct Speech {
		int action;
		int fire_ms;
		bool recognized;
	};

	struct Stroke {
		int walk = 0;
		int end_ms = 0;
	};

	double base_skill;
	Array<CpuView> history;
	Optional<Speech> speech;
	Stroke stroke;
	Optional<int> match_start_ms;
	int next_think_ms = 0;
	Optional<int> jump_ms;
	int escape_until_ms = 0;
	//見えている相手の状態 (技を始めた瞬間を知るため)
	int seen_opponent_status = 0;
	//相手の必殺技に気付いて、まだどうするか決めていない
	bool special_threat_pending = false;
	size_t seen_unmatched_utterances = 0;
	//相手の声がコマンドに当てはまらなかった回数を、時間とともに減らしたもの
	double frustration = 0.0;
	int last_ms = 0;

	[[nodiscard]] double skill(const CpuView& view) const;
	[[nodiscard]] double warmup(int now_ms) const;
	[[nodiscard]] int thinkDelayMs(int now_ms, double skill) const;
	[[nodiscard]] const CpuView& perceive(const CpuView& view, double skill);
	void notice(const CpuView& seen, int now_ms, double skill);
	void think(const CpuView& view, const CpuView& seen, double skill);
	void thinkOffense(const CpuView& view, double distance, double skill);
	void say(int character, int action, int start_ms);
	[[nodiscard]] int fireSpeech(const CpuView& view, const CpuView& seen, double skill);
	[[nodiscard]] int walk(const CpuView& view, const CpuView& seen, double skill);
};

/// @brief 設定ファイルの cpuSkill (0〜1) を読む。無ければ CpuBrain::DefaultBaseSkill
[[nodiscard]] double LoadCpuBaseSkill(FilePathView configPath);
