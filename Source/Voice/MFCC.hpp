# pragma once

# include <Siv3D.hpp>

struct MFCC {
	Array<double> feature; // 特徴量

	/// @brief 特徴量が埋まっていないかを調べる
	/// @return 特徴量が埋まっていれば false
	[[nodiscard]] bool isUnset() const;

	/// @brief ノルムを計算する
	/// @return ノルム
	[[nodiscard]] double norm() const;

	/// @brief もう一つの MFCC とのコサイン類似度を計算する
	/// @param other もう一つの MFCC
	/// @return コサイン類似度
	[[nodiscard]] double cosineSimilarity(const MFCC& other) const;
};
