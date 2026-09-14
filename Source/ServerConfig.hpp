# pragma once
# include "Multiplay/APIClient.hpp"

/// @brief 設定ファイルの server.url と server.password から API クライアントを作る。url が無ければ本番サーバーに繋ぐ
[[nodiscard]]
Multiplay::APIClient LoadAPIClient(FilePathView configPath, StringView version);
