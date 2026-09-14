# pragma once

# include <Siv3D.hpp>

struct VoiceCommand {
	StringView text;
	/// @brief 発音の並び。大文字は母音、小文字は無声化などで聞こえなくてもよい母音、_ は無声子音による無音
	StringView pronunciation;
	int32 action;
};

/// @brief キャラクターが使える音声コマンドを返す
/// @param character キャラクター番号 (0:玲, 1:ユウカ, 2:アイリ, 3:No.0)
[[nodiscard]] Array<VoiceCommand> VoiceCommandsOf(int32 character);

struct CommandRecognizerOptions {
	/// @brief 発話が終わったとみなして判定する無音の長さ (フレーム)
	size_t endSilenceFrames = 12;
	/// @brief 早めに判定する無音の長さ (フレーム)。ここでは earlyThreshold を満たすときだけ発動する
	size_t earlySilenceFrames = 4;
	/// @brief 早めの判定で発動する、1 フレームあたりの平均コストの上限
	double earlyThreshold = 0.3;
	/// @brief 発動しなかった発話に、後の発話をつなげられる無音の長さ (フレーム)
	size_t mergeSilenceFrames = 30;
	/// @brief 発動してから次の発話を聞き始めるまでのフレーム数
	size_t cooldownFrames = 30;
	/// @brief 発話とみなす最小の有声フレーム数
	size_t minVoicedFrames = 4;
	/// @brief 判定に使う発話の最大の長さ (フレーム)
	size_t maxUtteranceFrames = 180;
	/// @brief 各母音が続く最小のフレーム数
	size_t minVowelFrames = 3;
	/// @brief 無声子音による無音が続く最小のフレーム数
	size_t minGapFrames = 2;
	/// @brief 母音と母音の間の、どの母音にも当てはめないフレームのコスト
	double fillerCost = 1.5;
	/// @brief 1 フレームあたりの平均コストがこれ以下なら発動する
	double threshold = 0.7;
	/// @brief 別の行動のコマンドとのコストの差がこれ未満なら、取り違えを避けて発動しない
	double ambiguityMargin = 0.05;
	/// @brief 長いコマンドを優先する、コストの差の許容量
	double longerPreference = 0.05;
};

/// @brief 発話の終わりごとに、コマンドの発音の並びへの当てはまりを比べて判定する
class CommandRecognizer {
public:
	/// @brief フレームの間隔。フレームレートが高くても、この間隔より細かいフレームは捨てる
	static constexpr uint64 FrameIntervalUs = 1'000'000 / 60;

	[[nodiscard]] explicit CommandRecognizer(const CommandRecognizerOptions& options = {});

	/// @brief 音素の推定結果を与え、コマンドを判定する
	/// @param phonemeScores Phoneme::estimate の結果 (無音 2 つと、あいうえおの高低の 12 音素)
	/// @param character キャラクター番号 (0:玲, 1:ユウカ, 2:アイリ, 3:No.0)
	/// @param timeUs 現在時刻 (マイクロ秒)
	/// @return 発動した行動 (0 は無し)
	[[nodiscard]] int32 update(const Array<double>& phonemeScores, int32 character, uint64 timeUs = Time::GetMicrosec());

private:
	// あいうえお と 無音 の順の確率
	using Frame = std::array<double, 6>;

	CommandRecognizerOptions options;
	uint64 nextFrameUs = 0;
	size_t cooldown = 0;
	Array<Frame> utterance;
	size_t voicedFrames = 0;
	size_t silentFrames = 0;

	[[nodiscard]] int32 decide(const Array<Frame>& frames, int32 character, double threshold) const;
	[[nodiscard]] double alignmentCost(const Array<Frame>& frames, StringView pronunciation) const;
};
