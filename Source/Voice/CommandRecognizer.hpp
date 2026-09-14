# pragma once

# include "WordDetector.hpp"
# include <Siv3D.hpp>

struct VoiceCommand {
	StringView text;
	StringView vowels;
	int32 action;
};

/// @brief キャラクターが使える音声コマンドを、判定する順に返す
/// @param character キャラクター番号 (0:玲, 1:ユウカ, 2:アイリ, 3:No.0)
[[nodiscard]] Array<VoiceCommand> VoiceCommandsOf(int32 character);

class CommandRecognizer {
public:
	/// @brief 音素の推定結果を与え、コマンドを判定する
	/// @param phonemeScores Phoneme::estimate の結果
	/// @param character キャラクター番号 (0:玲, 1:ユウカ, 2:アイリ, 3:No.0)
	/// @param timeUs 現在時刻 (マイクロ秒)
	/// @return 発動した行動 (0 は無し)
	[[nodiscard]] int32 update(const Array<double>& phonemeScores, int32 character, uint64 timeUs = Time::GetMicrosec());

private:
	WordDetector wordDetector;
};
