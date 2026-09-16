# include "PostProcess.hpp"

namespace {
	[[nodiscard]] PixelShader LoadShader(const String& entryPoint) {
		PixelShader shader = HLSL{ Resource(U"shaders/post_process.hlsl"), entryPoint };
		if (!shader) throw Error{ U"Failed to load the post process shader: {}"_fmt(entryPoint) };
		return shader;
	}
}

PostProcess::PostProcess(GraphicsQuality quality) : finishShader{ LoadShader(U"PS_Finish") } {
	if (quality == GraphicsQuality::Low) return;
	const Size nearSize = Scene::Size() / 2;
	const Size wideSize = Scene::Size() / 4;
	bloom = Bloom{
		.brightPass = LoadShader(U"PS_BrightPass"),
		.finish = LoadShader(U"PS_FinishWithBloom"),
		.nearGlow = RenderTexture{ nearSize },
		.nearGlowInternal = RenderTexture{ nearSize },
		.wideGlow = RenderTexture{ wideSize },
		.wideGlowInternal = RenderTexture{ wideSize },
	};
}

void PostProcess::finish() const {
	constants->grainSeed = static_cast<float>(Scene::FrameCount() % 1024);
	Graphics2D::SetPSConstantBuffer(1, constants);

	if (!bloom) {
		const ScopedCustomShader2D shader{ finishShader };
		scene.draw();
		return;
	}

	{
		// Siv3D のぼかしは座標変換を打ち消さないので、光を作る間は変換をかけない
		const Transformer2D untransformed{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };
		{
			const ScopedRenderTarget2D target{ bloom->nearGlow.clear(ColorF{ 0.0 }) };
			const ScopedCustomShader2D shader{ bloom->brightPass };
			scene.scaled(0.5).draw();
		}
		Graphics2D::Flush();
		// 近くをにじませる光と、広く漂う光を重ねる
		Shader::GaussianBlur(bloom->nearGlow, bloom->nearGlowInternal, bloom->nearGlow);
		Shader::Downsample(bloom->nearGlow, bloom->wideGlow);
		Shader::GaussianBlur(bloom->wideGlow, bloom->wideGlowInternal, bloom->wideGlow);
		Shader::GaussianBlur(bloom->wideGlow, bloom->wideGlowInternal, bloom->wideGlow);
	}

	Graphics2D::SetPSTexture(1, bloom->nearGlow);
	Graphics2D::SetPSTexture(2, bloom->wideGlow);
	{
		const ScopedCustomShader2D shader{ bloom->finish };
		scene.draw();
	}
	Graphics2D::Flush();
	Graphics2D::SetPSTexture(1, none);
	Graphics2D::SetPSTexture(2, none);
}
