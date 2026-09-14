# include "ServerConfig.hpp"
# include "Multiplay/CurlTransport.hpp"

namespace
{
	constexpr StringView DefaultServerURL = U"https://shoutwars.trap.games/api";

	[[nodiscard]]
	Optional<String> ReadString(const JSON& server, const StringView key)
	{
		if (not server.contains(key))
		{
			return none;
		}

		if (not server[key].isString())
		{
			throw Error{ U"server.{} in the config file must be a string."_fmt(key) };
		}

		return server[key].getString();
	}
}

ServerConfig LoadServerConfig(const FilePathView configPath, const StringView version)
{
	const JSON config = JSON::Load(configPath);

	Optional<String> url;
	Optional<String> password;
	Optional<String> syncLogDirectory;

	if (config.isObject() && config.contains(U"server"))
	{
		const JSON server = config[U"server"];

		if (not server.isObject())
		{
			throw Error{ U"server in the config file must be an object." };
		}

		url = ReadString(server, U"url");
		password = ReadString(server, U"password");
		syncLogDirectory = ReadString(server, U"syncLogDirectory");
	}

	return ServerConfig{
		.api = Multiplay::APIClient{ std::make_shared<Multiplay::CurlTransport>(), url.value_or(String{ DefaultServerURL }), String{ version }, password.value_or(U"") },
		.syncLogDirectory = syncLogDirectory.value_or(U""),
	};
}
