# include "Realm.hpp"

String LoadRealm(FilePathView configPath) {
	const JSON config = JSON::Load(configPath);
	if (!config.isObject() || !config.contains(U"realm")) return U"";
	if (!config[U"realm"].isString()) throw Error{ U"realm in the config file must be a string." };
	return config[U"realm"].getString();
}

void SaveRealm(FilePathView configPath, StringView realm) {
	JSON config = JSON::Load(configPath);
	if (!config || !config.isObject()) config = {};
	if (realm.isEmpty()) config.erase(U"realm");
	else config[U"realm"] = realm;
	config.save(configPath);
}

String GenerateRealm() {
	//読み間違えやすい文字 (0 と o、1 と l など) を除く
	constexpr StringView Letters = U"23456789abcdefghjkmnpqrstuvwxyz";
	String realm;
	for (size_t i = 0; i < 10; ++i) realm << Letters[Random(Letters.size() - 1)];
	return realm;
}
