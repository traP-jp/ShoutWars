# pragma once

# include <Siv3D.hpp>

class MFCCAnalyzer {
public:
	const Microphone mic;
	const uint64 mfccHistoryLife;
	const size_t mfccOrder;
	const float preEmphasisCoefficient;

	/// @param mic マイク
	/// @param mfccHistoryLife MFCC の履歴のマイクロ秒の寿命
	/// @param mfccOrder MFCC 次数
	/// @param preEmphasisCoefficient 高域強調係数
	[[nodiscard]] explicit MFCCAnalyzer(
		Microphone mic, uint64 mfccHistoryLife = 10'000'000uLL, size_t mfccOrder = 12, float preEmphasisCoefficient = 0.97
	);

	/// @brief 現在のマイク音声でフォルマント解析をする
	/// @param frames サンプル数
	/// @param melChannels メル周波数の分割数
	/// @return MFCC
	/// @throw Error マイクが録音中でない
	Array<float> analyze(FFTSampleLength frames = FFTSampleLength::SL2K, size_t melChannels = 40);

	/// @brief 最後のメルスペクトラムを取得する
	/// @return メルスペクトラム
	[[nodiscard]] Array<float> getMelSpectrum() const;

	/// @brief MFCC の履歴を取得する
	/// @return マイクロ秒と MFCC の std::map
	[[nodiscard]] std::map<uint64, Array<float>> getMFCCHistory();

protected:
	static float freqToMel(float freq);
	static float melToFreq(float mel);

	Array<float> melSpectrum;
	std::map<uint64, Array<float>> mfccHistory;

	size_t cleanMFCCHistory();
};
