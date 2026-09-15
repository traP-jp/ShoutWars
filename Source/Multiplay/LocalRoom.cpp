# include "LocalRoom.hpp"

namespace Multiplay
{
	namespace
	{
		constexpr Duration TickDuration = 50ms;
	}

	LocalRoom::LocalRoom()
		: m_joined{ .sessionId = UUIDValue::Generate(), .userId = UUIDValue::Generate(), .code = U"", .nextTick = 0, .tickDuration = TickDuration }
		, m_users{ User{ .id = m_joined.userId, .name = U"You" } }
	{
		update();
	}

	void LocalRoom::update()
	{
		m_lastTick = static_cast<uint64>(m_clock.elapsed() / TickDuration);
	}

	void LocalRoom::sendReport(StringView, const JSON&) {}

	void LocalRoom::sendAction(const StringView type, const JSON& data)
	{
		m_actions << makeEvent(m_joined.userId, type, data);
	}

	void LocalRoom::start() {}

	Array<Event> LocalRoom::receiveReports()
	{
		return std::exchange(m_reports, {});
	}

	Array<Event> LocalRoom::receiveActions()
	{
		return std::exchange(m_actions, {});
	}

	const Joined& LocalRoom::joined() const noexcept
	{
		return m_joined;
	}

	const Array<User>& LocalRoom::users() const noexcept
	{
		return m_users;
	}

	bool LocalRoom::isOwner() const
	{
		return true;
	}

	Optional<uint64> LocalRoom::lastTick() const noexcept
	{
		return m_lastTick;
	}

	Array<Duration> LocalRoom::receiveRTTs()
	{
		return {};
	}

	Duration LocalRoom::timeSinceLastSync() const
	{
		return 0s;
	}

	const Optional<APIError>& LocalRoom::error() const noexcept
	{
		return m_error;
	}

	void LocalRoom::addCpu()
	{
		if (hasCpu())
		{
			throw Error{ U"The CPU has already joined" };
		}

		m_users << User{ .id = UUIDValue::Generate(), .name = U"CPU" };
	}

	bool LocalRoom::hasCpu() const noexcept
	{
		return (2 <= m_users.size());
	}

	void LocalRoom::sendCpuReport(const StringView type, const JSON& data)
	{
		if (not hasCpu())
		{
			throw Error{ U"The CPU has not joined" };
		}

		m_reports << makeEvent(m_users[1].id, type, data);
	}

	void LocalRoom::sendCpuAction(const StringView type, const JSON& data)
	{
		if (not hasCpu())
		{
			throw Error{ U"The CPU has not joined" };
		}

		m_actions << makeEvent(m_users[1].id, type, data);
	}

	Event LocalRoom::makeEvent(const UUIDValue& from, const StringView type, const JSON& data) const
	{
		return Event{ .id = UUIDValue::Generate(), .tick = m_lastTick.value_or(0), .from = from, .type = String{ type }, .data = detail::Detach(data) };
	}
}
