# include "VoiceVisualization.hpp"

namespace {
	// コーパスの母音ごとの MFCC (雑音抑制あり) の平均。
	// 線形判別分析で平面に落として角度を見る方法も試したが、別のマイクでは平面上の位置がずれて色が回ってしまい、12 次元のまま近さを比べる方が強かった
	constexpr std::array<std::array<double, 12>, 5> VowelCentroids = { {
		{ 23, 26, -10, -39, -23, 11, 2, -2, 3, -2, 13, -2 },
		{ -34, 35, 44, 11, -7, 10, 0, 2, 1, -3, 7, 0 },
		{ -6, 15, 25, 29, -1, -7, 1, 3, 5, -2, -1, 2 },
		{ -7, 12, 42, 15, -26, -6, 2, -3, -1, -3, 2, 3 },
		{ 26, 48, 17, -15, -27, -3, -3, -4, 6, 1, -1, -2 },
	} };
	// あいうえおの色相 (度)。HSV の色相は見え方が均一でないので、変化の目立つ黄の辺りは狭く、見分けにくい緑や青の辺りは広く取る
	constexpr std::array<double, 5> VowelHues = { 0, 55, 125, 205, 270 };
	// 各母音までの距離のこの乗数に反比例する重みで色を混ぜる。大きいほど 2 音の中間で急に色が変わり、小さいほどゆるやかに移る
	constexpr double DistanceExponent = 3.0;
	// 混ぜた色の鮮やかさ (0〜1) をこの乗数で持ち上げる。小さいほど母音の間の音も鮮やかに見える
	constexpr double SaturationExponent = 0.5;
}

HSV VowelColor(const MFCC& mfcc) {
	if (mfcc.feature.size() != VowelCentroids[0].size()) throw Error{ U"VowelColor expects {} MFCC coefficients, but got {}"_fmt(VowelCentroids[0].size(), mfcc.feature.size()) };
	std::array<double, VowelCentroids.size()> weights{};
	double totalWeight = 0.0;
	for (size_t vowel : step(VowelCentroids.size())) {
		double squaredDistance = 0.0;
		for (size_t i : step(mfcc.feature.size())) squaredDistance += Math::Square(mfcc.feature[i] - VowelCentroids[vowel][i]);
		if (squaredDistance == 0.0) return HSV{ VowelHues[vowel], 1.0, 1.0 };
		weights[vowel] = Math::Pow(squaredDistance, -DistanceExponent / 2);
		totalWeight += weights[vowel];
	}

	// 色相を向きとして重み付きで足す。向きが打ち消し合う (色相が定まらず急に回る) 音ほど和が短くなるので、
	// 長さを彩度にすると、色相が回るところは灰色になって色が途切れずにつながる
	Vec2 sum{ 0.0, 0.0 };
	for (size_t vowel : step(VowelCentroids.size())) {
		const double radians = Math::ToRadians(VowelHues[vowel]);
		sum += weights[vowel] / totalWeight * Vec2{ Math::Cos(radians), Math::Sin(radians) };
	}
	return HSV{ Math::Fmod(Math::ToDegrees(Math::Atan2(sum.y, sum.x)) + 360.0, 360.0), Math::Pow(sum.length(), SaturationExponent), 1.0 };
}

double VolumeDb(double rootMeanSquare) {
	return 20.0 * Math::Log10(Max(rootMeanSquare, 1e-6));
}

double VolumeLevel(double volumeDb) {
	return Clamp((volumeDb - MinVolumeDb) / -MinVolumeDb, 0.0, 1.0);
}

ColorF VowelDisplayColor(const HSV& vowelColor) {
	return HSV{ vowelColor.h, vowelColor.s * 0.7, 0.95 };
}
