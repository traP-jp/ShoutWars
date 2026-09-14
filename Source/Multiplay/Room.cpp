# include "Room.hpp"

namespace Multiplay
{
	namespace
	{
		constexpr size_t MaxEventsPerRequest = 64;

		constexpr Duration SyncTimeout = 3s;

		[[nodiscard]]
		bool IsTransient(const APIError& error)
		{
			static constexpr std::array<StringView, 5> Codes{ U"unavailable", U"timeout", U"network", U"already_synced", U"internal" };
			return std::ranges::contains(Codes, StringView{ error.code });
		}
	}

	Room::Room(APIClient api, Joined joined)
		: m_api{ std::move(api) }
		, m_joined{ std::move(joined) }
		, m_nextTick{ m_joined.nextTick }
	{
		sendSync();
	}

	void Room::update()
	{
		if (m_error)
		{
			return;
		}

		if (m_sync.isReady())
		{
			onSynced(m_sync.get());
		}
		else if ((not m_sync.isValid()) && m_retryTimer.reachedZero())
		{
			sendSync();
		}

		if (m_start.isReady())
		{
			onStarted(m_start.get());
		}
	}

	void Room::sendReport(const StringView type, const JSON& data)
	{
		const auto it = std::ranges::find_if(m_reports, [&](const OutgoingEvent& report) { return (report.type == type); });

		if (it != m_reports.end())
		{
			it->data = detail::Detach(data);
		}
		else
		{
			m_reports << OutgoingEvent{ UUIDValue::Generate(), String{ type }, detail::Detach(data) };
		}
	}

	void Room::sendAction(const StringView type, const JSON& data)
	{
		m_actions << OutgoingEvent{ UUIDValue::Generate(), String{ type }, detail::Detach(data) };
	}

	void Room::setRoomInfo(const JSON& roomInfo)
	{
		m_roomInfo = detail::Detach(roomInfo);
	}

	void Room::start()
	{
		m_startRequested = true;
		sendStartIfRequested();
	}

	Array<Event> Room::receiveReports()
	{
		return std::exchange(m_receivedReports, {});
	}

	Array<Event> Room::receiveActions()
	{
		return std::exchange(m_receivedActions, {});
	}

	const Joined& Room::joined() const noexcept
	{
		return m_joined;
	}

	const Array<User>& Room::users() const noexcept
	{
		return m_users;
	}

	bool Room::isOwner() const
	{
		return ((not m_users.isEmpty()) && (m_users.front().id == m_joined.userId));
	}

	bool Room::isStarted() const noexcept
	{
		return m_started;
	}

	Optional<uint64> Room::lastTick() const noexcept
	{
		return m_lastTick;
	}

	Array<Duration> Room::receiveRTTs()
	{
		return std::exchange(m_rtts, {});
	}

	Duration Room::timeSinceLastSync() const
	{
		return m_syncStopwatch.elapsed();
	}

	const Optional<APIError>& Room::error() const noexcept
	{
		return m_error;
	}

	void Room::sendSync()
	{
		m_sentActionCount = Min(m_actions.size(), MaxEventsPerRequest);
		m_sentRoomInfo = std::exchange(m_roomInfo, none);

		const SyncRequest request{
			.sessionId = m_joined.sessionId,
			.nextTick = m_nextTick,
			.applied = m_applied,
			.reports = std::exchange(m_reports, {}),
			.actions = Array<OutgoingEvent>(m_actions.begin(), (m_actions.begin() + m_sentActionCount)),
			.roomInfo = m_sentRoomInfo,
		};

		m_syncStopwatch.restart();
		m_sync = m_api.sync(request, SyncTimeout);
	}

	void Room::onSynced(std::expected<SyncResponse, APIError> result)
	{
		if (not result)
		{
			if (not IsTransient(result.error()))
			{
				m_error = result.error();
				return;
			}

			// 報告は古くなるので捨て、確認と部屋情報は送り直す。
			if (not m_roomInfo)
			{
				m_roomInfo = std::move(m_sentRoomInfo);
			}

			m_sentRoomInfo.reset();
			m_sentActionCount = 0;
			m_retryTimer.restart(m_joined.tickDuration);
			return;
		}

		if (result->desync)
		{
			m_error = APIError{ U"desync", U"The players' game states have diverged." };
			return;
		}

		m_rtts << (m_syncStopwatch.elapsed() - result->held);
		m_applied += (result->reports.size() + result->actions.size());
		std::ranges::move(result->reports, std::back_inserter(m_receivedReports));
		std::ranges::move(result->actions, std::back_inserter(m_receivedActions));
		m_users = std::move(result->users);
		m_started = result->started;
		m_nextTick = result->nextTick;
		m_lastTick = (result->nextTick - 1);
		m_actions.erase(m_actions.begin(), (m_actions.begin() + m_sentActionCount));
		m_sentActionCount = 0;
		m_sentRoomInfo.reset();

		sendSync();
		sendStartIfRequested();
	}

	void Room::sendStartIfRequested()
	{
		if (m_startRequested && (not m_started) && (not m_start.isValid()))
		{
			m_start = m_api.start(m_joined.sessionId);
		}
	}

	void Room::onStarted(const std::expected<Started, APIError> result)
	{
		// 送り直した開始要求が、先に届いていた分と重なると game_started が返る。
		if (result || (result.error().code == U"game_started"))
		{
			m_startRequested = false;
		}
		else if (not IsTransient(result.error()))
		{
			m_error = result.error();
		}
	}
}
