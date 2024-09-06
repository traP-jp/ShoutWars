# include "Phoneme.hpp"

using namespace std;

Phoneme::Phoneme(double defaultVolumeThreshold, size_t n, uint64 mfccHistoryLife)
	: volumeThreshold(defaultVolumeThreshold), mfccList(n), mfccHistoryLife(mfccHistoryLife) {
	for (auto&& mfcc : mfccList) mfcc.fill(0);
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

	const auto& currentMFCC = mfccAnalyzer->analyze(frames, 40);
	if (mic.rootMeanSquare() < volumeThreshold) return 0;

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

void Phoneme::setMFCC(uint64 id) {
	if (getMFCCHistory()->empty()) throw Error{ U"MFCC history is empty" };
	const auto &mfcc = (*mfccAnalyzer->getMFCCHistory()->rbegin()).second;
	copy(mfcc.begin(), mfcc.end(), mfccList[id].begin());
}

shared_ptr<map<uint64, Array<float>>> Phoneme::getMFCCHistory() const {
	if (!mfccAnalyzer) throw Error{ U"mic is not recording" };
	return mfccAnalyzer->getMFCCHistory();
}
