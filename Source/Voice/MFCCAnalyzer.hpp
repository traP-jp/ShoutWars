# pragma once

# include "MFCC.hpp"
# include <Siv3D.hpp>

class MFCCAnalyzer {
public:
	const Microphone mic;
	const uint64 mfccHistoryLife;
	const size_t mfccOrder;
	const double preEmphasisCoefficient;

	/// @param mic マイク
	/// @param mfccHistoryLife MFCC の履歴のマイクロ秒の寿命
	/// @param mfccOrder MFCC 次数
	/// @param preEmphasisCoefficient 高域強調係数
	[[nodiscard]] explicit MFCCAnalyzer(
		Microphone mic, uint64 mfccHistoryLife = 2'200'000uLL, size_t mfccOrder = 12, float preEmphasisCoefficient = 0.97f
	);

	/// @brief 現在のマイク音声でフォルマント解析をする
	/// @param frames サンプル数
	/// @param melChannels メル周波数の分割数
	/// @return MFCC
	/// @throw Error マイクが録音中でない
	MFCC analyze(FFTSampleLength frames = FFTSampleLength::SL2K, size_t melChannels = 40);

	/// @brief 最後のメルスペクトラムを取得する
	/// @return メルスペクトラム
	[[nodiscard]] Array<double> getMelSpectrum() const;

	/// @brief MFCC の履歴を取得する
	/// @return マイクロ秒と MFCC の std::map の共有ポインタ
	[[nodiscard]] std::shared_ptr<std::map<uint64, MFCC>> getMFCCHistory();

protected:
	static double freqToMel(double freq);
	static double melToFreq(double mel);

	Array<double> melSpectrum;
	std::shared_ptr<std::map<uint64, MFCC>> mfccHistory;

	size_t cleanMFCCHistory();
};
