# include "MFCCAnalyzer.hpp"

# include <complex>

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

MFCCAnalyzer::MFCCAnalyzer(Microphone mic, uint64 mfccHistoryLife, size_t mfccOrder, float preEmphasisCoefficient)
	: mic(mic), mfccHistoryLife(mfccHistoryLife), mfccOrder(mfccOrder), preEmphasisCoefficient(preEmphasisCoefficient) {}

Array<float> MFCCAnalyzer::analyze(FFTSampleLength frames, size_t melChannels) {
	if (!mic.isRecording()) throw Error{ U"mic must be recording" };

	// get data from mic
	Array<float> f(256uLL << FromEnum(frames), 0.0f);
	const auto& buffer = mic.getBuffer();
	const size_t writePos = mic.posSample();
	for (size_t pos : step(f.size())) {
		const size_t idx = (pos + writePos < f.size() ? mic.getBufferLength() : 0) + pos + writePos - f.size();
		f[pos] = buffer[idx].left; // NOTE: Use only one side!
	}

	// pre-emphasis
	for (size_t i : Range(f.size() - 1, 1, -1)) f[i] -= f[i - 1] * preEmphasisCoefficient;

	// hamming window
	for (size_t i : Range(f.size() - 2, 1)) f[i] *= 0.54f - 0.46f * cos(2 * Math::Pi * i / (f.size() - 1));
	f.front() = 0.0f;
	f.back() = 0.0f;

	// normalize
	const auto factor = ranges::max(f | views::transform([](auto x) { return abs(x); }));
	if (factor >= 1e-8f) for (auto&& x : f) x *= 1.0f / factor;

	// FFT
	FFTResult fftResult;
	FFT::Analyze(fftResult, f.data(), f.size(), mic.getSampleRate(), frames);

	// apply mel filter bank
	const float melMax = freqToMel(mic.getSampleRate() / 2);
	const float melMin = freqToMel(0);
	const float deltaMel = (melMax - melMin) / (melChannels + 1);
	Array<float> melPoints(melChannels + 2);
	for (size_t i : step(melPoints.size())) melPoints[i] = melToFreq(melMin + i * deltaMel);
	Array<size_t> bin(melChannels + 2);
	for (size_t i : step(bin.size())) bin[i] = floor((f.size() + 1) * melPoints[i] / mic.getSampleRate());
	melSpectrum.resize(melChannels);
	for (size_t i : step(melChannels)) {
		melSpectrum[i] = 0.0f;
		for (size_t j : Range(bin[i], bin[i + 1] - 1)) {
			melSpectrum[i] += fftResult.buffer[j] * (j - bin[i]) / (bin[i + 1] - bin[i]);
		}
		for (size_t j : Range(bin[i + 1], bin[i + 2] - 1)) {
			melSpectrum[i] += fftResult.buffer[j] * (bin[i + 2] - j) / (bin[i + 2] - bin[i + 1]);
		}
	}

	// DCT
	Array<float> mfcc(mfccOrder, 0.0f);
	for (size_t i : Range(1, mfccOrder)) {
		for (size_t j : step(melChannels)) {
			mfcc[i - 1] += log10(abs(melSpectrum[j])) * cos(Math::Pi * i * (j + 0.5) / melChannels) * 10;
		}
	}

	cleanMFCCHistory();
	return mfccHistory[Time::GetMicrosec()] = mfcc;
}

Array<float> MFCCAnalyzer::getMelSpectrum() const {
	return melSpectrum;
}

map<uint64, Array<float>> MFCCAnalyzer::getMFCCHistory() {
	cleanMFCCHistory();
	return mfccHistory;
}

float MFCCAnalyzer::freqToMel(float freq) {
	return 1127.01f * log(1.0f + freq / 700.0f);
}

float MFCCAnalyzer::melToFreq(float mel) {
	return 700.0f * (exp(mel / 1127.01f) - 1.0f);
}

size_t MFCCAnalyzer::cleanMFCCHistory() {
	const auto now = Time::GetMicrosec();
	return erase_if(mfccHistory, [this, now](const auto& p) { return now - p.first > mfccHistoryLife; });
}
