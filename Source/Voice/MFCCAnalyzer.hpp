# pragma once

# include "MFCC.hpp"
# include <Siv3D.hpp>

struct MFCCOptions {
	size_t order = 12;
	size_t melChannels = 20;
	double minFrequency = 150.0;
	double maxFrequency = 8000.0;
	double preEmphasisCoefficient = 0.97;
};

class MFCCAnalyzer {
public:
	const MFCCOptions options;

	[[nodiscard]] explicit MFCCAnalyzer(const MFCCOptions& options = {});

	/// @brief 音声の断片のメルスペクトルを求める
	/// @param samples 解析するサンプル (256 << FFTSampleLength 個)
	/// @param sampleRate サンプリング周波数
	/// @return メル周波数の各帯域の振幅
	[[nodiscard]] Array<double> melSpectrum(Array<float> samples, uint32 sampleRate) const;

	/// @brief メルスペクトルから MFCC を求める
	/// @param melSpectrum メル周波数の各帯域の振幅
	[[nodiscard]] MFCC cepstrum(const Array<double>& melSpectrum) const;

protected:
	static double freqToMel(double freq);
	static double melToFreq(double mel);
};
