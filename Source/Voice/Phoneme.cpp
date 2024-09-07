# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife)
	: configPath(configPath), volumeThreshold(defaultVolumeThreshold), mfccList(n), mfccHistoryLife(mfccHistoryLife) {
	for (auto&& mfcc : mfccList) mfcc.fill(0.0f);
	JSON config = JSON::Load(configPath);
	if (config && config.isObject()) {
		if (config[U"volumeThreshold"].isNumber()) {
			volumeThreshold = config[U"volumeThreshold"].get<double>();
		}
		if (config[U"mfcc"].isArray()) {
			for (size_t id : step(n)) {
				const auto mfcc = config[U"mfcc"][id];
				if (!mfcc.isArray()) continue;
				for (size_t i : step(mfccOrder)) {
					if (mfcc[i].isNumber()) mfccList[id][i] = mfcc[i].get<float>();
				}
			}
		}
	}
}

bool Phoneme::start() {
	mic = Microphone{ StartImmediately::Yes };
	if (!mic.isRecording()) return false;
	mfccAnalyzer = make_unique<MFCCAnalyzer>(mic, mfccHistoryLife, mfccOrder);
	return true;
}

void Phoneme::stop() {
	mic.stop();
	mfccAnalyzer.reset();
}

size_t Phoneme::estimate(FFTSampleLength frames) {
	if (!mic.isRecording()) return 0;

	// analyze MFCC
	const auto& currentMFCC = mfccAnalyzer->analyze(frames, 40);
	if (mic.rootMeanSquare() < volumeThreshold) return 0;

	// estimate phoneme
	const double currentNorm = sqrt(accumulate(
		currentMFCC.begin(), currentMFCC.end(), 0.0, [](const auto& norm, const auto& x) { return norm + x * x; }
	));
	if (currentNorm < 1e-8) return 0;
	return ranges::max_element(mfccList, {}, [&](const auto& targetMFCC) {
		// cosine similarity
		const double targetNorm = sqrt(accumulate(
			targetMFCC.begin(), targetMFCC.end(), 0.0, [](const auto& norm, const auto& x) { return norm + x * x; }
		));
		if (targetNorm < 1e-8) return 0.0;
		double innerProduct = 0.0;
		for (size_t i : step(mfccOrder)) innerProduct += currentMFCC[i] * targetMFCC[i];
		return innerProduct / currentNorm / targetNorm;
	}) - mfccList.begin();
}

bool Phoneme::isMFCCUnset() const {
	for (const auto& mfcc : mfccList) {
		if (ranges::all_of(mfcc, [](float x) { return x == 0.0f; })) return true;
	}
	return false;
}

void Phoneme::setMFCC(uint64 id) {
	if (getMFCCHistory()->empty()) throw Error{ U"MFCC history is empty" };
	const auto& mfcc = (*mfccAnalyzer->getMFCCHistory()->rbegin()).second;
	copy(mfcc.begin(), mfcc.end(), mfccList[id].begin());
}

bool Phoneme::save() const {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	config[U"volumeThreshold"] = volumeThreshold;
	for (size_t id : step(mfccList.size())) {
		config[U"mfcc"][id] = Array(mfccList[id].begin(), mfccList[id].end());
	}
	return config.save(configPath);
}

shared_ptr<map<uint64, Array<float>>> Phoneme::getMFCCHistory() const {
	if (!mfccAnalyzer) throw Error{ U"mic is not recording" };
	return mfccAnalyzer->getMFCCHistory();
}
