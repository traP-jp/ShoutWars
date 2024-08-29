# include "RoomInfo.hpp"

RoomInfo::RoomInfo() = default;

// メンバの初期化は初期化子リストを使ってください。
// 複雑な初期化が必要な場合は protected static 関数を作って噛ませてください。
RoomInfo::RoomInfo(const JSON& json) {}

JSON RoomInfo::toJSON() const {
	return {};
}
