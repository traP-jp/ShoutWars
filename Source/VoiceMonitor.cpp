# include "VoiceMonitor.hpp"
# include "Voice/VoiceVisualization.hpp"

namespace {
	constexpr size_t BarCount = 120;
}

VoiceMonitor::VoiceMonitor(const RectF& area) : area(area) {}

void VoiceMonitor::update(const Phoneme& phoneme) {
	threshold = phoneme.volumeThreshold;
	const auto& history = phoneme.getMFCCHistory();
	const bool voiced = !history.empty() && phoneme.rootMeanSquare() >= threshold;
	const Optional<HSV> color = voiced ? Optional<HSV>{ VowelColor(history.rbegin()->second) } : none;
	bars << Bar{ VolumeDb(phoneme.rootMeanSquare()), color };
	if (bars.size() > BarCount) bars.pop_front();
}

void VoiceMonitor::draw() const {
	area.rounded(8).draw(ColorF{ 0.0, 0.5 });
	const double barWidth = area.w / BarCount;
	for (auto&& [i, bar] : Indexed(bars)) {
		const double x = area.x + (BarCount - bars.size() + i) * barWidth;
		const ColorF color = bar.color ? VowelDisplayColor(*bar.color) : ColorF{ 0.6, 0.5 };
		RectF{ Arg::bottomLeft(x, area.bottomY()), barWidth, area.h * VolumeLevel(bar.volumeDb) }.draw(color);
	}
	const double thresholdY = area.bottomY() - area.h * VolumeLevel(VolumeDb(threshold));
	Line{ area.x, thresholdY, area.rightX(), thresholdY }.draw(LineStyle::SquareDot, 2, Palette::Skyblue);
}
