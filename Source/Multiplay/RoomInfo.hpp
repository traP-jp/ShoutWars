# pragma once

# include <Siv3D.hpp>

/// @brief 部屋のメタ情報 (部屋に参加する時に必要な情報)
struct RoomInfo {
	// TODO: 部屋の設定やプレイヤーが選択しているキャラクターなど

	//character:"{ \"プレイヤーのID\": キャラクターのID }"
	String character;
	bool is_ready = false;
	int room_timer = 0;

	[[nodiscard]] explicit RoomInfo();

	[[nodiscard]] explicit RoomInfo(const JSON& json);

	[[nodiscard]] JSON toJSON() const;
};
