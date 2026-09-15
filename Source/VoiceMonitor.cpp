# include "VoiceMonitor.hpp"
# include "Voice/CommandRecognizer.hpp"
# include "Voice/VoiceVisualization.hpp"
# include <ranges>

namespace {
	constexpr size_t BarCount = 120;
	constexpr size_t LoggedVowelCount = 16;
	constexpr double LogHeight = 44.0;
	// 同じ母音がこのフレーム数続いたら、聞き取った母音としてログに出す (単語判定の母音の最小の長さに合わせる)
	constexpr size_t MinVowelFrames = CommandRecognizerOptions{}.minVowelFrames;
	// 無音がこのフレーム数続いたら、発話の区切りとしてログに隙間を空ける
	constexpr size_t GapFrames = 12;
	constexpr double FadeStartSeconds = 2.0;
	constexpr double FadeSeconds = 1.5;
	const StringView VowelTexts = U"あいうえお";
}

VoiceMonitor::VoiceMonitor(const RectF& area) : area(area) {}

void VoiceMonitor::update(const Phoneme& phoneme, const Array<double>& phonemeScores) {
	threshold = phoneme.volumeThreshold;
	const auto& history = phoneme.getMFCCHistory();
	const bool voiced = !history.empty() && phoneme.rootMeanSquare() >= threshold;
	const Optional<HSV> color = voiced ? Optional<HSV>{ VowelColor(history.rbegin()->second) } : none;
	bars << Bar{ VolumeDb(phoneme.rootMeanSquare()), color };
	if (bars.size() > BarCount) bars.pop_front();

	Optional<size_t> vowel;
	if (voiced && !phoneme.isMFCCUnset()) vowel = VowelOfPhoneme(std::ranges::max_element(phonemeScores) - phonemeScores.begin());
	if (!vowel) {
		runVowel.reset();
		runFrames = 0;
		if (++silentFrames == GapFrames && !vowels.isEmpty() && vowels.back().vowel) vowels << LoggedVowel{ none, HSV{}, Scene::Time() };
	}
	else {
		silentFrames = 0;
		runFrames = (vowel == runVowel) ? runFrames + 1 : 1;
		runVowel = vowel;
		if (runFrames == MinVowelFrames) vowels << LoggedVowel{ vowel, *color, Scene::Time() };
	}
	if (vowels.size() > LoggedVowelCount) vowels.pop_front();
}

void VoiceMonitor::draw() const {
	const RectF graph{ area.x, area.y + LogHeight, area.w, area.h - LogHeight };
	graph.rounded(8).draw(ColorF{ 0.0, 0.5 });
	const double barWidth = graph.w / BarCount;
	for (auto&& [i, bar] : Indexed(bars)) {
		const double x = graph.x + (BarCount - bars.size() + i) * barWidth;
		const ColorF color = bar.color ? VowelDisplayColor(*bar.color) : ColorF{ 0.6, 0.5 };
		RectF{ Arg::bottomLeft(x, graph.bottomY()), barWidth, graph.h * VolumeLevel(bar.volumeDb) }.draw(color);
	}
	const double thresholdY = graph.bottomY() - graph.h * VolumeLevel(VolumeDb(threshold));
	Line{ graph.x, thresholdY, graph.rightX(), thresholdY }.draw(LineStyle::SquareDot, 2, Palette::Skyblue);

	// 新しい母音ほど右に並べ、古いものは薄くして消す
	const double now = Scene::Time();
	double x = area.rightX() - LogHeight / 2;
	for (const auto& logged : vowels | std::views::reverse) {
		if (!logged.vowel) {
			x -= LogHeight / 2;
			continue;
		}
		const double alpha = Clamp(1.0 - (now - logged.time - FadeStartSeconds) / FadeSeconds, 0.0, 1.0);
		if (alpha <= 0.0 || x < area.x) break;
		font(VowelTexts[*logged.vowel]).drawAt(TextStyle::Outline(0.2, ColorF{ 0.0, alpha }), LogHeight * 0.8, Vec2{ x, area.y + LogHeight / 2 }, ColorF{ VowelDisplayColor(logged.color), alpha });
		x -= LogHeight;
	}
}
