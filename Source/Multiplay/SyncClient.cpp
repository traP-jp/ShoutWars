# include "SyncClient.hpp"

using namespace std;

UUIDValue SyncClient::genUUIDv7() {
	// ref: https://www.boost.org/doc/libs/1_86_0/boost/uuid/time_generator_v7.hpp

	thread_local uint64 oldState = 0;
	const uint64 nowUs = Time::GetMicrosecSinceEpoch();
	uint64 state = ((nowUs / 1000) << 16) | ((nowUs % 1000) << 6);
	if (state <= oldState && nowUs / 1000 >= (oldState >> 16)) state = oldState + 1;
	oldState = state;

	array<uint8, 16> uuid;
	const uint64 random = Random(UINT64_MAX);
	memcpy(uuid.data() + 8, &random, sizeof(uint64));
	const uint64 timestamp_big_endian = byteswap(((state >> 16) << 16) | 0x7000 | ((state & 0xFFFF) >> 6));
	memcpy(uuid.data(), &timestamp_big_endian, sizeof(uint64));
	uuid[8] = 0x80 | (state & 0x3F);
	return UUIDValue{ uuid };
}

AsyncTask<unique_ptr<SyncClient>> SyncClient::createRoom(shared_ptr<APIClient> api, const String& userName, uint32 size) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/create", {
			{ U"version", api->version },
			{ U"user", { { U"name", userName } } },
			{ U"size", size },
		}).get();
		auto client = make_unique<SyncClient>(
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

AsyncTask<unique_ptr<SyncClient>> SyncClient::joinRoom(shared_ptr<APIClient> api, const String& roomName, const String& userName) {
	return Async([=] {
		const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/join", {
			{ U"version", api->version },
			{ U"name", roomName },
			{ U"user", { { U"name", userName } } },
		}).get();
		const JSON roomInfoJSON = response[U"room_info"];
		auto client = make_unique<SyncClient>(
			api,
			*UUIDValue::Parse(response[U"session_id"].getString()),
			*UUIDValue::Parse(response[U"id"].getString()),
			*UUIDValue::Parse(response[U"user_id"].getString()),
			roomName,
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

void SyncClient::sendReport(const String& type, const JSON& data) {
	pendingReports << make_unique<Event>(genUUIDv7(), UUIDValue::Nil(), userId, type, data);
}

void SyncClient::sendAction(const String& type, const JSON& data) {
	pendingActions << make_unique<Event>(genUUIDv7(), UUIDValue::Nil(), userId, type, data);
}

unique_ptr<SyncClient::Event> SyncClient::receiveReport() {
	if (receivedReports.empty()) return nullptr;
	auto event = move(receivedReports.front());
	receivedReports.pop_front();
	return event;
}

unique_ptr<SyncClient::Event> SyncClient::receiveAction() {
	if (receivedActions.empty()) return nullptr;
	auto event = move(receivedActions.front());
	receivedActions.pop_front();
	return event;
}

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

void SyncClient::update() {
	if (!isSyncing() && timer.reachedZero()) startSync();
	else if (syncTask.isReady()) finishSync();
}

void SyncClient::sendStart() {
	if (!isOwner()) throw Error(U"Owner only");
	if (isStarting()) throw Error(U"Already sending start");
	startTask = api->send(APIClient::HTTPMethod::POST, U"/room/start", {
		{ U"session_id", sessionId.str() },
	});
}

bool SyncClient::isStarting() const {
	return startTask.isValid();
}

Optional<SyncClient::StartInfo> SyncClient::receiveStart() {
	if (!startTask.isReady()) return none;
	const JSON response = startTask.get();
	return StartInfo{};
}

void SyncClient::startSync() {
	if (syncTask.isValid()) throw Error(U"Already syncing");
	if (!timer.reachedZero()) throw Error(U"Too early to sync");
	Array<JSON> reportsJSON, actionsJSON;
	for (const auto& report : pendingReports) reportsJSON << JSON{
		{ U"id", report->id.str() },
		{ U"type", report->type },
		{ U"event", report->data },
	};
	for (const auto& action : pendingActions) actionsJSON << JSON{
		{ U"id", action->id.str() },
		{ U"type", action->type },
		{ U"event", action->data },
	};
	sentActions.swap(pendingActions);
	sentReports.swap(pendingReports);
	syncTask = Async([=] {
		try {
			const JSON response = api->send(APIClient::HTTPMethod::POST, U"/room/sync", {
				{ U"session_id", sessionId.str() },
				{ U"room_info", roomInfo.toJSON() },
				{ U"reports", reportsJSON },
				{ U"actions", actionsJSON },
			}).get();
			sentReports.clear();
			sentActions.clear();
			return response;
		}
		catch (const Error& error) {
			for (auto& report : sentReports) pendingReports << move(report);
			for (auto& action : sentActions) pendingActions << move(action);
			sentReports.clear();
			sentActions.clear();
			throw error;
		}
	});
}

UUIDValue SyncClient::finishSync() {
	if (!syncTask.isValid()) throw Error(U"Not syncing");
	const JSON response = syncTask.get();
	timer.restart();
	const UUIDValue syncId = *UUIDValue::Parse(response[U"id"].getString());
	for (const auto& [i, reportJSON] : response[U"reports"]) {
		receivedReports << make_unique<Event>(
			*UUIDValue::Parse(reportJSON[U"id"].getString()),
			reportJSON.hasElement(U"sync_id") ? *UUIDValue::Parse(reportJSON[U"sync_id"].getString()) : syncId,
			*UUIDValue::Parse(reportJSON[U"from"].getString()),
			reportJSON[U"type"].getString(),
			JSON::Parse(reportJSON[U"event"].formatMinimum()) // こうする以外の方法が見つからなかった
		);
	}
	for (const auto& [i, actionJSON] : response[U"actions"]) {
		receivedActions << make_unique<Event>(
			*UUIDValue::Parse(actionJSON[U"id"].getString()),
			actionJSON.hasElement(U"sync_id") ? *UUIDValue::Parse(actionJSON[U"sync_id"].getString()) : syncId,
			*UUIDValue::Parse(actionJSON[U"from"].getString()),
			actionJSON[U"type"].getString(),
			JSON::Parse(actionJSON[U"event"].formatMinimum()) // こうする以外の方法が見つからなかった
		);
	}
	users.clear();
	for (const auto& [i, userJSON] : response[U"room_users"]) {
		const User user{
			.id = *UUIDValue::Parse(userJSON[U"id"].getString()),
			.name = userJSON[U"name"].getString(),
		};
		users.emplace(user.id, user);
	}
	return syncId;
}
