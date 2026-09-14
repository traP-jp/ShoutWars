# pragma once
# include "Multiplay/APIClient.hpp"

struct ServerConfig
{
	Multiplay::APIClient api;

	/// @brief 空なら同期の計測ログを書き出さない
	FilePath syncLogDirectory;
};

/// @brief 設定ファイルの server から読み込む。url が無ければ本番サーバーに繋ぐ
[[nodiscard]]
ServerConfig LoadServerConfig(FilePathView configPath, StringView version);
