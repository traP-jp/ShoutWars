# include "SyncClient.hpp"

using namespace std;

AsyncTask<shared_ptr<SyncClient>> SyncClient::createRoom(shared_ptr<APIClient> api, const String& userName, uint32 size) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/create", {
			{ U"version", api->version },
			{ U"user", { { U"name", userName } } },
			{ U"size", size },
		}).get();
		const auto client = make_shared<SyncClient>(
			api,
			*UUIDValue::Parse(response[U"session_id"].getString()),
			*UUIDValue::Parse(response[U"id"].getString()),
			*UUIDValue::Parse(response[U"user_id"].getString()),
			response[U"name"].getString()
		);
		System::Sleep(Interval);
		client->startSync();
		client->finishSync();
		return client;
	});
}

AsyncTask<shared_ptr<SyncClient>> SyncClient::joinRoom(shared_ptr<APIClient> api, const String& roomName, const String& userName) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/join", {
			{ U"version", api->version },
			{ U"name", roomName },
			{ U"user", { { U"name", userName } } },
		}).get();
		const JSON roomInfoJSON = response[U"room_info"];
		const auto client = make_shared<SyncClient>(
			api,
			*UUIDValue::Parse(response[U"session_id"].getString()),
			*UUIDValue::Parse(response[U"id"].getString()),
			*UUIDValue::Parse(response[U"user_id"].getString()),
			userName,
			roomInfoJSON.isObject() ? RoomInfo{ roomInfoJSON } : RoomInfo{}
		);
		System::Sleep(Interval);
		client->startSync();
		client->finishSync();
		return client;
	});
}

SyncClient::SyncClient(shared_ptr<APIClient> api, UUIDValue sessionId, UUIDValue roomId, UUIDValue userId, const String& roomName, const RoomInfo& roomInfo)
	: api(api), sessionId(sessionId), roomId(roomId), userId(userId), roomName(roomName), roomInfo(roomInfo) {}

const map<UUIDValue, SyncClient::User>& SyncClient::getUsers() const {
	return users;
}

const SyncClient::User& SyncClient::getMe() const {
	return users.at(userId);
}

bool SyncClient::isOwner() const {
	if (users.empty()) return false;
	return userId == users.begin()->first;
}

bool SyncClient::isSyncing() const {
	return syncTask.isValid();
}

Optional<Error> SyncClient::update() {
	try {
		if (!syncTask.isValid() && timer.reachedZero()) startSync();
		else if (syncTask.isReady()) finishSync();
		return none;
	}
	catch (const Error& error) {
		return error;
	}
}

void SyncClient::startSync() {
	if (syncTask.isValid()) throw Error(U"Already syncing");
	if (!timer.reachedZero()) throw Error(U"Too early to sync");
	syncTask = api->send(APIClient::HTTPMethod::POST, U"/room/sync", {
		{ U"session_id", sessionId.str() },
		{ U"room_info", roomInfo.toJSON() },
		{ U"reports", U"[]"_json }, // TODO
		{ U"actions", U"[]"_json }, // TODO
	});
}

UUIDValue SyncClient::finishSync() {
	if (!syncTask.isValid()) throw Error(U"Not syncing");
	const JSON response = syncTask.get();
	timer.restart();
	for (const auto& [i, reportJSON] : response[U"reports"]) {
		// TODO
	}
	for (const auto& [i, actionJSON] : response[U"actions"]) {
		// TODO
	}
	for (const auto& [i, userJSON] : response[U"room_users"]) {
		const User user{
			.id = *UUIDValue::Parse(userJSON[U"id"].getString()),
			.name = userJSON[U"name"].getString(),
		};
		users.emplace(user.id, user);
	}
	return *UUIDValue::Parse(response[U"id"].getString());
}
