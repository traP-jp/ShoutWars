# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife)
	: configPath(configPath), volumeThreshold(defaultVolumeThreshold), mfccList(n), mfccHistoryLife(mfccHistoryLife) {
	for (auto&& mfcc : mfccList) mfcc.feature = Array<float>(mfccOrder, 0.0f);
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
						if (mfcc[i].isNumber()) mfccList[id].feature[i] = mfcc[i].get<float>();
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
	return ranges::max_element(mfccList, {}, [&](const auto& targetMFCC) {
		return currentMFCC.cosineSimilarity(targetMFCC);
	}) - mfccList.begin();
}

bool Phoneme::isMFCCUnset() const {
	return ranges::any_of(mfccList, [](MFCC mfcc) { return mfcc.isUnset();  });
}

void Phoneme::setMFCC(uint64 id) {
	if (getMFCCHistory()->empty()) throw Error{ U"MFCC history is empty" };
	mfccList[id] = (*mfccAnalyzer->getMFCCHistory()->rbegin()).second;
}

bool Phoneme::save() const {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	config[U"volumeThreshold"] = volumeThreshold;
	for (size_t id : step(mfccList.size())) config[U"mfcc"][id] = mfccList[id].feature;
	return config.save(configPath);
}

shared_ptr<map<uint64, MFCC>> Phoneme::getMFCCHistory() const {
	if (!mfccAnalyzer) throw Error{ U"mic is not recording" };
	return mfccAnalyzer->getMFCCHistory();
}
