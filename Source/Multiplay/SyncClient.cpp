# include "SyncClient.hpp"

using namespace std;

AsyncTask<SyncClient> SyncClient::createRoom(shared_ptr<APIClient> api, const String& userName, uint32 size) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/create", {
			{ U"version", api->version },
			{ U"user", { { U"name", userName } } },
			{ U"size", size },
		}).get();
		return SyncClient{
			api,
			*UUIDValue::Parse(response[U"session_id"].getString()),
			*UUIDValue::Parse(response[U"id"].getString()),
			*UUIDValue::Parse(response[U"user_id"].getString()),
			response[U"name"].getString(),
		};
	});
}

AsyncTask<SyncClient> SyncClient::joinRoom(shared_ptr<APIClient> api, const String& roomName, const String& userName) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/join", {
			{ U"version", api->version },
			{ U"name", roomName },
			{ U"user", { { U"name", userName } } },
		}).get();
		return SyncClient{
			api,
			*UUIDValue::Parse(response[U"session_id"].getString()),
			*UUIDValue::Parse(response[U"id"].getString()),
			*UUIDValue::Parse(response[U"user_id"].getString()),
			userName,
			RoomInfo{ response[U"room_info"] },
		};
	});
}

SyncClient::SyncClient(shared_ptr<APIClient> api, UUIDValue sessionId, UUIDValue roomId, UUIDValue userId, const String& roomName, const RoomInfo& roomInfo)
	: api(api), sessionId(sessionId), roomId(roomId), userId(userId), roomName(roomName), roomInfo(roomInfo) {}
