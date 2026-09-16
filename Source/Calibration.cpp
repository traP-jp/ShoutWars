# include "Calibration.hpp"
# include "Voice/VoiceVisualization.hpp"
# include <ranges>

namespace {
	constexpr size_t FramesPerSecond = 60;
	constexpr StringView GraphsKey = U"calibrationGraphs";

	/// @brief 1 つの段階で録る区間の並び。区間ごとに、前後のフレームを捨てて真ん中を 1 つの音素に登録する
	struct StepPlan {
		StringView name;
		StringView voice;
		Array<size_t> phonemeIds;
		Array<StringView> vowels;
		/// @brief 入力感度の閾値を超えたフレームだけを数えるか
		bool voicedOnly;
		size_t leadFrames;
		size_t keptFrames;
		size_t tailFrames;

		[[nodiscard]] size_t sectionFrames() const { return leadFrames + keptFrames + tailFrames; }
		[[nodiscard]] size_t totalFrames() const { return sectionFrames() * phonemeIds.size(); }
	};

	const Array<StringView> Vowels = { U"あ", U"い", U"う", U"え", U"お" };

	// 環境音は無音と鼻息の枠に 1 秒ずつ入れる (鼻息を録らずに無音を 2 枠に入れた評価ツールと同じ条件)。
	// 母音は続けて発音するので、区間の先頭で前の音からの移り変わりを、末尾で次の音への先走りを捨てる。
	// 捨てる部分をグラフで見分けられるようにすると、プレイヤーがそこに合わせて切り替えてしまうので、見た目は区別しない
	const std::array<StepPlan, 3> Plans = { {
		{ U"環境音", U"", { 0, 1 }, {}, false, 0, FramesPerSecond, 0 },
		{ U"低音母音", U"低い声", { 3, 5, 7, 9, 11 }, Vowels, true, FramesPerSecond * 3 / 10, FramesPerSecond, FramesPerSecond * 3 / 10 },
		{ U"高音母音", U"高い声", { 2, 4, 6, 8, 10 }, Vowels, true, FramesPerSecond * 3 / 10, FramesPerSecond, FramesPerSecond * 3 / 10 },
	} };

	constexpr double SectionWidth = 100.0;
	constexpr double CardPadding = 30.0;
	constexpr double CardGap = 20.0;
	constexpr double CardTop = 180.0;
	constexpr double CardHeight = 560.0;
	constexpr double GraphTop = CardTop + 90.0;
	constexpr double GraphHeight = CardHeight - 190.0;

	const RectF ReturnRect{ 20, 20, 80, 80 };
	const RectF ButtonRect{ Arg::center(960, 870), 640, 110 };
	const RectF MeterRect{ 150, GraphTop, 50, GraphHeight };

	const ColorF GraphColor{ 0.16, 0.18, 0.24 };
	const ColorF GraphAlternateColor{ 0.2, 0.22, 0.28 };
	const ColorF SilentColor{ 0.4 };
	const ColorF AmbienceColor{ 0.6, 0.7, 0.8 };

	[[nodiscard]] double CardWidth(size_t index) {
		return Plans[index].phonemeIds.size() * SectionWidth + CardPadding * 2;
	}

	[[nodiscard]] RectF StepRect(size_t index) {
		double x = 420.0;
		for (size_t i : step(index)) x += CardWidth(i) + CardGap;
		return RectF{ x, CardTop, CardWidth(index), CardHeight };
	}

	[[nodiscard]] RectF GraphRect(size_t index) {
		return RectF{ StepRect(index).x + CardPadding, GraphTop, CardWidth(index) - CardPadding * 2, GraphHeight };
	}

	[[nodiscard]] double FromDb(double db) {
		return Math::Pow(10.0, db / 20.0);
	}

	[[nodiscard]] bool IsNumbers(const JSON& values) {
		for (const auto& value : values.arrayView()) {
			if (!value.isNumber()) return false;
		}
		return true;
	}

	void DrawThreshold(const RectF& area, double threshold) {
		const double y = area.bottomY() - area.h * VolumeLevel(VolumeDb(threshold));
		Line{ area.x, y, area.rightX(), y }.draw(LineStyle::SquareDot, 3, Palette::Skyblue);
	}
}

