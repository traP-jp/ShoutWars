# include "FormantAnalyzer.hpp"

# include <complex>

using namespace std;

/*
 * Thanks to
 *
 * - [フォルマントから母音推定してリップシンクを目指してみる - 凹みTips](https://tips.hecomi.com/entry/20131110/1384096497)
 * - [LipSyncをDIYする（前編） - Activ8 Tech Blog](https://synamon.hatenablog.com/entry/2018/09/15/200002)
 * - [LipSyncをDIYする（後編） - Activ8 Tech Blog](https://synamon.hatenablog.com/entry/2018/09/15/200105)
 *
 * 以下のコードは <https://gist.github.com/hecomi/7398975> を真似して書いています。hecomi さんにでかでかありがとう。
 */

FormantAnalyzer::FormantAnalyzer(Microphone mic, size_t frames, double coefficient, size_t lpcOrder)
	: mic(mic), frames(frames), coefficient(coefficient), lpcOrder(lpcOrder) {}

Array<double> FormantAnalyzer::analyze() {
	if (!mic.isRecording()) throw Error{ U"mic must be recording" };

	// get data from mic
	Array<double> data(frames + 1, 0.0);
	const auto& buffer = mic.getBuffer();
	const size_t writePos = mic.posSample();
	for (size_t pos : step(frames)) {
		if (pos + writePos < frames) continue;
		const auto& sample = buffer[pos + writePos - frames];
		data[pos] = (sample.left + sample.right) * 0.5;
	}
	normalize(data);

	for (size_t i : Range(data.size() - 2, 1, -1)) {
		// pre-emphasis
		data[i] -= data[i - 1] * coefficient;
		// hamming window
		data[i] *= 0.54 - 0.46 * cos(2 * Math::Pi * i / (data.size() - 1));
	}
	data.front() = 0.0;
	data.back() = 0.0;
	normalize(data);

	// autocorrelation
	Array<double> r(lpcOrder + 1, 0.0);
	for (size_t l : Range(0, lpcOrder)) {
		for (size_t n : step(data.size() - l)) r[l] += data[n] * data[n + l];
	}

	// Levinson-Durbin algorithm
	Array<double> a(lpcOrder + 1, 0.0), e(lpcOrder + 1, 0.0);
	a[0] = e[0] = 1.0;
	a[1] = -r[1] / r[0];
	e[1] = r[0] + r[1] * a[1];
	for (size_t k : Range(1, lpcOrder - 1)) {
		double lambda = 0.0;
		for (size_t j : Range(0, k)) lambda -= a[j] * r[k + 1 - j];
		lambda /= e[k];
		Array<double> u(k + 2), v(k + 2);
		u[0] = 1.0;
		v[0] = 0.0;
		for (size_t i : Range(1, k)) {
			u[i] = a[i];
			v[k + 1 - i] = a[i];
		}
		u[k + 1] = 0.0;
		v[k + 1] = 1.0;
		for (size_t i : Range(0, k + 1)) a[i] = u[i] + lambda * v[i];
		e[k + 1] = e[k] * (1.0 - lambda * lambda);
	}

	// digital filter
	lpcResult.resize(data.size());
	for (size_t n : step(lpcResult.size())) {
		auto z = exp(complex<double>(0.0, -2.0 * Math::Pi * n / lpcResult.size()));
		complex<double> numerator(0.0, 0.0), denominator(0.0, 0.0);
		for (size_t i : step(e.size())) numerator += e[e.size() - 1 - i] * pow(z, i);
		for (size_t i : step(a.size())) denominator += a[a.size() - 1 - i] * pow(z, i);
		lpcResult[n] = abs(numerator / denominator);
	}
	normalize(lpcResult);

	// extract peaks
	Array<double> formants;
	size_t l = 0;
	for (size_t i : Range(1, lpcResult.size() - 2)) {
		if (lpcResult[i] > lpcResult[i - 1]) l = i;
		if (lpcResult[i] == lpcResult[l] && lpcResult[i] > lpcResult[i + 1]) {
			formants << (i + l) * 0.5 * mic.getSampleRate() / frames;
		}
	}
	return formants;
}

Array<double> FormantAnalyzer::getLpcResult() const {
	return lpcResult;
}

template <typename T> void FormantAnalyzer::normalize(Array<T>& data, T scale) {
	const double factor = ranges::max(data | views::transform([](T x) { return abs(x); }));
	if (factor < 1e-8) return;
	for (auto&& x : data) x *= scale / factor;
}
