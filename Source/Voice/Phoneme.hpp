# pragma once

# include "MFCCAnalyzer.hpp"
# include <Siv3D.hpp>

class Phoneme {
public:
	static constexpr size_t mfccOrder = 12;

	FilePathView configPath;
	Microphone mic;
	double volumeThreshold;
	Array<MFCC> mfccList; // 0 はノイズ
	uint64 mfccHistoryLife;

	/// @param configPath 設定ファイルのパス
	/// @param defaultVolumeThreshold デフォルトのボリューム閾値
	/// @param n 音素の数
	/// @param mfccHistoryLife MFCC の履歴のマイクロ秒の寿命
	[[nodiscard]] explicit Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife = 2'200'000uLL);

	/// @brief 録音を開始する (録音中の場合は再開する)
	/// @return 録音の開始に成功したかどうか
	bool start();

	/// @brief 録音を終了する
	void stop();

	/// @brief 音声を解析し音素を推定する (重い処理なので 1 秒に 60 回までしか呼ぶな)
	/// @param frames 音声解析に使うサンプル数 (大きいほど重くなる)
	/// @return 推定された音素の ID (インデックス)
	[[nodiscard]] size_t estimate(FFTSampleLength frames = FFTSampleLength::SL2K);

	/// @brief MFCC を全て設定できていないかを調べる
	/// @return mfccList が全て埋まっていないかどうか
	bool isMFCCUnset() const;

	/// @brief 現在の MFCC で音素を登録する
	/// @param id 登録する音素の ID (インデックス)
	/// @throw Error 録音中でないか MFCC の履歴が空
	void setMFCC(uint64 id);

	/// @brief 設定をファイルに保存する
	/// @return 保存に成功したかどうか
	bool save() const;

	/// @brief MFCC の履歴を取得する
	/// @return マイクロ秒と MFCC の std::map の共有ポインタ
	/// @throw Error 録音中でない
	[[nodiscard]] std::shared_ptr<std::map<uint64, MFCC>> getMFCCHistory() const;

protected:
	std::unique_ptr<MFCCAnalyzer> mfccAnalyzer;
};
