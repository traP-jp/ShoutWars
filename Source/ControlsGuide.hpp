# pragma once

# include <Siv3D.hpp>

/// @brief ゲーム画面の、移動とジャンプの操作説明。キーとコントローラーのボタンを図形で描く
class ControlsGuide {
public:
	/// @param topRight 説明の右上の位置
	void draw(const Vec2& topRight) const;

private:
	Font font{ FontMethod::MSDF, 48, Typeface::Heavy };
};
