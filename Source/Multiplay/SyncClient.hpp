# pragma once

# include "RoomInfo.hpp"
# include "APIClient.hpp"

# include <Siv3D.hpp>

class SyncClient {
public:
	struct User {
		const UUIDValue id;
		const String name;
	};

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param userName ユーザー名 (32 文字以内)
	 * @param size 部屋の最大人数 (2~4)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 403: 部屋数の上限に達した }
	 */
	static [[nodiscard]] AsyncTask<SyncClient> createRoom(std::shared_ptr<APIClient> api, const String& userName, uint32 size = 2);

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param roomName 部屋の名前 (6 桁の数字)
	 * @param userName ユーザー名 (32 文字以内)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 400: 部屋のバージョンが異なる, 403: 入室できない, 404: 部屋が存在しない }
	 */
	static [[nodiscard]] AsyncTask<SyncClient> joinRoom(std::shared_ptr<APIClient> api, const String& roomName, const String& userName);

	const std::shared_ptr<APIClient> api;

	const UUIDValue sessionId;
	const UUIDValue roomId;
	const UUIDValue userId;
	const String roomName; // 6 桁の数字

protected:
	RoomInfo roomInfo; // オーナー以外が更新したい時は同期でイベントを送る

	SyncClient(std::shared_ptr<APIClient> api, UUIDValue sessionId, UUIDValue roomId, UUIDValue userId, const String& roomName, const RoomInfo& roomInfo = RoomInfo{});
};
