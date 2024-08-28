# pragma once

# include <Siv3D.hpp>

class APIClient {
public:
	struct ServerStatus {
		const MillisecondsF ping;
		const int32 roomCount;
		const int32 roomLimit;
	};

	const String version;
	const URL url;
	const String password;

	/**
	 * @param version クライアントのバージョン
	 * @param url API の URL (e.g. U"https://shoutwars.trap.games/api/v0")
	 * @param password API のパスワード (未設定の場合は省略)
	 */
	[[nodiscard]] explicit APIClient(const String& version, const URL& url, const String& password = U"");

	[[nodiscard]] AsyncTask<ServerStatus> fetchServerStatus() const;

protected:
	enum class HTTPMethod { GET, POST };

	class HTTPError : public Error {
	public:
		const HTTPStatusCode statusCode;

		[[nodiscard]] explicit HTTPError(HTTPStatusCode statusCode, const String& message);
	};

	AsyncTask<JSON> send(HTTPMethod method, const String& path, const JSON& data = JSON::Invalid()) const;
};
