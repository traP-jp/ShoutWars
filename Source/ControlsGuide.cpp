# include "ControlsGuide.hpp"

namespace {
	constexpr double TitleSize = 24.0;
	constexpr double LabelSize = 28.0;
	constexpr double KeySize = 22.0;
	constexpr double KeyHeight = 40.0;
	constexpr double IconSize = 40.0;
	constexpr double RowHeight = 54.0;
	constexpr double Gap = 12.0;
	constexpr double Padding = 14.0;
	const StringView Title = U"キーボード・コントローラーで動く";
	const StringView Slash = U"/";

	enum class ControllerIcon {
		DirectionalPad,
		FaceButtons,
	};

	struct Row {
		StringView label;
		Array<StringView> keys;
		ControllerIcon icon;
	};

	const Array<Row> Rows = {
		{ U"移動", { U"←", U"→" }, ControllerIcon::DirectionalPad },
		{ U"ジャンプ", { U"↑", U"Space" }, ControllerIcon::FaceButtons },
	};

	[[nodiscard]] double KeyWidth(const Font& font, StringView key) {
		return Max(KeyHeight, font(key).region(KeySize).w + 20.0);
	}

	[[nodiscard]] double KeysWidth(const Font& font, const Array<StringView>& keys) {
		double width = 0.0;
		for (const auto& key : keys) width += KeyWidth(font, key);
		return width + Gap / 2 * (keys.size() - 1);
	}

	void DrawIcon(ControllerIcon icon, const Vec2& center) {
		if (icon == ControllerIcon::DirectionalPad) {
			RectF{ Arg::center(center), IconSize * 0.32, IconSize }.rounded(4).draw(Palette::White);
			RectF{ Arg::center(center), IconSize, IconSize * 0.32 }.rounded(4).draw(Palette::White);
			return;
		}
		for (const Vec2& direction : { Vec2{ 0, -1 }, Vec2{ 1, 0 }, Vec2{ 0, 1 }, Vec2{ -1, 0 } }) {
			Circle{ center + direction * IconSize * 0.34, IconSize * 0.15 }.draw(Palette::White);
		}
	}
}

void ControlsGuide::draw(const Vec2& topRight) const {
	const auto outline = TextStyle::Outline(0.2, ColorF{ 0.0 });
	double labelWidth = 0.0, keysWidth = 0.0;
	for (const auto& row : Rows) {
		labelWidth = Max(labelWidth, font(row.label).region(LabelSize).w);
		keysWidth = Max(keysWidth, KeysWidth(font, row.keys));
	}
	const double slashWidth = font(Slash).region(LabelSize).w;
	const double contentWidth = Max(font(Title).region(TitleSize).w, labelWidth + Gap + keysWidth + Gap + slashWidth + Gap + IconSize);
	const RectF panel{ topRight.x - contentWidth - Padding * 2, topRight.y, contentWidth + Padding * 2, Padding * 2 + TitleSize + Gap + RowHeight * Rows.size() };

	font(Title).draw(outline, TitleSize, Vec2{ panel.x + Padding, panel.y + Padding }, Palette::White);
	const double iconX = panel.rightX() - Padding - IconSize / 2;
	const double keysLeft = iconX - IconSize / 2 - Gap - slashWidth - Gap - keysWidth;
	for (auto&& [i, row] : Indexed(Rows)) {
		const double y = panel.y + Padding + TitleSize + Gap + RowHeight * (i + 0.5);
		font(row.label).draw(outline, LabelSize, Arg::rightCenter(keysLeft - Gap, y), Palette::White);
		double x = keysLeft;
		for (const auto& key : row.keys) {
			const RectF keyRect{ x, y - KeyHeight / 2, KeyWidth(font, key), KeyHeight };
			keyRect.rounded(6).draw(ColorF{ 0.15, 0.9 }).drawFrame(2, Palette::White);
			font(key).drawAt(KeySize, keyRect.center(), Palette::White);
			x = keyRect.rightX() + Gap / 2;
		}
		font(Slash).drawAt(outline, LabelSize, Vec2{ iconX - IconSize / 2 - Gap - slashWidth / 2, y }, ColorF{ 0.8 });
		DrawIcon(row.icon, Vec2{ iconX, y });
	}
}
