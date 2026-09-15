# include "CommandFeedback.hpp"
# include "Voice/CommandRecognizer.hpp"

namespace {
	// コマンド一覧の画像の、1 行目の見出しの上端と行の間隔 (画像から測った値)。
	// 行の境目は、ふりがなの上端と上の行の影の下端の間に置く
	constexpr double FirstRowTop = 22.0;
	constexpr double RowPitch = 48.8;
	constexpr double RowBoundaryOffset = -19.4;

	constexpr double HintSize = 30.0;
	constexpr double HintGap = 12.0;
	const StringView Hint = U"マイクに向かって叫んで発動！";

	constexpr double HighlightSeconds = 0.9;
	constexpr double ShakeSeconds = 0.6;
	constexpr double MoveNameSeconds = 1.0;

	[[nodiscard]] double Shake(double elapsed, double amplitude) {
		if (elapsed >= ShakeSeconds) return 0.0;
		return Math::Sin(elapsed * 55.0) * amplitude * (1.0 - elapsed / ShakeSeconds);
	}
}

void CommandFeedback::started(size_t player, int32 action, int32 character, bool isSelf) {
	const double now = Scene::Time();
	if (isSelf) row = RowFeedback{ action, false, now };
	moveNames.remove_if([now](const MoveName& name) { return now - name.time >= MoveNameSeconds; });
	for (const auto& command : VoiceCommandsOf(character)) {
		if (command.action != action) continue;
		moveNames << MoveName{ player, U"「{}」"_fmt(command.text), now };
		break;
	}
}

void CommandFeedback::blocked(int32 action, bool gaugeShortage) {
	row = RowFeedback{ action, true, Scene::Time() };
	if (gaugeShortage) gaugeShortageTime = Scene::Time();
}

double CommandFeedback::gaugeShake() const {
	return gaugeShortageTime ? Shake(Scene::Time() - *gaugeShortageTime, 10.0) : 0.0;
}

void CommandFeedback::drawCommandList(const Texture& commandList, const Vec2& pos) const {
	font(Hint).draw(TextStyle::Outline(0.2, ColorF{ 0.0 }), HintSize, pos, Palette::White);

	const Vec2 listPos = pos + Vec2{ 0.0, HintSize + HintGap };
	const double now = Scene::Time();
	const int32 rows = static_cast<int32>(Math::Round((commandList.height() - (FirstRowTop + RowBoundaryOffset)) / RowPitch));
	for (int32 action = 1; action <= rows; ++action) {
		const double top = (action == 1) ? 0.0 : Math::Round(FirstRowTop + RowBoundaryOffset + RowPitch * (action - 1));
		const double bottom = (action == rows) ? commandList.height() : Math::Round(FirstRowTop + RowBoundaryOffset + RowPitch * action);
		const auto region = commandList(0, top, commandList.width(), bottom - top);
		const bool targeted = row && (row->action == action);
		const double elapsed = targeted ? now - row->time : Math::Inf;

		if (targeted && row->blocked && elapsed < ShakeSeconds) {
			region.draw(listPos + Vec2{ Shake(elapsed, 9.0), top }, ColorF{ 0.5 });
			continue;
		}
		const bool highlighted = targeted && !row->blocked && elapsed < HighlightSeconds;
		const double strength = highlighted ? 1.0 - EaseInQuad(elapsed / HighlightSeconds) : 0.0;
		// 光らせる行は、後ろに黄色い帯を敷いたうえで、同じ行を加算で重ねて明るくする
		if (highlighted) RectF{ listPos + Vec2{ -8.0, top }, commandList.width() + 16.0, bottom - top }.rounded(10).draw(ColorF{ 1.0, 0.85, 0.2, 0.45 * strength });
		region.draw(listPos + Vec2{ 0.0, top });
		if (highlighted) {
			const ScopedRenderStates2D additive{ BlendState::Additive };
			region.draw(listPos + Vec2{ 0.0, top }, ColorF{ 1.0, strength });
			region.draw(listPos + Vec2{ 0.0, top }, ColorF{ 1.0, 0.5 * strength });
		}
	}
}

void CommandFeedback::drawMoveNames(const Array<Vec2>& positions) const {
	const double now = Scene::Time();
	for (const auto& name : moveNames) {
		const double elapsed = now - name.time;
		if (elapsed >= MoveNameSeconds) continue;
		const double pop = 1.0 + 0.2 * (1.0 - Min(elapsed / 0.12, 1.0));
		const double alpha = 0.7 * Clamp((MoveNameSeconds - elapsed) / 0.25, 0.0, 1.0);
		const Vec2 center = positions[name.player] + Vec2{ 0.0, -230.0 - 30.0 * EaseOutCubic(elapsed / MoveNameSeconds) };
		font(name.text).drawAt(TextStyle::Outline(0.25, ColorF{ 1.0, alpha }), 38.0 * pop, center, ColorF{ 0.85, 0.0, 0.0, alpha });
	}
}
