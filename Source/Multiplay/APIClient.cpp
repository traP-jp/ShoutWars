# include "APIClient.hpp"

namespace Multiplay
{
	namespace
	{
		/// @brief スリーブ中のサーバーが起きるまでの待ち (約 10 秒) より長く取る
		constexpr Duration RequestTimeout = 15s;

		[[nodiscard]]
		UUIDValue ReadUUID(const JSON& json)
		{
			if (const auto uuid = UUIDValue::Parse(json.getString()))
			{
				return *uuid;
			}

			throw Error{ U"Invalid UUID: {}"_fmt(json.getString()) };
		}

		[[nodiscard]]
		Array<Event> ReadEvents(const JSON& json, const uint64 lastTick)
		{
			Array<Event> events;

			for (const auto& element : json.arrayView())
			{
				events << Event{
					.id = ReadUUID(element[U"id"]),
					.tick = (element.contains(U"tick") ? element[U"tick"].get<uint64>() : lastTick),
					.from = ReadUUID(element[U"from"]),
					.type = element[U"type"].getString(),
					.data = detail::Detach(element[U"data"]),
				};
			}

			return events;
		}

		[[nodiscard]]
		JSON ToJSON(const Array<OutgoingEvent>& events)
		{
			Array<JSON> array;

			for (const auto& event : events)
			{
				array << JSON{ { U"id", event.id.str() }, { U"type", event.type }, { U"data", event.data } };
			}

			return array;
		}

		[[nodiscard]]
		Joined ReadJoined(const JSON& json, String code)
		{
			return Joined{
				.sessionId = ReadUUID(json[U"session_id"]),
				.userId = ReadUUID(json[U"user_id"]),
				.code = std::move(code),
				.nextTick = json[U"next_tick"].get<uint64>(),
				.tickDuration = Milliseconds{ json[U"tick_ms"].get<int64>() },
				.roomInfo = (json.contains(U"room_info") ? detail::Detach(json[U"room_info"]) : JSON{}),
			};
		}

		[[nodiscard]]
		SyncResponse ReadSyncResponse(const JSON& json)
		{
			const uint64 nextTick = json[U"next_tick"].get<uint64>();
			const JSON usersJSON = json[U"room_users"];

			Array<User> users;

			for (const auto& element : usersJSON.arrayView())
			{
				users << User{
					.id = ReadUUID(element[U"id"]),
					.name = element[U"name"].getString(),
					.absent = element[U"absent"].get<bool>(),
				};
			}

			return SyncResponse{
				.nextTick = nextTick,
				.users = std::move(users),
				.started = json[U"started"].get<bool>(),
				.reports = ReadEvents(json[U"reports"], (nextTick - 1)),
				.actions = ReadEvents(json[U"actions"], (nextTick - 1)),
				.desync = json[U"desync"].get<bool>(),
				.held = Milliseconds{ json[U"held_ms"].get<int64>() },
			};
		}
	}

	namespace detail
	{
		JSON Detach(const JSON& view)
		{
			JSON value;
			value = view;
			return value;
		}

		std::expected<JSON, APIError> ReadResponse(std::expected<HTTPResult, TransportError> result)
		{
			if (not result)
			{
				return std::unexpected{ (result.error() == TransportError::Timeout)
					? APIError{ U"timeout", U"The server did not respond in time." }
					: APIError{ U"network", U"Could not communicate with the server." } };
			}

			const int32 status = FromEnum(result->status);
			JSON body = JSON::FromMessagePack(result->body);

			// スリーブ明けの NeoShowcase はアプリに届く前に 3xx や HTML を返す。
			if (((300 <= status) && (status < 400)) || (not body.isObject()))
			{
				return std::unexpected{ APIError{ U"unavailable", U"The server is not ready (HTTP {})."_fmt(status) } };
			}

			if (400 <= status)
			{
				try
				{
					const JSON error = body[U"error"];
					return std::unexpected{ APIError{ error[U"code"].getString(), error[U"message"].getString() } };
				}
				catch (const Error& error)
				{
					return std::unexpected{ InvalidResponse(error) };
				}
			}

			return body;
		}

		APIError InvalidResponse(const Error& error)
		{
			return APIError{ U"invalid_response", error.what() };
		}
	}

	APIClient::APIClient(std::shared_ptr<IHTTPTransport> transport, URL baseURL, String version, String password)
		: m_transport{ std::move(transport) }
		, m_baseURL{ std::move(baseURL) }
		, m_version{ std::move(version) }
		, m_password{ std::move(password) } {}

	APICall<Joined> APIClient::create(const StringView userName, const int32 size) const
	{
		const JSON body{ { U"version", m_version }, { U"user", JSON{ { U"name", userName } } }, { U"size", size } };

		return send<Joined>(HTTPMethod::Post, U"/v3/room/create", body, RequestTimeout,
			[](const JSON& json) { return ReadJoined(json, json[U"code"].getString()); });
	}

	APICall<Joined> APIClient::join(const StringView code, const StringView userName) const
	{
		const JSON body{ { U"version", m_version }, { U"code", code }, { U"user", JSON{ { U"name", userName } } } };

		return send<Joined>(HTTPMethod::Post, U"/v3/room/join", body, RequestTimeout,
			[code = String{ code }](const JSON& json) { return ReadJoined(json, code); });
	}

	APICall<SyncResponse> APIClient::sync(const SyncRequest& request, const Duration timeout) const
	{
		JSON body{
			{ U"session_id", request.sessionId.str() },
			{ U"next_tick", request.nextTick },
			{ U"applied", request.applied },
			{ U"reports", ToJSON(request.reports) },
			{ U"actions", ToJSON(request.actions) },
		};

		if (request.roomInfo)
		{
			body[U"room_info"] = *request.roomInfo;
		}

		return send<SyncResponse>(HTTPMethod::Post, U"/v3/room/sync", body, timeout, ReadSyncResponse);
	}

	APICall<Started> APIClient::start(const UUIDValue& sessionId) const
	{
		return send<Started>(HTTPMethod::Post, U"/v3/room/start", JSON{ { U"session_id", sessionId.str() } }, RequestTimeout,
			[](const JSON&) { return Started{}; });
	}

	APICall<ServerStatus> APIClient::status() const
	{
		return send<ServerStatus>(HTTPMethod::Get, U"/v3/status", JSON::Invalid(), RequestTimeout,
			[](const JSON& json) { return ServerStatus{ json[U"room_count"].get<int32>(), json[U"room_limit"].get<int32>() }; });
	}

	template <class Type>
	APICall<Type> APIClient::send(const HTTPMethod method, const StringView path, const JSON& body, const Duration timeout, typename APICall<Type>::Decoder decode) const
	{
		HTTPRequest request{ .method = method, .url = (m_baseURL + path), .timeout = timeout };

		if (method == HTTPMethod::Post)
		{
			request.headers[U"Content-Type"] = U"application/msgpack";
			request.body = body.toMessagePack();
		}

		if (not m_password.isEmpty())
		{
			request.headers[U"Authorization"] = (U"Bearer " + m_password);
		}

		return APICall<Type>{ m_transport->send(std::move(request)), std::move(decode) };
	}
}
