# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife, const PhonemeOptions& options)
	: configPath(configPath), volumeThreshold(defaultVolumeThreshold), mfccHistoryLife(mfccHistoryLife),
	options(options), mfccAnalyzer(options.mfcc), registeredSpectra(n) {
	if (n < 1) throw Error{ U"The number of phoneme must not be empty" };
	JSON config = JSON::Load(configPath);
	try {
		if (config && config.isObject()) {
			if (config[U"volumeThreshold"].isNumber()) {
				volumeThreshold = config[U"volumeThreshold"].get<double>();
			}
			if (config[U"phonemeSpectra"].isArray()) {
				for (size_t id : step(Min(n, config[U"phonemeSpectra"].size()))) {
					for (const auto& spectrum : config[U"phonemeSpectra"][id].arrayView()) {
						if (!spectrum.isArray() || spectrum.size() != options.mfcc.melChannels) continue;
						registeredSpectra[id] << Array<double>(spectrum.size(), Arg::generator = [&, i = size_t{ 0 }]() mutable { return spectrum[i++].get<double>(); });
					}
				}
			}
		}
	}
	catch (...) {
		// 設定ファイルの読み込みに失敗した際は無いものとして扱うため、握りつぶす
		registeredSpectra = Array<Array<Array<double>>>(n);
	}
	updateFeatures();
}

bool Phoneme::start() {
	noiseSuppressor.reset();
	suppressedSamples.clear();
	mic = options.noiseSuppressionWeights
		? Microphone{ unspecified, NoiseSuppressor::SampleRate, SecondsF{ 1.0 }, Loop::Yes, StartImmediately::Yes }
		: Microphone{};
	if (mic.isRecording()) {
		noiseSuppressor = make_unique<NoiseSuppressor>(Blob{ options.noiseSuppressionWeights }, options.noiseSuppressionDryMix);
		micReadPos = mic.posSample();
	}
	else {
		mic = Microphone{ StartImmediately::Yes };
	}
	spectrumHistory.clear();
	mfccHistory.clear();
	return mic.isRecording();
}

void Phoneme::stop() {
	mic.stop();
}

Array<double> Phoneme::estimate(FFTSampleLength frames) {
	if (!mic.isRecording()) return silenceScores();
	if (noiseSuppressor) suppressNewSamples();
	return estimate(latestSamples(frames), mic.getSampleRate(), rootMeanSquare(), Time::GetMicrosec());
}

double Phoneme::rootMeanSquare() const {
	if (!noiseSuppressor) return mic.rootMeanSquare();
	const size_t length = Min<size_t>(NoiseSuppressor::SampleRate / 50, suppressedSamples.size());
	double sum = 0.0;
	for (size_t i : step(length)) sum += Math::Square(static_cast<double>(suppressedSamples[suppressedSamples.size() - length + i]));
	return length ? Math::Sqrt(sum / length) : 0.0;
}

Array<double> Phoneme::estimate(Array<float> samples, uint32 sampleRate, double rootMeanSquare, uint64 timeUs) {
	auto spectrum = mfccAnalyzer.melSpectrum(std::move(samples), sampleRate);
	const auto currentMFCC = mfccAnalyzer.cepstrum(spectrum);
	const bool loud = 20.0 * log10(rootMeanSquare / volumeThreshold) >= options.silenceMarginDb;
	const auto expired = [this, timeUs](const auto& p) { return timeUs - p.first > mfccHistoryLife; };
	erase_if(spectrumHistory, expired);
	erase_if(mfccHistory, expired);
	spectrumHistory[timeUs] = std::move(spectrum);
	mfccHistory[timeUs] = currentMFCC;

	if (rootMeanSquare < volumeThreshold || isMFCCUnset()) return silenceScores();
	return options.k ? nearestNeighborScores(currentMFCC, loud) : averageScores(currentMFCC);
}

bool Phoneme::isMFCCUnset() const {
	return registeredSpectra.any([](const auto& spectra) { return spectra.isEmpty(); });
}

bool Phoneme::isMFCCUnset(size_t id) const {
	return registeredSpectra.at(id).isEmpty();
}

void Phoneme::setMFCC(uint64 id, uint64 timeUs, uint64 durationUs) {
	if (spectrumHistory.empty()) throw Error{ U"MFCC history is empty" };
	Array<Array<double>> spectra;
	for (const auto& [historyUs, spectrum] : spectrumHistory) {
		if (historyUs + durationUs >= timeUs) spectra << spectrum;
	}
	setSpectra(id, std::move(spectra));
}

void Phoneme::setSpectra(size_t id, Array<Array<double>> spectra) {
	registeredSpectra.at(id) = std::move(spectra);
	updateFeatures();
}

const Array<double>& Phoneme::latestSpectrum() const {
	if (spectrumHistory.empty()) throw Error{ U"Spectrum history is empty" };
	return spectrumHistory.rbegin()->second;
}

