# pragma once
# include "common.hpp"

class GlowBorder {
private:
	PixelShader outlineShader;
	RenderTexture outline;
	Texture texture;
public:
	void init(const Texture& texture);
	void draw(const Vec2& pos, const ColorF& color = Palette::White) const;
	void drawAt(const Vec2& pos, const ColorF& color = Palette::White) const;
};
