# pragma once

# include <Siv3D.hpp>

class APIClient {
public:
	enum class HTTPMethod { GET, POST };

	class HTTPError : public Error {
	public:
		const HTTPStatusCode statusCode;

		[[nodiscard]] explicit HTTPError(HTTPStatusCode statusCode, const String& message);
	};

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

	/**
	 * @param method HTTP メソッド (GET または POST)
	 * @param path リクエストのパス (e.g. U"/")
	 * @param data リクエストのボディ (POST の場合のみ)
	 * @return レスポンスの JSON の AsyncTask
	 * @throw HTTPError 共通にサーバーが返しうるエラーは { 400: パラメータが不正, 404: API が無効, 500: サーバーエラー }
	 * @throw Error HTTP 通信に失敗した場合
	 */
	AsyncTask<JSON> send(HTTPMethod method, const String& path, const JSON& data = JSON::Invalid()) const;

	/**
	 * @brief サーバーのステータスを取得する。エラーは send 参照
	 * @return サーバーのステータスの AsyncTask
	 */
	[[nodiscard]] AsyncTask<ServerStatus> fetchServerStatus() const;
};
