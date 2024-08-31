# pragma once

# include "RoomInfo.hpp"
# include "APIClient.hpp"

# include <Siv3D.hpp>

class SyncClient {
public:
	struct Event {
		const UUIDValue id;
		const UUIDValue syncId;
		const UUIDValue from;
		const String type;
		const JSON data;
	};

	struct User {
		const UUIDValue id;
		const String name;
	};

	struct StartInfo {};

	inline static const MillisecondsF Interval = 100ms;

	static [[nodiscard]] UUIDValue genUUIDv7();

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param userName ユーザー名 (32 文字以内)
	 * @param size 部屋の最大人数 (2~4)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 403: 部屋数の上限に達した }
	 */
	static [[nodiscard]] AsyncTask<std::unique_ptr<SyncClient>> createRoom(std::shared_ptr<APIClient> api, const String& userName, uint32 size = 2);

	/**
	 * @brief エラーは APIClient::send も参照
	 * @param api API クライアント
	 * @param roomName 部屋の名前 (6 桁の数字)
	 * @param userName ユーザー名 (32 文字以内)
	 * @return 部屋を同期するためのクライアントの AsyncTask
	 * @throw APIClient::HTTPError { 400: 部屋のバージョンが異なる, 403: 入室できない, 404: 部屋が存在しない }
	 */
	static [[nodiscard]] AsyncTask<std::unique_ptr<SyncClient>> joinRoom(std::shared_ptr<APIClient> api, const String& roomName, const String& userName);

	const std::shared_ptr<APIClient> api;

	const UUIDValue sessionId;
	const UUIDValue roomId;
	const UUIDValue userId;
	const String roomName; // 6 桁の数字
	
	RoomInfo roomInfo;

	[[nodiscard]] SyncClient(std::shared_ptr<APIClient> api, UUIDValue sessionId, UUIDValue roomId, UUIDValue userId, const String& roomName, const RoomInfo& roomInfo = RoomInfo{});

	/**
	 * @brief 報告イベントの送信を予約する (自分には送信されない)
	 * @param type イベントの種類
	 * @param data イベントのデータ
	 */
	void sendReport(const String& type, const JSON& data);

	/**
	 * @brief 確認イベントの送信を予約する (自分にも送信される)
	 * @param type イベントの種類
	 * @param data イベントのデータ
	 */
	void sendAction(const String& type, const JSON& data);

	/**
	 * @brief 報告イベントを受信する (毎フレーム呼び出す)
	 * @return 報告イベントがあればその情報を返す
	 * @retval nullptr 全ての報告イベントを受信済み
	 */
	[[nodiscard]] std::unique_ptr<Event> receiveReport();

	/**
	 * @brief 確認イベントを受信する (毎フレーム呼び出す)
	 * @return 確認イベントがあればその情報を返す
	 * @retval nullptr 全ての確認イベントを受信済み
	 */
	[[nodiscard]] std::unique_ptr<Event> receiveAction();

	[[nodiscard]] const std::map<UUIDValue, User>& getUsers() const;

	[[nodiscard]] const User& getMe() const;

	/**
	 * @return 自分がオーナーかどうか
	 */
	[[nodiscard]] bool isOwner() const;

	[[nodiscard]] bool isSyncing() const;

	/**
	 * @brief 同期の通信に関する処理をする時間ならする。(毎フレーム呼び出す) エラーは APIClient::send も参照
	 * @throw APIClient::HTTPError { 401: セッションが無効, 403: 同期の条件を満たしていない, 404: 部屋が削除された, 429: 同期タイミングが早すぎる }
	 */
	void update();

	/**
	 * @brief ゲームを開始する申請を送り、部屋の新規参加を受け付けなくする (オーナーのみ送信可能)
	 * @throw Error オーナーでないか、既に開始申請を送信済み
	 */
	void sendStart();

	[[nodiscard]] bool isStarting() const;

	/**
	 * @brief ゲームが開始したことを受信する (送信者のみ一度だけ受信可能) 受信したらイベントで全員に通知しましょう
	 * @return ゲームが開始したらその情報を返す
	 * @retval none ゲームが開始したことを受信していない
	 * @throw APIClient::HTTPError { 401: セッションが無効, 403: 開始の条件を満たしていない, 404: 部屋が削除された }
	 */
	[[nodiscard]] Optional<StartInfo> receiveStart();

protected:
	Array<std::unique_ptr<Event>> pendingReports, pendingActions;
	Array<std::unique_ptr<Event>> sentReports, sentActions; // 送信に失敗する可能性があるので保持
	Array<std::unique_ptr<Event>> receivedReports, receivedActions;
	std::map<UUIDValue, User> users; // 一番 ID が若いのがオーナー
	AsyncTask<JSON> syncTask;
	Timer timer{ Interval, StartImmediately::Yes };
	AsyncTask<JSON> startTask;

	void startSync();

	UUIDValue finishSync();
};
