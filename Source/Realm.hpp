# pragma once
# include <Siv3D.hpp>

/// @brief 合言葉の上限 (サーバーと揃える)
inline constexpr size_t MaxRealmLength = 64;

/// @brief 設定ファイルの realm (合言葉) を読む。書かれていなければ空
[[nodiscard]] String LoadRealm(FilePathView configPath);

/// @brief 合言葉を設定ファイルに書く。空なら realm を消す
void SaveRealm(FilePathView configPath, StringView realm);

/// @brief 仲間内で共有しやすく、当てられにくい合言葉を作る
[[nodiscard]] String GenerateRealm();
