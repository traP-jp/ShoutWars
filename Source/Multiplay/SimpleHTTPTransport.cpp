# include "SimpleHTTPTransport.hpp"

namespace Multiplay
{
	namespace
	{
		using Result = std::expected<HTTPResult, TransportError>;

		[[nodiscard]]
		Result Send(const HTTPRequest& request)
		{
			MemoryWriter writer;
			const Stopwatch stopwatch{ StartImmediately::Yes };
			const HTTPResponse response = (request.method == HTTPMethod::Get)
				? SimpleHTTP::Get(request.url, request.headers, writer)
				: SimpleHTTP::Post(request.url, request.headers, request.body.data(), request.body.size(), writer);
			const Duration elapsed = stopwatch.elapsed();

			if (response.isInvalid())
			{
				return std::unexpected{ TransportError::Network };
			}

			return HTTPResult{ .status = response.getStatusCode(), .body = writer.retrieve(), .elapsed = elapsed };
		}

		class SimpleHTTPCall final : public IHTTPCall
		{
		public:

			explicit SimpleHTTPCall(HTTPRequest request)
				: m_timeout{ request.timeout }
			{
				std::promise<Result> promise;
				m_result = promise.get_future();

				// 同期版の SimpleHTTP は中断できないため、タイムアウトや破棄で見捨てたスレッドは切り離して最後まで走らせる。
				// DEBT: 送信中にアプリを終了すると、切り離したスレッドが終了処理と競合しうる。libcurl 版で中断できるようにする
				std::thread{ [request = std::move(request), promise = std::move(promise)]() mutable
				{
					promise.set_value(Send(request));
				} }.detach();
			}

			bool isReady() override
			{
				return (isCompleted() || (m_timeout <= m_stopwatch.elapsed()));
			}

			Result get() override
			{
				if (not isCompleted())
				{
					return std::unexpected{ TransportError::Timeout };
				}

				return m_result.get();
			}

		private:

			std::future<Result> m_result;

			Duration m_timeout;

			Stopwatch m_stopwatch{ StartImmediately::Yes };

			[[nodiscard]]
			bool isCompleted() const
			{
				return (m_result.wait_for(0s) == std::future_status::ready);
			}
		};
	}

	std::unique_ptr<IHTTPCall> SimpleHTTPTransport::send(HTTPRequest request)
	{
		return std::make_unique<SimpleHTTPCall>(std::move(request));
	}
}
