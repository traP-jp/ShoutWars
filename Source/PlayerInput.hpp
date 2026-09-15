# pragma once

# include <Siv3D.hpp>

/// @brief 押されている方向
struct Directions {
	bool up = false;
	bool left = false;
	bool down = false;
	bool right = false;
};

/// @brief キーボード (矢印キーと WASD) とコントローラー (十字キーと左スティック) の、押されている方向
/// @remark コントローラーは 1 台目だけを見る。XInput 対応のものと、それ以外のゲームパッドの両方に対応する
[[nodiscard]] Directions PressedDirections();

/// @brief コントローラーの右側の 4 つのボタンのどれかが押されているか (決定やジャンプに使う)
/// @remark 機種によって A ボタンの位置や番号が違うので、どれを押してもよいことにする。XInput では A B X Y、それ以外のゲームパッドでは 1〜4 番目のボタン
[[nodiscard]] bool ControllerFaceButtonPressed();
