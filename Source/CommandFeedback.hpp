# pragma once

# include "common.hpp"

/// @brief 技が出たときと出せなかったときに、コマンド一覧の行・技名・AP ゲージの揺れで知らせる
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

	/// @brief 自分の AP ゲージを横に揺らす量
	[[nodiscard]] double gaugeShake() const;

	/// @brief 案内の文とコマンド一覧を、行ごとの強調を付けて描く
	/// @param commandList コマンド一覧の画像
	/// @param pos 案内の文の左上 (コマンド一覧はその下に描く)
	void drawCommandList(const Texture& commandList, const Vec2& pos) const;

	/// @brief 技名を、技を出したプレイヤーの頭の上に描く
	/// @param positions 各プレイヤーの位置
	void drawMoveNames(const Array<Vec2>& positions) const;

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
	Optional<RowFeedback> row;
	Optional<double> gaugeShortageTime;
	Array<MoveName> moveNames;
};
