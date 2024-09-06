# include "RoomInfo.hpp"

RoomInfo::RoomInfo() = default;

RoomInfo::RoomInfo(const JSON& json) {
	//player一覧を取得
	for (const auto& object : json[U"member"].arrayView()) {
		player.push_back(object[U"player"].getString());
		character.push_back(object[U"character"].get<int32>());
		is_ready.push_back(object[U"is_ready"].get<bool>());
	}
}

JSON RoomInfo::toJSON() const {
	JSON json;
	Array<JSON> members;
	for (size_t i = 0; i < player.size(); ++i) {
		JSON member;
		member[U"player"] = player.at(i);
		member[U"character"] = character.at(i);
		member[U"is_ready"] = is_ready.at(i);
		members.push_back(member);
	}
	json[U"member"] = members;
	return json;
}
