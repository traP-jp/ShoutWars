# include "GlowBorder.hpp"

void GlowBorder::init(const Texture& from_texture)
{
	outlineShader = HLSL{ U"Shader/outline.hlsl", U"PS" };
	if (!outlineShader)
		throw Error{ U"Failed to load outline shader" };

	texture = from_texture;

	const Size padding{ 40, 40 };
	const Size size = texture.size() + padding;

	RenderTexture source{ size };
	outline = RenderTexture{ size };

	{
		const ScopedRenderTarget2D target{ source.clear(ColorF{ 0.0, 0.0 }) };
		// Siv3DのデフォルトBlendStateはアルファ書き込みがOFF(Zero, One)のため、
		// レンダーターゲットにアルファ値を正しく書き込むBlendStateを指定
		const ScopedRenderStates2D blend{ BlendState{ true, Blend::SrcAlpha, Blend::InvSrcAlpha, BlendOp::Add, Blend::One, Blend::InvSrcAlpha, BlendOp::Add } };
		texture.drawAt(size / 2);
		Graphics2D::Flush();
	}

	{
		const ScopedRenderTarget2D target{ outline.clear(ColorF{ 0.0, 0.0 }) };
		const ScopedRenderStates2D blend{ BlendState::Opaque };
		const ScopedCustomShader2D shader{ outlineShader };

		source.draw(ColorF{ 1.0 });
		Graphics2D::Flush();
	}

}

void GlowBorder::draw(bool drawBorder, const Vec2& pos, const ColorF& color, const double scale) const
{
	if (drawBorder) {
		const ScopedRenderStates2D blend{ BlendState::Additive };
		outline.scaled(scale).draw(pos - Vec2{ 20, 20 }, color);
	}
	texture.scaled(scale).draw(pos);
}

void GlowBorder::drawAt(bool drawBorder, const Vec2& pos, const ColorF& color, const double scale) const
{
	if (drawBorder) {
		const ScopedRenderStates2D blend{ BlendState::Additive };
		outline.scaled(scale).drawAt(pos, color);
	}
	texture.scaled(scale).drawAt(pos);
}
