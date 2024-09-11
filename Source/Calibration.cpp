# include "Calibration.hpp"

using namespace std;

Calibration::Calibration(const InitData& init) : IScene(init) {
	for (uint32 i : step(6)) {
		phonemeRects << RectF{ 140.0 + i * 240.0, 620.0, 60.0, 360.0 };
		phonemeRects << RectF{ 260.0 + i * 240.0, 620.0, 60.0, 360.0 };
	}
	getData().phoneme.start();
}

Calibration::~Calibration() {
	getData().phoneme.stop();
}

void Calibration::update() {
	auto&& phoneme = getData().phoneme;

	// 音声解析
	phoneme.mic.fft(fftResult, FFTSampleLength::SL2K);
	phonemeId = phoneme.estimate(FFTSampleLength::SL2K);

	// 音素登録ボタン
	for (size_t id : step(12)) {
		if (phonemeRects[id].mouseOver()) {
			Cursor::RequestStyle(CursorStyle::Hand);
			if (MouseL.down()) isWaitingToSet = true;
			if (isWaitingToSet && MouseL.pressedDuration() >= 0.4s) {
				isWaitingToSet = false;
				phoneme.setMFCC(id);
			}
		}
	}
	if (MouseL.up()) isWaitingToSet = false;

	// OK ボタン
	if (!phoneme.isMFCCUnset() && okButton.mouseOver()) {
		Cursor::RequestStyle(CursorStyle::Hand);
		if (MouseL.down()) {
			phoneme.save();
			State old_scene = getData().before_scene;
			getData().before_scene = State::Config;
			changeScene(old_scene, 0.5s);
		}
	}

	//通信は継続
	if (getData().before_scene == State::Matching) {
		getData().client->update();
	}
}

void Calibration::draw() const {
	const auto nowUs = Time::GetMicrosec();
	auto&& phoneme = getData().phoneme;

	// 背景
	RectF{ 0, 0, 1920, 1080 }.draw(Palette::Black);

	// スペクトラム
	const size_t hz = 800;
	for (size_t i : step(min(fftResult.buffer.size(), hz))) {
		RectF{
			Arg::bottomLeft(i * 1920.0 / hz, 1080),
			1920.0 / hz, (1 + log10(fftResult.buffer[i] * 2) / 6) * 1080
		}.draw(HSV{ 240 - 0.45 * i, 0.3 });
	}

	// MFCC のグラフ
	RectF{ Arg::topLeft(120, 140), 1680.0, 360.0 }.draw(Palette::Black);
	for (const auto [timeUs, mfcc] : *phoneme.getMFCCHistory()) {
		for (size_t i : step(12)) {
			RectF{
				Arg::topLeft(1800.0 - 1680.0 * (nowUs - timeUs) / (phoneme.mfccHistoryLife / 1.1), 140.0 + i * 30.0),
				1680.0 * 50'000 / (phoneme.mfccHistoryLife / 1.1), 30.0
			}.draw(HSV{ 510.0 - mfcc.feature[i] * 2.0, 0.6 });
		}
	}
	RectF{ Arg::rightCenter(120, 320), 200.0, 366.0 }.draw(Palette::Black);
	RectF{ Arg::leftCenter(1800, 320), 200.0, 366.0 }.draw(Palette::Black);
	RectF{ Arg::topLeft(120, 140), 1680.0, 360.0 }.drawFrame(6, Palette::White);

	// 音素登録
	for (size_t id : step(12)) {
		const auto& rect = phonemeRects[id];
		rect.draw(Palette::Black);
		for (size_t i : step(12)) {
			RectF{
				rect.x, rect.y + i * (rect.h / 12), rect.w, rect.h / 12
			}.draw(HSV{ 510.0 - phoneme.mfccList[id].feature[i] * 2.0, 0.6 });
		}
		if (!rect.mouseOver()) rect.drawFrame(4, Palette::White);
		else if (!MouseL.pressed()) rect.drawFrame(12, Palette::White);
		else if (MouseL.pressedDuration() < 0.5s) rect.drawFrame(12, Palette::Orange);
		else rect.drawFrame(10, Palette::Limegreen);
		font(phonemeNames[id]).draw(
			30,
			Arg::topCenter(rect.bottomCenter() + Vec2{ 0, 20 }),
			id == phonemeId ? Palette::Limegreen : Palette::Orange
		);
	}
	font(U"登録したい音素を発音しながら緑に光るまで長押ししてください。").draw(40, 120, 540);

	// 入力感度
	double rootThreshold = sqrt(phoneme.volumeThreshold);
	RectF{ Arg::bottomRight(1700.0, 940.0), 80.0, sqrt(phoneme.mic.rootMeanSquare()) * 360.0 }.draw(Palette::White);
	RectF{ Arg::rightCenter(1700.0, 940.0 - rootThreshold * 360.0), 80.0, 4.0 }.draw(Palette::Skyblue);
	RectF{ Arg::bottomRight(1700.0, 940.0), 80.0, 360.0 }.drawFrame(
		4, phoneme.mic.rootMeanSquare() < phoneme.volumeThreshold ? Palette::Orange : Palette::Lime
	);
	SimpleGUI::VerticalSlider(rootThreshold, 0.0, 1.0, Vec2{ 1740.0, 565.0 }, 390.0);
	phoneme.volumeThreshold = pow(rootThreshold, 2.0);
	font(U"入力感度").draw(40, 1620, 980);

	// タイトル
	font(U"キャリブレーション").draw(60, 40, 30);

	// OK ボタン
	if (phoneme.isMFCCUnset()) {
		okButton.draw(Palette::Forestgreen);
		font(U"未登録の音素\nがあります").drawAt(25, okButton.center(), Palette::White);
	}
	else {
		okButton.draw(Palette::Limegreen);
		font(U"OK").drawAt(50, okButton.center(), Palette::Black);
	}
	if (okButton.mouseOver()) okButton.drawFrame(4, Palette::White);
}
