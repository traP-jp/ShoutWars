# pragma once
# include "APIClient.hpp"

namespace Multiplay
{
	/// @brief 部屋との同期。サーバーの部屋 (Room) と、通信しない 1 人用の部屋 (LocalRoom) がある
	class IRoom
	{
	public:

		virtual ~IRoom() = default;

		/// @brief 毎フレーム呼ぶ
		virtual void update() = 0;

		/// @brief 次の同期で送る。同じ type の報告は最後のものだけが送られる
		virtual void sendReport(StringView type, const JSON& data) = 0;

		/// @brief 次の同期で送る。サーバーが受け取るまで送り直す
		virtual void sendAction(StringView type, const JSON& data) = 0;

		/// @brief 部屋主のみ有効
		virtual void start() = 0;

		[[nodiscard]]
		virtual Array<Event> receiveReports() = 0;

		[[nodiscard]]
		virtual Array<Event> receiveActions() = 0;

		[[nodiscard]]
		virtual const Joined& joined() const noexcept = 0;

		/// @brief 参加順。先頭が部屋主
		[[nodiscard]]
		virtual const Array<User>& users() const noexcept = 0;

		[[nodiscard]]
		virtual bool isOwner() const = 0;

		/// @brief 受信した最後のレコードの tick 番号
		[[nodiscard]]
		virtual Optional<uint64> lastTick() const noexcept = 0;

		/// @brief 前回の呼び出し以降に測った通信の往復時間 (サーバーでの保留を除く)
		[[nodiscard]]
		virtual Array<Duration> receiveRTTs() = 0;

		/// @brief 最後に同期を送ってからの経過時間
		[[nodiscard]]
		virtual Duration timeSinceLastSync() const = 0;

		/// @brief 値があれば同期は止まっている
		[[nodiscard]]
		virtual const Optional<APIError>& error() const noexcept = 0;
	};
}
