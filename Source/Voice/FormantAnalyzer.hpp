# pragma once

# include <Siv3D.hpp>

class FormantAnalyzer {
public:
	const Microphone mic;
	const size_t frames;
	const double coefficient;
	const size_t lpcOrder;

	/// @param mic マイク
	/// @param lpcOrder LPC 係数の次数
	[[nodiscard]] explicit FormantAnalyzer(
		Microphone mic, size_t frames = 1024, double coefficient = 0.95, size_t lpcOrder = 64
	);

	/// @brief 現在のマイク音声でフォルマント解析をする
	/// @return 全てのフォルマント
	/// @throw Error マイクが録音中でない
	Array<double> analyze();

	/// @brief 最後の LPC の結果を取得する
	/// @return スペクトル包絡線
	[[nodiscard]] Array<double> getLpcResult() const;

protected:
	template <typename T> static void normalize(Array<T>& data, T scale = 1.0);

	Array<double> lpcResult;
};
