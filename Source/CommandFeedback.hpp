# pragma once

# include "common.hpp"

/// @brief 技が出たとき・出せなかったとき・声がコマンドに当てはまらなかったときに、コマンド一覧の行・技名・AP ゲージの揺れ・「？」で知らせる
class CommandFeedback {
public:
	/// @brief 技が出た
	/// @param player 技を出したプレイヤーの番号
	/// @param action 行動の番号 (CommandRecognizer の action)
	/// @param character 技を出したプレイヤーのキャラ
	/// @param isSelf 自分の技か (自分の技だけコマンド一覧の行を光らせる)
	void started(size_t player, int32 action, int32 character, bool isSelf);

	/// @brief 自分のコマンドを聞き取ったが、技を出せなかった
	/// @param gaugeShortage AP ゲージが足りなかったか
	void blocked(int32 action, bool gaugeShortage = false);

	/// @brief 声が言い終えたのに、どのコマンドにも当てはまらなかった
	/// @param player 声を出したプレイヤーの番号
	/// @param isSelf 自分の声か (自分の声のときだけ、技が出にくいときのヒントを光らせる)
	void unmatched(size_t player, bool isSelf);

	/// @brief 自分の AP ゲージを横に揺らす量
	[[nodiscard]] double gaugeShake() const;

	/// @brief 案内の文とコマンド一覧を、行ごとの強調を付けて描く
	/// @param commandList コマンド一覧の画像
	/// @param pos 案内の文の左上 (コマンド一覧はその下に描く)
	/// @param guardCooldown ガードを壊されてから再びガードできるまでの残りの割合 (0 ならガードできる)
	void drawCommandList(const Texture& commandList, const Vec2& pos, double guardCooldown) const;

	/// @brief 技名を、技を出したプレイヤーの頭の上に描く
	/// @param positions 各プレイヤーの位置
	void drawMoveNames(const Array<Vec2>& positions) const;

	/// @brief 声が当てはまらなかったプレイヤーの頭の上に「？」を描く
	/// @param positions 各プレイヤーの位置
	void drawUnmatchedMarks(const Array<Vec2>& positions) const;

	/// @brief 技が出にくいときのヒントを描く。自分の声が当てはまらなかった直後は光らせる
	/// @param center ヒントの中心
	void drawVoiceHint(const Vec2& center) const;

private:
	struct RowFeedback {
		int32 action;
		bool blocked;
		double time;
	};

	struct MoveName {
		size_t player;
		String text;
		double time;
	};

	Font font{ FontMethod::MSDF, 48, Typeface::Heavy };
	Texture questionMark{ Emoji{ U"❓" } };
	Optional<RowFeedback> row;
	Optional<double> gaugeShortageTime;
	Array<MoveName> moveNames;
	HashTable<size_t, double> unmatchedTimes;
	Optional<double> selfUnmatchedTime;
};
