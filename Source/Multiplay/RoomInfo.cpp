# include "RoomInfo.hpp"

RoomInfo::RoomInfo() = default;

RoomInfo::RoomInfo(const JSON& json) {
	character = json[U"character"].getString();
	room_timer = json[U"room_timer"].get<int>();
}

JSON RoomInfo::toJSON() const {
	JSON json;
	json[U"character"] = character;
	json[U"room_timer"] = room_timer;
	return json;
}
