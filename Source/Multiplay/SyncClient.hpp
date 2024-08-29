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

	inline static const MillisecondsF Interval = 100ms;

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param userName ユーザー名 (32 文字以内)
	 * @param size 部屋の最大人数 (2~4)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 403: 部屋数の上限に達した }
	 */
	static [[nodiscard]] AsyncTask<std::shared_ptr<SyncClient>> createRoom(std::shared_ptr<APIClient> api, const String& userName, uint32 size = 2);

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param roomName 部屋の名前 (6 桁の数字)
	 * @param userName ユーザー名 (32 文字以内)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 400: 部屋のバージョンが異なる, 403: 入室できない, 404: 部屋が存在しない }
	 */
	static [[nodiscard]] AsyncTask<std::shared_ptr<SyncClient>> joinRoom(std::shared_ptr<APIClient> api, const String& roomName, const String& userName);

	const std::shared_ptr<APIClient> api;

	const UUIDValue sessionId;
	const UUIDValue roomId;
	const UUIDValue userId;
	const String roomName; // 6 桁の数字

	RoomInfo roomInfo;

	[[nodiscard]] SyncClient(std::shared_ptr<APIClient> api, UUIDValue sessionId, UUIDValue roomId, UUIDValue userId, const String& roomName, const RoomInfo& roomInfo = RoomInfo{});

	[[nodiscard]] const std::map<UUIDValue, User>& getUsers() const;

	[[nodiscard]] const User& getMe() const;

	/**
	 * @return 自分がオーナーかどうか
	 */
	[[nodiscard]] bool isOwner() const;

	[[nodiscard]] bool isSyncing() const;

	/**
	 * @brief 同期の通信に関する処理をする時間ならする (毎フレーム呼び出す)
	 * @return エラーがあればそれを返す。(ログなどに出力することを推奨) APIClient::send も参照
	 * @retval none 問題無し
	 * @retval APIClient::HTTPError { 401: セッションが無効, 403: 同期の条件を満たしていない, 404: 部屋が削除された, 429: 同期タイミングが早すぎる }
	 */
	Optional<Error> update();

protected:
	std::map<UUIDValue, User> users; // 一番 ID が若いのがオーナー
	AsyncTask<JSON> syncTask;
	Timer timer{ Interval, StartImmediately::Yes };

	void startSync();

	UUIDValue finishSync();
};
