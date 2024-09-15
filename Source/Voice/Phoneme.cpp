# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(FilePathView configPath, double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife)
	: configPath(configPath), volumeThreshold(defaultVolumeThreshold), mfccList(n), mfccHistoryLife(mfccHistoryLife) {
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
	if (!mic.isRecording()) return false;
	mfccAnalyzer = make_unique<MFCCAnalyzer>(mic, mfccHistoryLife, mfccOrder);
	return true;
}

void Phoneme::stop() {
	mic.stop();
	mfccAnalyzer.reset();
}

Array<double> Phoneme::estimate(FFTSampleLength frames) {
	const size_t n = mfccList.size();
	Array<double> silenceScores(n, -1.0);
	silenceScores[0] = 1.0;
	if (!mic.isRecording()) return silenceScores;

	// analyze MFCC
	const auto& currentMFCC = mfccAnalyzer->analyze(frames, 40);
	if (mic.rootMeanSquare() < volumeThreshold) return silenceScores;

	// estimate phoneme
	return mfccList.map([&](MFCC mfcc) -> double { return currentMFCC.cosineSimilarity(mfcc); });
}

bool Phoneme::isMFCCUnset() const {
	return ranges::any_of(mfccList, [](MFCC mfcc) { return mfcc.isUnset(); });
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
