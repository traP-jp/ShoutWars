# pragma once
# include <Siv3D.hpp>

/// @brief 描画の品質。低いときは重い演出を省く
enum class GraphicsQuality {
	Low,
	High,
};

/// @brief 設定ファイルの graphicsQuality ("low" か "high") を読む。書かれていなければ High
[[nodiscard]] GraphicsQuality LoadGraphicsQuality(FilePathView configPath);
