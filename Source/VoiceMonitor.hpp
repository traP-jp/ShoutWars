# pragma once

# include "common.hpp"

/// @brief 直近の声を覚えて、音量と母音の色の流れるグラフと、聞き取った母音の並びを描く
class VoiceMonitor {
public:
	/// @param area 母音の並びとグラフを描く範囲
	explicit VoiceMonitor(const RectF& area);

	/// @brief 1 フレーム分の解析結果を覚える
	/// @param phoneme 直前に estimate した Phoneme
	/// @param phonemeScores estimate の戻り値
	void update(const Phoneme& phoneme, const Array<double>& phonemeScores);

	void draw() const;

private:
	struct Bar {
		double volumeDb;
		/// @brief 入力感度の閾値を下回ったフレームでは none
		Optional<HSV> color;
	};

	/// @brief 聞き取った母音 1 つ。vowel が none なら、発話の区切り
	struct LoggedVowel {
		Optional<size_t> vowel;
		HSV color;
		double time;
	};

	RectF area;
	Font font{ FontMethod::MSDF, 48, Typeface::Heavy };
	Array<Bar> bars;
	Array<LoggedVowel> vowels;
	double threshold = 0.0;
	Optional<size_t> runVowel;
	size_t runFrames = 0;
	size_t silentFrames = 0;
};
