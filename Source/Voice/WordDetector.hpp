# pragma once

# include "MFCC.hpp"
# include <Siv3D.hpp>

class WordDetector {
public:
	uint64 coolTime; // 単語が判定されてから次に検出を開始するまでのマイクロ秒のクールタイム
	uint64 wordTimeout; // 母音同士が連続していると判定できるマイクロ病の最大時間
	uint64 wordTimeLimit; // 各母音の可能性のバッファーのマイクロ秒の寿命
	uint64 scoresHistoryLife; // 母音スコアの履歴のマイクロ秒の寿命
	double scoreThreshold; // これに scoreHistoryLife を掛けた値よりも母音スコアの面積が大きい場合にその母音の可能性があるとする

	/// @param coolTime 単語が判定されてから次に検出を開始するまでのマイクロ秒のクールタイム
	/// @param wordTimeout 母音同士が連続していると判定できるマイクロ病の最大時間
	/// @param wordTimeLimit 各母音の可能性のバッファーのマイクロ秒の寿命
	/// @param scoresHistoryLife 母音スコアの履歴のマイクロ秒の寿命
	/// @param scoreThreshold これに scoreHistoryLife を掛けた値よりも母音スコアの面積が大きい場合にその母音の可能性があるとする
	[[nodiscard]] explicit WordDetector(
		uint64 coolTime = 500'000uLL, uint64 wordTimeout = 400'000uLL, uint64 wordTimeLimit = 5'000'000uLL,
		uint64 scoresHistoryLife = 150'000uLL, double scoreThreshold = 0.6
	);

	/// @brief 現在の母音スコアを履歴に追加し、各母音の可能性をバッファーに追加する
	/// @param scores 追加するスコア
	/// @return 追加された各母音の可能性
	HashTable<char32, bool> addScores(HashTable<char32, double> scores);

	/// @brief 各母音の可能性をバッファーの後ろから単語を逆順に調べ、検出したらバッファーを空にする
	/// @param word 検出する単語 (母音のみ/スペース未対応)
	/// @return 単語が検出されたかどうか
	[[nodiscard]] bool detect(String word);

	/// @brief 母音スコアの履歴から各母音の可能性を調べる
	/// @return 各母音の可能性
	[[nodiscard]] HashTable<char32, bool> vowelChances();

	/// @brief 各母音の可能性のバッファーを取得する
	/// @param limit 遡る最大時間のマイクロ秒 (0 で無限)
	/// @return 各母音の可能性のバッファー
	[[nodiscard]] Array<std::pair<uint64, HashTable<char32, bool>>> getVowelsBuffer(uint64 limit = 0uLL) const;

protected:
	Array<std::pair<uint64, HashTable<char32, bool>>> vowelsBuffer;
	Array<std::pair<uint64, HashTable<char32, double>>> scoresHistory;
	uint64 lastDetected;
};
