# include "Calibration.hpp"

using namespace std;

Calibration::Calibration(const InitData& init) : IScene(init) {
	if (System::EnumerateMicrophones().isEmpty()) throw Error{ U"No microphone is connected" };
	mic = Microphone{ StartImmediately::Yes };
	if (not mic.isRecording()) throw Error{ U"Failed to start recording" };
	mfccAnalyzer = make_unique<MFCCAnalyzer>(mic);
}

void Calibration::update() {
	ClearPrint();
	Print << U"bufferLength={}\nsampleRate={}\nposSample={}"_fmt(mic.getBufferLength(), mic.getSampleRate(), mic.posSample());
	mic.fft(fftResult, FFTSampleLength::SL2K);
	mfcc = mfccAnalyzer->analyze(FFTSampleLength::SL2K);
	Print << U"MFCC:";
	for (size_t i : step(mfcc.size())) Print << U"  [{:2}]={:.4f}"_fmt(i, mfcc[i]);
}

void Calibration::draw() const {
	const size_t hz = 800;
	for (size_t i : step(hz)) {
		RectF{
			Arg::bottomLeft(i * 1920.0 / hz, 1080),
			1920.0 / hz, (1 + log10(fftResult.buffer[i] * 2) / 6) * 1080
		}.draw(HSV{ 240 - 0.5 * i });
	}
	RectF{ Arg::leftCenter(0, 540), 1920, 4 }.draw(Palette::Dimgray);
	LineString points;
	for (size_t i : step(mfcc.size())) points << Vec2{ (i + 0.5) * 1920.0 / mfcc.size(), 540 - mfcc[i] * 3 };
	points.draw(6, Palette::Black);
	points.draw(4, Palette::White);
}

void Calibration::drawFadeIn(double t) const {
	// TODO
}

void Calibration::drawFadeOut(double t) const {
	// TODO
}
