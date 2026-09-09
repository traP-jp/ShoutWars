# pragma once
# include "common.hpp"

class GlowBorder {
private:
	PixelShader outlineShader;
	RenderTexture outline;
	Texture texture;
public:
	void init(const Texture& texture);
	void draw(bool drawBorder, const Vec2& pos, const ColorF& color = Palette::White, const double scale = 1.0) const;
	void drawAt(bool drawBorder, const Vec2& pos, const ColorF& color = Palette::White, const double scale = 1.0) const;
};