Calibration::Calibration(const InitData& init) : IScene(init), graphs(LoadGraphs(getData().phoneme.configPath)) {
	//登録が消された段階 (展示用モードの起動時など) の古いグラフは出さない
	for (auto&& [plan, graph] : std::views::zip(Plans, graphs)) {
		if (plan.phonemeIds.any([&](size_t id) { return getData().phoneme.isMFCCUnset(id); })) graph.clear();
	}
	returnGlow.init(returnImage);
	getData().phoneme.start();
}

Calibration::~Calibration() {
	getData().phoneme.stop();
}

void Calibration::update() {
	auto& phoneme = getData().phoneme;

	phoneme.mic.fft(fftResult, FFTSampleLength::SL2K);
	phonemeScores = phoneme.estimate(FFTSampleLength::SL2K);
	if (isRecording) recordFrame();

	isReturnHovered = ReturnRect.mouseOver();
	if (isReturnHovered) Cursor::RequestStyle(CursorStyle::Hand);
	if (ReturnRect.leftClicked()) {
		close();
		return;
	}

	for (size_t step : ::step(Plans.size())) {
		if (!StepRect(step).mouseOver()) continue;
		Cursor::RequestStyle(CursorStyle::Hand);
		if (StepRect(step).leftClicked()) {
			selectedStep = step;
			isRecording = false;
		}
	}

	if (!isRecording && ButtonRect.mouseOver()) {
		Cursor::RequestStyle(CursorStyle::Hand);
		if (ButtonRect.leftClicked()) pressButton();
	}

	//通信は継続
	if ((getData().before_scene == State::Matching) && getData().room) {
		getData().room->update();
	}
}

void Calibration::recordFrame() {
	auto& phoneme = getData().phoneme;
	const StepPlan& plan = Plans[*selectedStep];
	const double rootMeanSquare = phoneme.rootMeanSquare();
	if (!phoneme.mic.isRecording() || (plan.voicedOnly && rootMeanSquare < phoneme.volumeThreshold)) return;

	const size_t section = recording.bars.size() / plan.sectionFrames();
	const size_t position = recording.bars.size() % plan.sectionFrames();
	if (plan.leadFrames <= position && position < plan.leadFrames + plan.keptFrames) recording.spectra[section] << phoneme.latestSpectrum();
	recording.bars << Bar{ VolumeDb(rootMeanSquare), phoneme.getMFCCHistory().rbegin()->second };
	if (recording.bars.size() == plan.totalFrames()) finishTake();
}

void Calibration::finishTake() {
	auto& phoneme = getData().phoneme;
	const size_t step = *selectedStep;
	for (auto&& [id, spectra] : std::views::zip(Plans[step].phonemeIds, recording.spectra)) phoneme.setSpectra(id, spectra);
	graphs[step] = std::move(recording.bars);
	isRecording = false;
	selectedStep.reset();
	if (!isGuided) return;
	// 順に進めている途中で前の段階を録り直したときは、止まっていた段階に戻る
	if (step == guidedSteps) ++guidedSteps;
	if (guidedSteps < Plans.size()) selectedStep = guidedSteps;
	else isGuided = false;
}

void Calibration::pressButton() {
	if (!selectedStep) {
		if (isComplete()) {
			close();
			return;
		}
		isGuided = true;
		guidedSteps = 0;
		selectedStep = 0;
		return;
	}
	recording = Take{ .spectra = Array<Array<Array<double>>>(Plans[*selectedStep].phonemeIds.size()) };
	isRecording = true;
}

void Calibration::close() {
	cancelSound.playOneShot();
	getData().phoneme.save();
	SaveGraphs(getData().phoneme.configPath, graphs);
	State old_scene = getData().before_scene;
	getData().before_scene = State::Config;
	changeScene(old_scene, 0.5s);
}

bool Calibration::isComplete() const {
	return !isGuided && !getData().phoneme.isMFCCUnset();
}

StringView Calibration::buttonLabel() const {
	if (!selectedStep) return isComplete() ? U"ゲームに戻る" : U"キャリブレーションにすすむ";
	if (Plans[*selectedStep].voicedOnly) return isRecording ? U"声を出すと進みます" : U"録音をはじめる";
	return isRecording ? U"しゃべらないで" : U"しゃべらずにクリック";
}