void Phoneme::suppressNewSamples() {
	const auto& buffer = mic.getBuffer();
	const size_t bufferLength = mic.getBufferLength();
	const size_t writePos = mic.posSample();
	// 雑音抑制が追いつかないときに溜め込むと、次のフレームがさらに重くなるので、古い音声は捨てる
	constexpr size_t MaxBacklog = NoiseSuppressor::SampleRate / 10;
	const size_t available = (writePos + bufferLength - micReadPos) % bufferLength;
	const size_t skipped = available > MaxBacklog ? available - MaxBacklog : 0;
	Array<float> fresh(available - skipped);
	for (size_t i : step(fresh.size())) fresh[i] = buffer[(micReadPos + skipped + i) % bufferLength].left;
	micReadPos = writePos;
	suppressedSamples.append(noiseSuppressor->process(fresh));
	constexpr size_t KeptSamples = 8192;
	if (suppressedSamples.size() > KeptSamples) suppressedSamples.erase(suppressedSamples.begin(), suppressedSamples.end() - KeptSamples);
}

MFCC Phoneme::averageMFCC(size_t id) const {
	MFCC average{ Array<double>(options.mfcc.order, 0.0) };
	for (const auto& mfcc : registered[id]) {
		for (size_t i : step(options.mfcc.order)) average.feature[i] += mfcc.feature[i] / registered[id].size();
	}
	return average;
}

bool Phoneme::save() const {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	config[U"volumeThreshold"] = volumeThreshold;
	config[U"phonemeSpectra"] = registeredSpectra;
	return config.save(configPath);
}

const map<uint64, MFCC>& Phoneme::getMFCCHistory() const {
	return mfccHistory;
}

Array<float> Phoneme::latestSamples(FFTSampleLength frames) const {
	Array<float> f(256uLL << FromEnum(frames), 0.0f);
	if (noiseSuppressor) {
		const size_t length = Min(f.size(), suppressedSamples.size());
		std::copy(suppressedSamples.end() - length, suppressedSamples.end(), f.end() - length);
		return f;
	}
	const auto& buffer = mic.getBuffer();
	const size_t writePos = mic.posSample();
	for (size_t pos : step(f.size())) {
		const size_t idx = (pos + writePos < f.size() ? mic.getBufferLength() : 0) + pos + writePos - f.size();
		f[pos] = buffer[idx].left; // NOTE: Use only one side!
	}
	return f;
}

Array<double> Phoneme::silenceScores() const {
	Array<double> scores(registeredSpectra.size(), options.k ? 0.0 : -1.0);
	scores[0] = 1.0;
	return scores;
}

Array<double> Phoneme::averageScores(const MFCC& mfcc) const {
	Array<double> scores(registered.size());
	for (size_t id : step(registered.size())) {
		scores[id] = options.distance == PhonemeDistance::Cosine ? mfcc.cosineSimilarity(averageMFCC(id)) : -distance(mfcc, averageMFCC(id));
	}
	return scores;
}

Array<double> Phoneme::nearestNeighborScores(const MFCC& mfcc, bool loud) const {
	Array<std::pair<double, size_t>> distances;
	for (size_t id : Range(loud ? Min(options.silentPhonemes, registered.size() - 1) : 0, registered.size() - 1)) {
		for (const auto& sample : registered[id]) distances.emplace_back(distance(mfcc, sample), id);
	}
	const size_t k = Min(options.k, distances.size());
	ranges::partial_sort(distances, distances.begin() + k);
	Array<double> scores(registered.size(), 0.0);
	for (size_t i : step(k)) scores[distances[i].second] += 1.0 / k;
	return scores;
}

double Phoneme::distance(const MFCC& a, const MFCC& b) const {
	if (options.distance == PhonemeDistance::Cosine) return 1.0 - a.cosineSimilarity(b);
	double sum = 0.0;
	for (size_t i : step(a.feature.size())) sum += Math::Square((a.feature[i] - b.feature[i]) / featureScale[i]);
	return sum;
}

void Phoneme::updateFeatures() {
	registered = registeredSpectra.map([this](const auto& spectra) { return spectra.map([this](const auto& spectrum) { return mfccAnalyzer.cepstrum(spectrum); }); });

	const size_t dimensions = options.mfcc.order;
	featureScale.assign(dimensions, 1.0);
	if (!options.standardize) return;
	Array<double> sum(dimensions, 0.0), squareSum(dimensions, 0.0);
	size_t count = 0;
	for (const auto& mfccs : registered) {
		for (const auto& mfcc : mfccs) {
			for (size_t i : step(dimensions)) {
				sum[i] += mfcc.feature[i];
				squareSum[i] += Math::Square(mfcc.feature[i]);
			}
			++count;
		}
	}
	if (count < 2) return;
	for (size_t i : step(dimensions)) featureScale[i] = Max(Math::Sqrt(squareSum[i] / count - Math::Square(sum[i] / count)), 1e-6);
}
