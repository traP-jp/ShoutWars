# include "APIClient.hpp"

APIClient::HTTPError::HTTPError(HTTPStatusCode statusCode, const String& message)
	: Error(message), statusCode(statusCode) {}

APIClient::APIClient(const String& version, const URL& url, const String& password)
	: version(version), url(url), password(password) {}

AsyncTask<JSON> APIClient::send(HTTPMethod method, const String& path, const JSON& data) const {
	return Async([=] {
		HashTable<String, String> headers = { { U"Content-Type", U"application/msgpack" } };
		if (!password.isEmpty()) headers[U"Authorization"] = U"Bearer " + password;
		HTTPResponse response;
		MemoryWriter responseWriter;
		switch (method) {
		case HTTPMethod::GET:
			if (data) throw Error(U"GET request must not have a body");
			response = SimpleHTTP::Get(url + path, headers, responseWriter);
			break;
		case HTTPMethod::POST:
			const Blob body = (data ? data : JSON(nullptr)).toMessagePack();
			response = SimpleHTTP::Post(url + path, headers, body.data(), body.size(), responseWriter);
			break;
		}
		const JSON responseData = JSON::FromMessagePack(responseWriter.getBlob());
		if (!response.isOK()) {
			throw HTTPError(response.getStatusCode(), responseData[U"error"].getString());
		}
		return responseData;
	});
}

AsyncTask<APIClient::ServerStatus> APIClient::fetchServerStatus() const {
	return Async([=] {
		Stopwatch stopwatch{ StartImmediately::Yes };
		const JSON response = send(HTTPMethod::GET, U"/status").get();
		return ServerStatus{
			.ping = stopwatch.elapsed(),
			.roomCount = response[U"room_count"].get<int32>(),
			.roomLimit = response[U"room_limit"].get<int32>(),
		};
	});
}
