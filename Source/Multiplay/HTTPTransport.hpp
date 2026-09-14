# pragma once
# include <Siv3D.hpp>
# include <expected>

namespace Multiplay
{
	enum class HTTPMethod
	{
		Get,
		Post,
	};

	struct HTTPRequest
	{
		HTTPMethod method = HTTPMethod::Get;

		URL url;

		HashTable<String, String> headers;

		Blob body;

		Duration timeout;
	};

	struct HTTPResult
	{
		HTTPStatusCode status = HTTPStatusCode::Invalid;

		Blob body;
	};

	enum class TransportError
	{
		Timeout,
		Network,
	};

	/// @brief 送信中の 1 リクエスト。破棄すると通信を中断する
	class IHTTPCall
	{
	public:

		virtual ~IHTTPCall() = default;

		[[nodiscard]]
		virtual bool isReady() = 0;

		/// @remark isReady() が true を返した後に 1 回だけ呼べる
		[[nodiscard]]
		virtual std::expected<HTTPResult, TransportError> get() = 0;
	};

	class IHTTPTransport
	{
	public:

		virtual ~IHTTPTransport() = default;

		[[nodiscard]]
		virtual std::unique_ptr<IHTTPCall> send(HTTPRequest request) = 0;
	};
}
