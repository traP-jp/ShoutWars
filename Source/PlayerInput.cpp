# include "PlayerInput.hpp"

namespace {
	// スティックはこれ以上倒したときだけ方向として扱い、軽く触れただけで動かないようにする
	constexpr double StickThreshold = 0.5;
}

Directions PressedDirections() {
	Directions directions{
		.up = KeyW.pressed() || KeyUp.pressed(),
		.left = KeyA.pressed() || KeyLeft.pressed(),
		.down = KeyS.pressed() || KeyDown.pressed(),
		.right = KeyD.pressed() || KeyRight.pressed(),
	};
	if (const auto xinput = XInput(0); xinput.isConnected()) {
		directions.up |= xinput.buttonUp.pressed() || (StickThreshold < xinput.leftThumbY);
		directions.left |= xinput.buttonLeft.pressed() || (xinput.leftThumbX < -StickThreshold);
		directions.down |= xinput.buttonDown.pressed() || (xinput.leftThumbY < -StickThreshold);
		directions.right |= xinput.buttonRight.pressed() || (StickThreshold < xinput.leftThumbX);
	}
	if (const auto gamepad = Gamepad(0); gamepad.isConnected()) {
		const auto axis = [&](size_t i) { return (i < gamepad.axes.size()) ? gamepad.axes[i] : 0.0; };
		directions.up |= gamepad.povUp.pressed() || (axis(1) < -StickThreshold);
		directions.left |= gamepad.povLeft.pressed() || (axis(0) < -StickThreshold);
		directions.down |= gamepad.povDown.pressed() || (StickThreshold < axis(1));
		directions.right |= gamepad.povRight.pressed() || (StickThreshold < axis(0));
	}
	return directions;
}

bool ControllerFaceButtonPressed() {
	if (const auto xinput = XInput(0); xinput.isConnected()) {
		if (xinput.buttonA.pressed() || xinput.buttonB.pressed() || xinput.buttonX.pressed() || xinput.buttonY.pressed()) return true;
	}
	if (const auto gamepad = Gamepad(0); gamepad.isConnected()) {
		for (size_t i : step(Min<size_t>(4, gamepad.buttons.size()))) {
			if (gamepad.buttons[i].pressed()) return true;
		}
	}
	return false;
}
