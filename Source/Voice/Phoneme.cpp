# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife)
	: configPath(configPath), volumeThreshold(defaultVolumeThreshold), mfccList(n), mfccHistoryLife(mfccHistoryLife),
	mfccAnalyzer(mfccOrder) {
	if (n < 1) throw Error{ U"The number of phoneme must not be empty" };
	for (auto&& mfcc : mfccList) mfcc.feature = Array<double>(mfccOrder, 0.0);
	JSON config = JSON::Load(configPath);
	try {
		if (config && config.isObject()) {
			if (config[U"volumeThreshold"].isNumber()) {
				volumeThreshold = config[U"volumeThreshold"].get<double>();
			}
			if (config[U"mfcc"].isArray()) {
				for (size_t id : step(n)) {
					const auto mfcc = config[U"mfcc"][id];
					if (!mfcc.isArray() || mfcc.size() != mfccOrder) continue;
					for (size_t i : step(mfccOrder)) {
						if (mfcc[i].isNumber()) mfccList[id].feature[i] = mfcc[i].get<double>();
					}
				}
			}
		}
	}
	catch (...) {
		// 設定ファイルの読み込みに失敗した際は無いものとして扱うため、握りつぶす
	}
}

bool Phoneme::start() {
	mic = Microphone{ StartImmediately::Yes };
	mfccHistory.clear();
	return mic.isRecording();
}

void Phoneme::stop() {
	mic.stop();
}

Array<double> Phoneme::estimate(FFTSampleLength frames) {
	const size_t n = mfccList.size();
	if (!mic.isRecording()) {
		Array<double> silenceScores(n, -1.0);
		silenceScores[0] = 1.0;
		return silenceScores;
	}
	return estimate(latestSamples(frames), mic.getSampleRate(), mic.rootMeanSquare(), Time::GetMicrosec());
}

Array<double> Phoneme::estimate(Array<float> samples, uint32 sampleRate, double rootMeanSquare, uint64 timeUs) {
	const size_t n = mfccList.size();

	const auto currentMFCC = mfccAnalyzer.analyze(std::move(samples), sampleRate, 40);
	erase_if(mfccHistory, [this, timeUs](const auto& p) { return timeUs - p.first > mfccHistoryLife; });
	mfccHistory[timeUs] = currentMFCC;

	if (rootMeanSquare < volumeThreshold) {
		Array<double> silenceScores(n, -1.0);
		silenceScores[0] = 1.0;
		return silenceScores;
	}
	return mfccList.map([&](const MFCC& mfcc) -> double { return currentMFCC.cosineSimilarity(mfcc); });
}

bool Phoneme::isMFCCUnset() const {
	return ranges::any_of(mfccList, [](const MFCC& mfcc) { return mfcc.isUnset(); });
}

void Phoneme::setMFCC(uint64 id, uint64 timeUs) {
	if (mfccHistory.empty()) throw Error{ U"MFCC history is empty" };
	mfccList[id] = MFCC{ Array<double>(mfccOrder, 0.0) };
	size_t count = 0;
	for (const auto& [historyUs, mfcc] : mfccHistory) {
		if (historyUs + 500'000 >= timeUs) {
			for (size_t i : step(mfccOrder)) mfccList[id].feature[i] += mfcc.feature[i];
			++count;
		}
	}
	for (size_t i : step(mfccOrder)) mfccList[id].feature[i] /= count;
}

bool Phoneme::save() const {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	config[U"volumeThreshold"] = volumeThreshold;
	for (size_t id : step(mfccList.size())) config[U"mfcc"][id] = mfccList[id].feature;
	return config.save(configPath);
}

const map<uint64, MFCC>& Phoneme::getMFCCHistory() const {
	return mfccHistory;
}

Array<float> Phoneme::latestSamples(FFTSampleLength frames) const {
	Array<float> f(256uLL << FromEnum(frames), 0.0f);
	const auto& buffer = mic.getBuffer();
	const size_t writePos = mic.posSample();
	for (size_t pos : step(f.size())) {
		const size_t idx = (pos + writePos < f.size() ? mic.getBufferLength() : 0) + pos + writePos - f.size();
		f[pos] = buffer[idx].left; // NOTE: Use only one side!
	}
	return f;
}