String Calibration::guideMessage() const {
	if (!selectedStep) {
		return isComplete() ? U"上のグラフを選択して採り直せます" : U"しゃべっていないときは線より下、声を出すと線を超えるように入力感度を合わせてください";
	}
	const StepPlan& plan = Plans[*selectedStep];
	if (!plan.voicedOnly) {
		return isRecording ? U"基準となる音を録音しています" : U"{} 秒間、基準となる音を録音します"_fmt(plan.totalFrames() / FramesPerSecond);
	}
	if (!isRecording) return U"ボタンを押したら、{}で「あーいーうーえーおー」と続けて伸ばしてください"_fmt(plan.voice);
	return U"{}で「{}ー」と伸ばしてください"_fmt(plan.voice, plan.vowels[recording.bars.size() / plan.sectionFrames()]);
}

String Calibration::recognitionLabel() const {
	const auto& phoneme = getData().phoneme;
	if (phoneme.isMFCCUnset()) return U"認識中: -";
	const size_t id = std::ranges::max_element(phonemeScores) - phonemeScores.begin();
	for (const StepPlan& plan : Plans) {
		const auto it = std::ranges::find(plan.phonemeIds, id);
		if (it != plan.phonemeIds.end()) return plan.vowels.isEmpty() ? U"認識中: 無音" : U"認識中: {}"_fmt(plan.vowels[it - plan.phonemeIds.begin()]);
	}
	throw Error{ U"Phoneme {} is not in any calibration step"_fmt(id) };
}

void Calibration::draw() const {
	//仕上げは背景のスペクトルだけにかけ、UI は読みやすいようその上に描く
	getData().post_process.draw([&] {
		const size_t hz = 800;
		for (size_t i : step(Min(fftResult.buffer.size(), hz))) {
			RectF{
				Arg::bottomLeft(i * 1920.0 / hz, 1080),
				1920.0 / hz, (1 + log10(fftResult.buffer[i] * 2) / 6) * 1080
			}.draw(HSV{ 240 - 0.45 * i, 0.3 });
		}
		RectF{ 0, 0, 1920, 1080 }.draw(ColorF{ 0.0, 0.6 });
	});

	returnGlow.draw(isReturnHovered, ReturnRect.pos);
	font(U"キャリブレーション").draw(60, 120, 25);

	drawSensitivity();
	for (size_t step : ::step(Plans.size())) drawStep(step);

	const bool enabled = !isRecording;
	ButtonRect.rounded(20).draw(enabled ? ColorF{ 0.2, 0.6, 0.3 } : ColorF{ 0.25 });
	if (enabled && ButtonRect.mouseOver()) ButtonRect.rounded(20).drawFrame(4, Palette::White);
	font(buttonLabel()).drawAt(44, ButtonRect.center(), Palette::White);
	font(guideMessage()).drawAt(34, Vec2{ 960, 990 }, Palette::White);
}

void Calibration::drawSensitivity() const {
	auto& phoneme = getData().phoneme;
	font(U"入力感度").drawAt(36, Vec2{ MeterRect.x + 70, CardTop + 45 }, Palette::White);

	const bool voiced = phoneme.rootMeanSquare() >= phoneme.volumeThreshold;
	const auto& history = phoneme.getMFCCHistory();
	const ColorF volumeColor = voiced && !history.empty() ? VowelDisplayColor(VowelColor(history.rbegin()->second)) : SilentColor;
	MeterRect.draw(GraphColor);
	RectF{ Arg::bottomLeft(MeterRect.bl()), MeterRect.w, MeterRect.h * VolumeLevel(VolumeDb(phoneme.rootMeanSquare())) }.draw(volumeColor);
	DrawThreshold(MeterRect, phoneme.volumeThreshold);
	MeterRect.drawFrame(4, voiced ? Palette::Lime : Palette::Orange);

	double level = VolumeLevel(VolumeDb(phoneme.volumeThreshold));
	if (SimpleGUI::VerticalSlider(level, Vec2{ MeterRect.rightX() + 30, MeterRect.y }, MeterRect.h)) {
		phoneme.volumeThreshold = FromDb(MinVolumeDb + level * -MinVolumeDb);
	}

	font(recognitionLabel()).drawAt(34, Vec2{ MeterRect.x + 70, MeterRect.bottomY() + 50 }, voiced ? ColorF{ Palette::Lime } : ColorF{ 0.7 });
}

