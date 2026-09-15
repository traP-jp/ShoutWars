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
	constexpr double VoiceHintSize = 30.0;
	const StringView VoiceHint = U"技が出にくいときは、ゆっくり丁寧に叫ぶか、キャリブレーションをやり直してみてね";
	constexpr int32 GuardAction = 4;
	constexpr int32 CooldownCells = 10;

	constexpr double HighlightSeconds = 0.9;
	constexpr double ShakeSeconds = 0.6;
	constexpr double MoveNameSeconds = 1.0;
	constexpr double UnmatchedMarkSeconds = 1.2;
	constexpr double VoiceHintGlowSeconds = 1.5;
	constexpr double VoiceHintBlinkSeconds = 0.3;

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

void CommandFeedback::unmatched(size_t player, bool isSelf) {
	const double now = Scene::Time();
	unmatchedTimes[player] = now;
	if (isSelf) selfUnmatchedTime = now;
}

double CommandFeedback::gaugeShake() const {
	return gaugeShortageTime ? Shake(Scene::Time() - *gaugeShortageTime, 10.0) : 0.0;
}

void CommandFeedback::drawCommandList(const Texture& commandList, const Vec2& pos, double guardCooldown) const {
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

		const bool shaking = targeted && row->blocked && elapsed < ShakeSeconds;
		const bool highlighted = targeted && !row->blocked && elapsed < HighlightSeconds;
		const double strength = highlighted ? 1.0 - EaseInQuad(elapsed / HighlightSeconds) : 0.0;
		const Vec2 rowPos = listPos + Vec2{ shaking ? Shake(elapsed, 9.0) : 0.0, top };
		// 光らせる行は、後ろに黄色い帯を敷いたうえで、同じ行を加算で重ねて明るくする
		if (highlighted) RectF{ rowPos + Vec2{ -8.0, 0.0 }, commandList.width() + 16.0, bottom - top }.rounded(10).draw(ColorF{ 1.0, 0.85, 0.2, 0.45 * strength });
		region.draw(rowPos, shaking ? ColorF{ 0.5 } : ColorF{ 1.0 });
		if (highlighted) {
			const ScopedRenderStates2D additive{ BlendState::Additive };
			region.draw(rowPos, ColorF{ 1.0, strength });
			region.draw(rowPos, ColorF{ 1.0, 0.5 * strength });
		}
		// ガードを壊された後は、ガードの行に、再びガードできるまでのゲージ [###.......] を半透明で重ねる
		if ((action == GuardAction) && (0.0 < guardCooldown)) {
			const RectF rowRect{ rowPos, commandList.width(), bottom - top };
			rowRect.draw(ColorF{ 0.0, 0.45 });
			const double cellWidth = (rowRect.w - 150.0) / CooldownCells;
			const int32 filledCells = static_cast<int32>(Math::Ceil((1.0 - guardCooldown) * CooldownCells));
			for (int32 cell = 0; cell < CooldownCells; ++cell) {
				const RectF cellRect{ rowRect.x + 140.0 + cell * cellWidth, rowRect.centerY() - 10.0, cellWidth - 4.0, 20.0 };
				cellRect.rounded(3).draw((cell < filledCells) ? ColorF{ 1.0, 0.85 } : ColorF{ 1.0, 0.2 });
			}
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

void CommandFeedback::drawUnmatchedMarks(const Array<Vec2>& positions) const {
	const double now = Scene::Time();
	for (const auto& [player, time] : unmatchedTimes) {
		const double elapsed = now - time;
		if (elapsed >= UnmatchedMarkSeconds) continue;
		const double pop = 1.0 + 0.3 * (1.0 - Min(elapsed / 0.15, 1.0));
		const double alpha = Clamp((UnmatchedMarkSeconds - elapsed) / 0.3, 0.0, 1.0);
		const Vec2 center = positions[player] + Vec2{ 0.0, -190.0 - 40.0 * EaseOutCubic(elapsed / UnmatchedMarkSeconds) };
		questionMark.resized(80.0 * pop).drawAt(center, ColorF{ 1.0, alpha });
	}
}

void CommandFeedback::drawVoiceHint(const Vec2& center) const {
	const double elapsed = selfUnmatchedTime ? Scene::Time() - *selfUnmatchedTime : Math::Inf;
	const double blink = 0.5 + 0.5 * Math::Cos(Math::TwoPi * elapsed / VoiceHintBlinkSeconds);
	const double strength = (elapsed < VoiceHintGlowSeconds) ? blink * (1.0 - EaseInQuad(elapsed / VoiceHintGlowSeconds)) : 0.0;
	const auto text = font(VoiceHint);
	// 後ろに黄色い帯を敷いて点滅させる
	if (0.0 < strength) text.regionAt(VoiceHintSize, center).stretched(16.0, 4.0).rounded(10).draw(ColorF{ 1.0, 0.85, 0.2, 0.8 * strength });
	text.drawAt(TextStyle::Outline(0.2, ColorF{ 0.0 }), VoiceHintSize, center, Palette::White);
}
