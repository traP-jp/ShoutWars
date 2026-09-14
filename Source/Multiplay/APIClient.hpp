# pragma once
# include "HTTPTransport.hpp"

namespace Multiplay
{
	struct APIError
	{
		/// @brief サーバーの返した code。サーバーの応答を得られなかった場合は unavailable / timeout / network / invalid_response
		String code;

		String message;
	};

	struct Joined
	{
		UUIDValue sessionId;

		UUIDValue userId;

		String code;

		uint64 nextTick = 0;

		Duration tickDuration;

		/// @brief 部屋の作成時は null
		JSON roomInfo;
	};

	struct User
	{
		UUIDValue id;

		String name;

		bool absent = false;
	};

	struct OutgoingEvent
	{
		UUIDValue id;

		String type;

		JSON data;
	};

	struct Event
	{
		UUIDValue id;

		uint64 tick = 0;

		UUIDValue from;

		String type;

		JSON data;
	};

	struct SyncRequest
	{
		UUIDValue sessionId;

		uint64 nextTick = 0;

		uint64 applied = 0;

		Array<OutgoingEvent> reports;

		Array<OutgoingEvent> actions;

		Optional<JSON> roomInfo;
	};

	struct SyncResponse
	{
		uint64 nextTick = 0;

		Array<User> users;

		bool started = false;

		Array<Event> reports;

		Array<Event> actions;

		bool desync = false;

		Duration held;
	};

	struct Started {};

	struct ServerStatus
	{
		int32 roomCount = 0;

		int32 roomLimit = 0;
	};

	namespace detail
	{
		/// @brief JSON の一部をコピーすると元の JSON を参照したままになるため、代入で中身を複製する
		[[nodiscard]]
		JSON Detach(const JSON& view);

		[[nodiscard]]
		std::expected<JSON, APIError> ReadResponse(std::expected<HTTPResult, TransportError> result);

		[[nodiscard]]
		APIError InvalidResponse(const Error& error);
	}

	/// @brief 送信中の API 呼び出し。破棄した後に届いた応答は捨てられる
	template <class Type>
	class APICall
	{
	public:

		using Decoder = std::function<Type(const JSON&)>;

		APICall() = default;

		APICall(std::unique_ptr<IHTTPCall> call, Decoder decode)
			: m_call{ std::move(call) }
			, m_decode{ std::move(decode) } {}

		[[nodiscard]]
		bool isValid() const noexcept
		{
			return static_cast<bool>(m_call);
		}

		[[nodiscard]]
		bool isReady()
		{
			return (m_call && m_call->isReady());
		}

		/// @remark isReady() が true を返した後に 1 回だけ呼べる
		[[nodiscard]]
		std::expected<Type, APIError> get()
		{
			auto result = std::exchange(m_call, nullptr)->get();
			m_elapsed = (result ? Optional<Duration>{ result->elapsed } : none);

			const auto response = detail::ReadResponse(std::move(result));

			if (not response)
			{
				return std::unexpected{ response.error() };
			}

			try
			{
				return m_decode(*response);
			}
			catch (const Error& error)
			{
				return std::unexpected{ detail::InvalidResponse(error) };
			}
		}

		/// @brief 直前の get() で応答を受け取れていれば、リクエストを投げてから受け取り終えるまでの時間
		[[nodiscard]]
		Optional<Duration> elapsed() const noexcept
		{
			return m_elapsed;
		}

	private:

		std::unique_ptr<IHTTPCall> m_call;

		Decoder m_decode;

		Optional<Duration> m_elapsed;
	};

	class APIClient
	{
	public:

		/// @param baseURL バージョンのパス (/v3) を含まない URL (例: U"https://shoutwars.trap.games/develop/api")
		APIClient(std::shared_ptr<IHTTPTransport> transport, URL baseURL, String version, String password = U"");

		[[nodiscard]]
		APICall<Joined> create(StringView userName, int32 size) const;

		[[nodiscard]]
		APICall<Joined> join(StringView code, StringView userName) const;

		[[nodiscard]]
		APICall<SyncResponse> sync(const SyncRequest& request, Duration timeout) const;

		[[nodiscard]]
		APICall<Started> start(const UUIDValue& sessionId) const;

		[[nodiscard]]
		APICall<ServerStatus> status() const;

	private:

		std::shared_ptr<IHTTPTransport> m_transport;

		URL m_baseURL;

		String m_version;

		String m_password;

		template <class Type>
		[[nodiscard]]
		APICall<Type> send(HTTPMethod method, StringView path, const JSON& body, Duration timeout, typename APICall<Type>::Decoder decode) const;
	};
}