void Calibration::drawStep(size_t step) const {
	const auto& phoneme = getData().phoneme;
	const StepPlan& plan = Plans[step];
	const bool selected = selectedStep == step;
	const bool recordingHere = selected && isRecording;
	const Array<Bar>& bars = recordingHere ? recording.bars : graphs[step];
	const RectF rect = StepRect(step);
	rect.rounded(16).draw(selected ? ColorF{ 0.12, 0.15, 0.22, 0.95 } : ColorF{ 0.08, 0.85 });
	rect.rounded(16).drawFrame(selected ? 4 : 2, selected ? ColorF{ Palette::Skyblue } : rect.mouseOver() ? ColorF{ Palette::White } : ColorF{ 0.4 });

	const bool registered = plan.phonemeIds.all([&](size_t id) { return !phoneme.isMFCCUnset(id); });
	font(registered ? U"{} ✓"_fmt(plan.name) : String{ plan.name }).drawAt(40, Vec2{ rect.centerX(), rect.y + 45 }, Palette::White);

	const RectF graph = GraphRect(step);
	for (size_t section : ::step(plan.phonemeIds.size())) {
		RectF{ graph.x + section * SectionWidth, graph.y, SectionWidth, graph.h }.draw(section % 2 ? GraphAlternateColor : GraphColor);
	}
	const double barWidth = SectionWidth / plan.sectionFrames();
	for (auto&& [i, bar] : Indexed(bars)) {
		RectF{ Arg::bottomLeft(graph.x + i * barWidth, graph.bottomY()), barWidth, graph.h * VolumeLevel(bar.volumeDb) }.draw(plan.vowels.isEmpty() ? AmbienceColor : VowelDisplayColor(VowelColor(bar.mfcc)));
	}
	if (recordingHere) {
		const double height = graph.h * VolumeLevel(VolumeDb(phoneme.rootMeanSquare()));
		RectF{ Arg::bottomLeft(graph.x + bars.size() * barWidth, graph.bottomY()), Max(barWidth, 4.0), height }.draw(ColorF{ 1.0, 0.4 });
	}
	for (size_t section : Range(1, plan.phonemeIds.size() - 1)) {
		const double x = graph.x + section * SectionWidth;
		Line{ x, graph.y, x, graph.bottomY() }.draw(2, ColorF{ 0.5 });
	}
	DrawThreshold(graph, phoneme.volumeThreshold);
	graph.drawFrame(2, ColorF{ 0.5 });

	const size_t currentSection = bars.size() / plan.sectionFrames();
	for (auto&& [section, vowel] : Indexed(plan.vowels)) {
		const bool current = recordingHere && section == currentSection;
		const ColorF color = current ? ColorF{ Palette::Yellow } : section < currentSection ? ColorF{ Palette::White } : ColorF{ 0.6 };
		font(vowel).drawAt(current ? 48 : 36, Vec2{ graph.x + (section + 0.5) * SectionWidth, graph.bottomY() + 50 }, color);
	}
}

Array<Array<Calibration::Bar>> Calibration::LoadGraphs(FilePathView configPath) {
	Array<Array<Bar>> graphs(Plans.size());
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject() || !config[GraphsKey].isArray()) return graphs;
	JSON saved = config[GraphsKey];
	for (size_t step : ::step(Min(Plans.size(), saved.size()))) {
		if (!saved[step].isArray()) continue;
		Array<Bar> bars;
		for (const auto& bar : saved[step].arrayView()) {
			if (!bar.isArray() || bar.size() != MFCCOptions{}.order + 1 || !IsNumbers(bar)) break;
			bars << Bar{ bar[0].get<double>(), MFCC{ Array<double>(bar.size() - 1, Arg::generator = [&, i = size_t{ 1 }]() mutable { return bar[i++].get<double>(); }) } };
		}
		// 録音の段取りを変えて長さが合わなくなった古いグラフは捨てる
		if (bars.size() == Plans[step].totalFrames()) graphs[step] = std::move(bars);
	}
	return graphs;
}

void Calibration::SaveGraphs(FilePathView configPath, const Array<Array<Bar>>& graphs) {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	config[GraphsKey] = graphs.map([](const Array<Bar>& bars) {
		return JSON(bars.map([](const Bar& bar) {
			Array<JSON> values{ JSON(Math::Round(bar.volumeDb * 10.0) / 10.0) };
			for (double coefficient : bar.mfcc.feature) values << JSON(Math::Round(coefficient * 10.0) / 10.0);
			return JSON(values);
		}));
	});
	config.save(configPath);
}
