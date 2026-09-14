# pragma once

# include "MFCCAnalyzer.hpp"
# include <Siv3D.hpp>

enum class PhonemeDistance {
	Cosine,
	Euclidean,
};

struct PhonemeOptions {
	MFCCOptions mfcc;
	/// @brief 0 なら登録した平均との距離で推定する
	size_t k = 5;
	PhonemeDistance distance = PhonemeDistance::Cosine;
	/// @brief ユークリッド距離を測る前に、登録した特徴量の分散で各次元を正規化する
	bool standardize = true;
};

class Phoneme {
public:
	FilePathView configPath;
	Microphone mic;
	double volumeThreshold;
	uint64 mfccHistoryLife;

	/// @param configPath 設定ファイルのパス
	/// @param defaultVolumeThreshold デフォルトのボリューム閾値
	/// @param n 音素の数
	/// @param mfccHistoryLife MFCC の履歴のマイクロ秒の寿命
	/// @param options 特徴量と推定方法
	[[nodiscard]] explicit Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife = 2'200'000uLL, const PhonemeOptions& options = {});

	/// @brief 録音を開始する (録音中の場合は再開する)
	/// @return 録音の開始に成功したかどうか
	bool start();

	/// @brief 録音を終了する
	void stop();

	/// @brief マイクの音声を解析し音素を推定する (重い処理なので 1 秒に 60 回までしか呼ぶな)
	/// @param frames 音声解析に使うサンプル数 (大きいほど重くなる)
	/// @return それぞれの音素らしさ (大きいほどその音素らしい)
	[[nodiscard]] Array<double> estimate(FFTSampleLength frames = FFTSampleLength::SL2K);

	/// @brief 音声の断片を解析し音素を推定する
	/// @param samples 直近のサンプル
	/// @param sampleRate サンプリング周波数
	/// @param rootMeanSquare 直近 20 ms の音量
	/// @param timeUs 現在時刻 (マイクロ秒)
	/// @return それぞれの音素らしさ (大きいほどその音素らしい)
	[[nodiscard]] Array<double> estimate(Array<float> samples, uint32 sampleRate, double rootMeanSquare, uint64 timeUs);

	/// @brief 登録していない音素があるかを調べる
	bool isMFCCUnset() const;

	/// @brief 直近の MFCC で音素を登録する
	/// @param id 登録する音素の ID (インデックス)
	/// @param timeUs 現在時刻 (マイクロ秒)
	/// @param durationUs 遡る時間 (マイクロ秒)
	/// @throw Error MFCC の履歴が空
	void setMFCC(uint64 id, uint64 timeUs = Time::GetMicrosec(), uint64 durationUs = 1'000'000);

	/// @brief 登録した MFCC の平均を取得する
	/// @param id 音素の ID (インデックス)
	[[nodiscard]] MFCC averageMFCC(size_t id) const;

	/// @brief 設定をファイルに保存する
	/// @return 保存に成功したかどうか
	bool save() const;

	/// @brief MFCC の履歴を取得する
	/// @return マイクロ秒と MFCC の std::map
	[[nodiscard]] const std::map<uint64, MFCC>& getMFCCHistory() const;

protected:
	PhonemeOptions options;
	MFCCAnalyzer mfccAnalyzer;
	Array<Array<MFCC>> registered;
	std::map<uint64, MFCC> mfccHistory;
	Array<double> featureScale;

	[[nodiscard]] Array<float> latestSamples(FFTSampleLength frames) const;
	[[nodiscard]] Array<double> silenceScores() const;
	[[nodiscard]] Array<double> averageScores(const MFCC& mfcc) const;
	[[nodiscard]] double distance(const MFCC& a, const MFCC& b) const;
	[[nodiscard]] Array<double> nearestNeighborScores(const MFCC& mfcc) const;
	void updateFeatureScale();
};
