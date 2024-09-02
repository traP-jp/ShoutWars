# include "Calibration.hpp"

using namespace std;

Calibration::Calibration(const InitData& init) : IScene(init) {
	if (System::EnumerateMicrophones().isEmpty()) throw Error{ U"No microphone is connected" };
	mic = Microphone{ StartImmediately::Yes };
	if (not mic.isRecording()) throw Error{ U"Failed to start recording" };
	formantAnalyzer = make_unique<FormantAnalyzer>(mic);
}

void Calibration::update() {
	ClearPrint();
	Print << U"bufferLength={}\nsampleRate={}\nposSample={}"_fmt(mic.getBufferLength(), mic.getSampleRate(), mic.posSample());
	Print << formantAnalyzer->analyze();
}

void Calibration::draw() const {
	const auto& lpcResult = formantAnalyzer->getLpcResult();
	for (size_t i : step(lpcResult.size())) {
		RectF{
			Arg::bottomLeft(i * 1920.0 / lpcResult.size(), 1080),
			1920.0 / lpcResult.size(), lpcResult[i] * 1080
		}.draw(HSV{ i });
	}
}

void Calibration::drawFadeIn(double t) const {
	// TODO
}

void Calibration::drawFadeOut(double t) const {
	// TODO
}
