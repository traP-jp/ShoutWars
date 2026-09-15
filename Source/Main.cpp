#include "common.hpp"
#include "Credits.hpp"
#include "logo.hpp"
#include "Title.hpp"
#include "Matching.hpp"
#include "Game.hpp"
#include "Calibration.hpp"

namespace {
	/// @brief 設定ファイルの exhibition が true なら展示用モード (フルスクリーンで起動し、前の人のキャリブレーションを消す)
	[[nodiscard]] bool LoadExhibitionMode(FilePathView configPath) {
		const JSON config = JSON::Load(configPath);
		if (!config.isObject() || !config.contains(U"exhibition")) return false;
		if (!config[U"exhibition"].isBool()) throw Error{ U"exhibition in the config file must be a boolean." };
		return config[U"exhibition"].get<bool>();
	}

	//フルスクリーンでは閉じるボタンが無いので、ESC で閉じられることを、どの画面でも UI と被らない左下の隅に小さく書いておく
	void DrawExitHint(const Font& font) {
		font(U"ESC で終了").draw(TextStyle::Outline(0.35, ColorF{ 0.0 }), 22, Arg::bottomLeft(6, 1078), Palette::White);
	}
}

void Main() {
	Credits::Load(U"CREDITS.ini").each(LicenseManager::AddLicense);

	// 背景の色を設定する | Set the background color
	Scene::SetBackground(ColorF{ 0.0, 0.0, 0.0 });
	//windowsサイズ
	Window::Resize(1920, 1080);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetStyle(WindowStyle::Sizable);
	Window::Resize(1280, 720);
	//フルスクリーン
	const bool exhibition = LoadExhibitionMode(U"config.json");
	if (exhibition) Window::SetFullscreen(true);
	//タイトル
	Window::SetTitle(U"Shout Wars v{}"_fmt(GameVersion));

	App manager;
	manager.add<logo>(State::logo);
	manager.add<Calibration>(State::Calibration);
	manager.add<Title>(State::Title);
	manager.add<Matching>(State::Matching);
	manager.add<Game>(State::Game);
	//展示では遊ぶ人が入れ替わるので、前の人の声の登録を残さない (入力感度は会場に合わせたものなので残す)
	if (exhibition) manager.get()->phoneme.clearMFCC();
	

# if defined(_DEBUG) || defined(DEBUG)
	manager.init(State::Title);
# endif

	const Font exitHintFont{ FontMethod::MSDF, 32, Typeface::Heavy };
	while (System::Update() && manager.update()) {
		if (exhibition) DrawExitHint(exitHintFont);
	}
}
