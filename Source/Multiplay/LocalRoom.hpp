# pragma once
# include "IRoom.hpp"

namespace Multiplay
{
	/// @brief 通信しない 1 人用の部屋。自分の行動はそのまま自分に届き、CPU の参加と CPU からの報告・行動は外から差し込む
	class LocalRoom : public IRoom
	{
	public:

		LocalRoom();

		/// @brief tick を経過時間に合わせて進める
		void update() override;

		/// @brief 相手の CPU は手元で動くので、自分の報告は誰にも届けない
		void sendReport(StringView type, const JSON& data) override;

		/// @brief 次の receiveActions で自分に届く
		void sendAction(StringView type, const JSON& data) override;

		void start() override;

		[[nodiscard]]
		Array<Event> receiveReports() override;

		[[nodiscard]]
		Array<Event> receiveActions() override;

		[[nodiscard]]
		const Joined& joined() const noexcept override;

		[[nodiscard]]
		const Array<User>& users() const noexcept override;

		[[nodiscard]]
		bool isOwner() const override;

		[[nodiscard]]
		Optional<uint64> lastTick() const noexcept override;

		[[nodiscard]]
		Array<Duration> receiveRTTs() override;

		[[nodiscard]]
		Duration timeSinceLastSync() const override;

		[[nodiscard]]
		const Optional<APIError>& error() const noexcept override;

		/// @brief CPU を部屋に参加させる
		void addCpu();

		[[nodiscard]]
		bool hasCpu() const noexcept;

		/// @brief CPU からの報告を届ける
		void sendCpuReport(StringView type, const JSON& data);

		/// @brief CPU からの行動を届ける
		void sendCpuAction(StringView type, const JSON& data);

	private:

		Joined m_joined;

		Array<User> m_users;

		Stopwatch m_clock{ StartImmediately::Yes };

		Optional<uint64> m_lastTick;

		Array<Event> m_reports;

		Array<Event> m_actions;

		Optional<APIError> m_error;

		[[nodiscard]]
		Event makeEvent(const UUIDValue& from, StringView type, const JSON& data) const;
	};
}
