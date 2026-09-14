# include "MFCCAnalyzer.hpp"

# include <algorithm>
# include <bit>
# include <ranges>

using namespace std;

/*
 * Thanks to
 *
 * - [フォルマントから母音推定してリップシンクを目指してみる - 凹みTips](https://tips.hecomi.com/entry/20131110/1384096497)
 * - [LipSyncをDIYする（前編） - Activ8 Tech Blog](https://synamon.hatenablog.com/entry/2018/09/15/200002)
 * - [LipSyncをDIYする（後編） - Activ8 Tech Blog](https://synamon.hatenablog.com/entry/2018/09/15/200105)
 * - [音響特徴量「メルスペクトル」と「MFCC（メル周波数ケプストラム係数）」の解説と実例紹介 | Hmcomm株式会社](https://fast-d.hmcom.co.jp/techblog/melspectrum-mfcc/)
 * - [MFCC（メル周波数ケプストラム係数）入門 #Python - Qiita](https://qiita.com/tmtakashi_dist/items/eecb705ea48260db0b62)
 * - [uLipSync のアルゴリズム改善を行ってみた - 凹みTips](https://tips.hecomi.com/entry/2023/03/31/022324)
 */

MFCCAnalyzer::MFCCAnalyzer(const MFCCOptions& options) : options(options) {}

MFCC MFCCAnalyzer::analyze(Array<float> f, uint32 sampleRate) const {
	const auto frames = FFTSampleLength(countr_zero(f.size()) - 8);
	const size_t melChannels = options.melChannels;

	for (size_t i = f.size() - 1; i >= 1; --i) f[i] -= static_cast<float>(f[i - 1] * options.preEmphasisCoefficient);

	if (options.hammingWindow) {
		for (size_t i : step(f.size())) f[i] *= static_cast<float>(0.54 - 0.46 * cos(2 * Math::Pi * i / (f.size() - 1)));
	}
	else {
		f.front() = 0.0f;
		f.back() = 0.0f;
	}

	FFTResult fftResult;
	FFT::Analyze(fftResult, f.data(), f.size(), sampleRate, frames);

	const double maxFrequency = options.maxFrequency > 0.0 ? options.maxFrequency : sampleRate / 2.0;
	const double melMax = freqToMel(maxFrequency);
	const double melMin = freqToMel(options.minFrequency);
	const double deltaMel = (melMax - melMin) / (melChannels + 1);
	Array<size_t> bin(melChannels + 2);
	for (size_t i : step(bin.size())) {
		bin[i] = static_cast<size_t>(floor((f.size() + 1) * melToFreq(melMin + i * deltaMel) / sampleRate));
	}
	Array<double> melSpectrum(melChannels);
	for (size_t i : step(melChannels)) {
		for (size_t j = bin[i]; j < bin[i + 1]; ++j) {
			melSpectrum[i] += fftResult.buffer[j] * (j - bin[i]) / (bin[i + 1] - bin[i]);
		}
		for (size_t j = bin[i + 1]; j < bin[i + 2]; ++j) {
			melSpectrum[i] += fftResult.buffer[j] * (bin[i + 2] - j) / (bin[i + 2] - bin[i + 1]);
		}
		// 無音で log(0) にならないようにする
		melSpectrum[i] = log10(melSpectrum[i] + 1e-10);
	}

	MFCC mfcc{ Array<double>(options.order, 0.0) };
	for (size_t i : step(options.order)) {
		for (size_t j : step(melChannels)) {
			mfcc.feature[i] += melSpectrum[j] * cos(Math::Pi * (i + 1) * (j + 0.5) / melChannels) * 10;
		}
	}
	return mfcc;
}

double MFCCAnalyzer::freqToMel(double freq) {
	return 1127.01 * log(1.0 + freq / 700.0);
}

double MFCCAnalyzer::melToFreq(double mel) {
	return 700.0 * (exp(mel / 1127.01) - 1.0);
}
