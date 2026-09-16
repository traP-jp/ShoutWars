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

	/// @brief 描画のたびに待って、目標のフレームレートを超えないようにする
	/// @remark https://qiita.com/m4saka/items/5da6cd4b57bc894d35dd の FrameRateLimit アドオン (CC0) を元にした
	class FrameRateLimitAddon : public IAddon {
	public:
		explicit FrameRateLimitAddon(int32 targetFps)
			: oneFrameDuration{ std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::duration<double>{ 1.0 / targetFps }) } {}

		void postPresent() override {
			//少しの遅れ (寝過ごしなど) は次のフレームで取り戻して、平均を目標のフレームレートに保つ。大きく遅れたら今から数え直す
			sleepUntil = Max(sleepUntil + oneFrameDuration, std::chrono::steady_clock::now() - MaxDrift);
			std::this_thread::sleep_until(sleepUntil);
		}

	private:
		static constexpr std::chrono::steady_clock::duration MaxDrift = 10ms;
		std::chrono::steady_clock::duration oneFrameDuration;
		std::chrono::steady_clock::time_point sleepUntil = std::chrono::steady_clock::now();
	};
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
	//DEBT: フレーム数で時間を数えている箇所があり、垂直同期のままだと 60 Hz より高いリフレッシュレートの画面で速く回って挙動が変わる (#24)。
	//直すまでは、そういう画面では垂直同期を切って 60 fps に制限する (60 Hz の画面では、垂直同期のまま制限しない方がなめらか)
	if (60.5 < System::GetCurrentMonitor().refreshRate.value_or(60.0)) {
		Graphics::SetVSyncEnabled(false);
		Addon::Register(U"FrameRateLimit", std::make_unique<FrameRateLimitAddon>(60));
	}
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
