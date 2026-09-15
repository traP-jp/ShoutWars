# include "NoiseSuppressor.hpp"
# include <rnnoise.h>

NoiseSuppressor::NoiseSuppressor(Blob weights, double dryMix)
	: weights(std::move(weights)), dryMix(static_cast<float>(dryMix)) {
	model = rnnoise_model_from_buffer(this->weights.data(), static_cast<int>(this->weights.size()));
	if (!model) throw Error{ U"RNNoise の重みを読み込めません" };
	state = rnnoise_create(model);
	if (!state) {
		rnnoise_model_free(model);
		throw Error{ U"RNNoise の重みの形式が合いません" };
	}
}

NoiseSuppressor::~NoiseSuppressor() {
	rnnoise_destroy(state);
	rnnoise_model_free(model);
}

Array<float> NoiseSuppressor::process(const Array<float>& samples) {
	// RNNoise は 16 bit 整数の値の範囲の float を受け取る
	constexpr float Scale = 32768.0f;
	const size_t frame = static_cast<size_t>(rnnoise_get_frame_size());
	pending.append(samples);
	Array<float> output;
	size_t pos = 0;
	Array<float> in(frame), out(frame);
	for (; pos + frame <= pending.size(); pos += frame) {
		for (size_t i : step(frame)) in[i] = pending[pos + i] * Scale;
		rnnoise_process_frame(state, out.data(), in.data());
		for (size_t i : step(frame)) output << (out[i] / Scale * (1.0f - dryMix) + pending[pos + i] * dryMix);
	}
	pending.erase(pending.begin(), pending.begin() + pos);
	return output;
}
