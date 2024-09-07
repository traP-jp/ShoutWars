# pragma once

# include <Siv3D.hpp>

class WordDetector {
public:
	uint64 vowelHistoryLife;

	/// @param vowelHistoryLife 母音の履歴のマイクロ秒の寿命
	[[nodiscard]] explicit WordDetector(uint64 vowelHistoryLife = 500'000uLL);

	/// @brief 現在の母音を履歴に追加し、ピーク母音をバッファーに追加する
	/// @param vowel 追加する母音
	/// @return 追加されたピーク母音
	char32 addVowel(char32 vowel);

	/// @brief 母音のバッファーの最後の単語を調べ、検出したらバッファーを空にする
	/// @param word 検出する単語 (母音のみ)
	/// @return 単語が検出されたかどうか
	[[nodiscard]] bool detect(String word);

	/// @brief 母音の履歴から最も多い母音を調べる
	/// @return ピーク母音
	[[nodiscard]] char32 peak();

	/// @brief 母音のバッファーを取得する
	/// @param limit 文字数上限 (0 で無限)
	/// @return 母音バッファー
	[[nodiscard]] StringView getVowelBuffer(size_t limit = 0) const;

protected:
	String vowelBuffer;
	Array<std::pair<uint64, char32>> vowelHistory;
};
