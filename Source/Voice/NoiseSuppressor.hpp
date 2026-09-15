# pragma once

# include <Siv3D.hpp>

struct DenoiseState;
struct RNNModel;

/// @brief RNNoise で 48 kHz の音声の雑音を抑制する
class NoiseSuppressor {
public:
	static constexpr uint32 SampleRate = 48000;

	/// @param weights RNNoise の学習済みの重み
	/// @param dryMix 抑制前の音声を混ぜる割合 (抑制で声まで削りすぎないように)
	/// @throw Error 重みを読み込めない
	[[nodiscard]] explicit NoiseSuppressor(Blob weights, double dryMix = 0.1);

	~NoiseSuppressor();

	NoiseSuppressor(const NoiseSuppressor&) = delete;
	NoiseSuppressor& operator=(const NoiseSuppressor&) = delete;

	/// @brief 音声を 10 ms ずつ処理し、処理し終えた分を返す (端数は次の呼び出しに持ち越す)
	/// @param samples 続きの音声
	/// @return 雑音を抑制した音声
	[[nodiscard]] Array<float> process(const Array<float>& samples);

private:
	Blob weights;
	RNNModel* model = nullptr;
	DenoiseState* state = nullptr;
	float dryMix;
	Array<float> pending;
};
