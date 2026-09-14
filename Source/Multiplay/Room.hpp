# pragma once
# include "APIClient.hpp"

namespace Multiplay
{
	/// @brief 参加した部屋との同期。構築した時点から同期を始める
	class Room
	{
	public:

		/// @param logDirectory 空でなければ、同期ごとの計測をこのディレクトリに CSV で書き出す
		Room(APIClient api, Joined joined, FilePathView logDirectory = U"");

		/// @brief 毎フレーム呼ぶ
		void update();

		/// @brief 次の同期で送る。同じ type の報告は最後のものだけが送られる
		void sendReport(StringView type, const JSON& data);

		/// @brief 次の同期で送る。サーバーが受け取るまで送り直す
		void sendAction(StringView type, const JSON& data);

		/// @brief 部屋主のみ有効
		void setRoomInfo(const JSON& roomInfo);

		/// @brief 部屋主のみ有効。開始したかどうかは isStarted() で分かる
		void start();

		[[nodiscard]]
		Array<Event> receiveReports();

		[[nodiscard]]
		Array<Event> receiveActions();

		[[nodiscard]]
		const Joined& joined() const noexcept;

		/// @brief 参加順。先頭が部屋主
		[[nodiscard]]
		const Array<User>& users() const noexcept;

		[[nodiscard]]
		bool isOwner() const;

		[[nodiscard]]
		bool isStarted() const noexcept;

		/// @brief 受信した最後のレコードの tick 番号
		[[nodiscard]]
		Optional<uint64> lastTick() const noexcept;

		/// @brief 前回の呼び出し以降に測った通信の往復時間 (サーバーでの保留を除く)
		[[nodiscard]]
		Array<Duration> receiveRTTs();

		/// @brief 最後に同期を送ってからの経過時間
		[[nodiscard]]
		Duration timeSinceLastSync() const;

		/// @brief 値があれば同期は止まっている
		[[nodiscard]]
		const Optional<APIError>& error() const noexcept;

	private:

		APIClient m_api;

		Joined m_joined;

		uint64 m_nextTick;

		Optional<uint64> m_lastTick;

		uint64 m_applied = 0;

		Array<OutgoingEvent> m_reports;

		/// @brief 先頭の m_sentActionCount 件は送信中
		Array<OutgoingEvent> m_actions;

		size_t m_sentActionCount = 0;

		Optional<JSON> m_roomInfo;

		Optional<JSON> m_sentRoomInfo;

		Array<Event> m_receivedReports;

		Array<Event> m_receivedActions;

		Array<User> m_users;

		bool m_started = false;

		bool m_startRequested = false;

		APICall<SyncResponse> m_sync;

		Stopwatch m_syncStopwatch;

		Timer m_retryTimer;

		APICall<Started> m_start;

		Array<Duration> m_rtts;

		Optional<APIError> m_error;

		TextWriter m_log;

		void sendSync();

		void onSynced(std::expected<SyncResponse, APIError> result);

		void writeLog(Duration elapsed, const std::expected<SyncResponse, APIError>& result);

		void sendStartIfRequested();

		void onStarted(std::expected<Started, APIError> result);
	};
}
