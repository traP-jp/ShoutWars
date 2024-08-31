# pragma once

# include <Siv3D.hpp>

/// @brief 部屋のメタ情報 (部屋に参加する時に必要な情報)
struct RoomInfo {
	// TODO: 部屋の設定やプレイヤーが選択しているキャラクターなど

	[[nodiscard]] explicit RoomInfo();

	[[nodiscard]] explicit RoomInfo(const JSON& json);

	[[nodiscard]] JSON toJSON() const;
};
