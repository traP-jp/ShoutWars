# pragma once
# include <Siv3D.hpp>
# include "GraphicsQuality.hpp"

/// @brief 背景とキャラに、画面全体の仕上げ (ビネット・色調整・フィルムグレイン。品質が高ければブルームも) をかける
class PostProcess {
public:
	[[nodiscard]] explicit PostProcess(GraphicsQuality quality);

	/// @brief drawScene で描いたものに仕上げをかけて、今の描画先に描く
	/// @remark 仕上げの途中の描画には座標変換をかけず、最後に画面へ描くときだけ今の座標変換 (画面の揺れなど) をかける
	template <class DrawScene>
	void draw(DrawScene&& drawScene) const {
		{
			const ScopedRenderTarget2D target{ scene.clear(ColorF{ 0.0 }) };
			const Transformer2D untransformed{ Mat3x2::Identity(), Transformer2D::Target::SetLocal };
			drawScene();
		}
		Graphics2D::Flush();
		scene.resolve();
		finish();
	}

private:
	struct Constants {
		float grainSeed = 0.0f;
		float _unused[3] = {};
	};

	/// @brief 明るいところだけを取り出してぼかしたもの
	struct Bloom {
		PixelShader brightPass;
		PixelShader finish;
		RenderTexture nearGlow;
		RenderTexture nearGlowInternal;
		RenderTexture wideGlow;
		RenderTexture wideGlowInternal;
	};

	MSRenderTexture scene{ Scene::Size() };
	PixelShader finishShader;
	Optional<Bloom> bloom;
	mutable ConstantBuffer<Constants> constants;

	void finish() const;
};
