# pragma once

# include "MFCC.hpp"
# include <Siv3D.hpp>

class MFCCAnalyzer {
public:
	const size_t mfccOrder;
	const double preEmphasisCoefficient;

	/// @param mfccOrder MFCC 次数
	/// @param preEmphasisCoefficient 高域強調係数
	[[nodiscard]] explicit MFCCAnalyzer(size_t mfccOrder = 12, float preEmphasisCoefficient = 0.97f);

	/// @brief 音声の断片を解析する
	/// @param samples 解析するサンプル (256 << FFTSampleLength 個)
	/// @param sampleRate サンプリング周波数
	/// @param melChannels メル周波数の分割数
	/// @return MFCC
	[[nodiscard]] MFCC analyze(Array<float> samples, uint32 sampleRate, size_t melChannels = 40) const;

protected:
	static double freqToMel(double freq);
	static double melToFreq(double mel);
};
