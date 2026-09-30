# pragma once

# include "common.hpp"

/// @brief 直近の声を覚えて、音量と母音の色の流れるグラフを描く
/// @remark 母音の文字は出さない。単語判定はフレームごとの最大値の並びを答えではなく基準線として使うので、文字にすると判定とずれて誤解を招く (#47)
class VoiceMonitor {
public:
	/// @param area グラフを描く範囲
	explicit VoiceMonitor(const RectF& area);

	/// @brief 1 フレーム分の解析結果を覚える
	/// @param phoneme 直前に estimate した Phoneme
	void update(const Phoneme& phoneme);

	void draw() const;

private:
	struct Bar {
		double volumeDb;
		/// @brief 入力感度の閾値を下回ったフレームでは none
		Optional<HSV> color;
	};

	RectF area;
	Array<Bar> bars;
	double threshold = 0.0;
};
