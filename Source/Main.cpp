#include "common.hpp"
#include "logo.hpp"
#include "Title.hpp"
#include "Matching.hpp"
#include "Game.hpp"
#include "Calibration.hpp"

void Main() {
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.0, 0.0, 0.0 });
	//windowsサイズ
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);
	Window::Resize(1280, 720);
	//フルスクリーン
	//Window::SetFullscreen(true);
	//タイトル
	Window::SetTitle(U"ShoutWars");

	App manager;
	manager.add<logo>(State::logo);
	manager.add<Calibration>(State::Calibration);
	manager.add<Title>(State::Title);
	manager.add<Matching>(State::Matching);
	manager.add<Game>(State::Game);
	

	//XXX:debug用
	//manager.init(State::Title);

	while (System::Update() && manager.update()) {};
}
