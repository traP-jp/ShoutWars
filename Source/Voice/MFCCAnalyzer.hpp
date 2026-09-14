# pragma once

# include "MFCC.hpp"
# include <Siv3D.hpp>

struct MFCCOptions {
	size_t order = 12;
	size_t melChannels = 40;
	double minFrequency = 0.0;
	/// @brief 0 ならナイキスト周波数
	double maxFrequency = 8000.0;
	bool hammingWindow = true;
	double preEmphasisCoefficient = 0.97;
};

class MFCCAnalyzer {
public:
	const MFCCOptions options;

	[[nodiscard]] explicit MFCCAnalyzer(const MFCCOptions& options = {});

	/// @brief 音声の断片を解析する
	/// @param samples 解析するサンプル (256 << FFTSampleLength 個)
	/// @param sampleRate サンプリング周波数
	/// @return MFCC
	[[nodiscard]] MFCC analyze(Array<float> samples, uint32 sampleRate) const;

protected:
	static double freqToMel(double freq);
	static double melToFreq(double mel);
};
