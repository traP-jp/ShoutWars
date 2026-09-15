# pragma once

// 音声の解析結果を画面に表すための補助関数

# include "MFCC.hpp"
# include <Siv3D.hpp>

/// @brief MFCC を、あいうえおがそれぞれ別の色になる色に変換する
/// @param mfcc 12 次の MFCC
/// @return 色相 (あ 0, い 55, う 125, え 205, お 270) と彩度 (0〜1)。明度は 1。母音の間の音は近い母音の色を混ぜた色になり、どの母音ともつかない音ほど彩度が下がる
/// @remark りすりす 1 人の声で決めた変換なので、声によっては色がずれる。登録や推定には使わず、表示の飾りに使う
[[nodiscard]] HSV VowelColor(const MFCC& mfcc);

/// @brief 画面に表す音量の下限 (dB)。これより小さい音は高さ 0 で描く
inline constexpr double MinVolumeDb = -60.0;

/// @brief 音量 (RMS) を dB にする
[[nodiscard]] double VolumeDb(double rootMeanSquare);

/// @brief dB の音量を、下限から 0 dB までを 0〜1 にした、グラフやメーターでの高さの割合にする
[[nodiscard]] double VolumeLevel(double volumeDb);

/// @brief VowelColor の色を、画面に描くときの色にする
[[nodiscard]] ColorF VowelDisplayColor(const HSV& vowelColor);
