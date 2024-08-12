#include "common.hpp"
#include "Game.hpp"

void Main() {
	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.0, 0.0, 0.0 });
	//windowsサイズ
	Window::Resize(1920, 1080);
	//フルスクリーン
	Window::SetFullscreen(true);
	//タイトル
	Window::SetTitle(U"ShoutWars");

	App manager;
	//manager.add<Title>(State::Title);
	//manager.add<Matching>(State::Matching);
	manager.add<Game>(State::Game);
	//manager.add<Result>(State::Result);

	while (System::Update() && manager.update()) {};
}
