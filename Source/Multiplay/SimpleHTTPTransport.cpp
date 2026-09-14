# include "SimpleHTTPTransport.hpp"

namespace Multiplay
{
	namespace
	{
		class SimpleHTTPCall final : public IHTTPCall
		{
		public:

			explicit SimpleHTTPCall(const HTTPRequest& request)
				: m_task{ (request.method == HTTPMethod::Get)
					? SimpleHTTP::GetAsync(request.url, request.headers)
					: SimpleHTTP::PostAsync(request.url, request.headers, request.body.data(), request.body.size()) }
				, m_timeout{ request.timeout } {}

			bool isReady() override
			{
				// SimpleHTTP はタイムアウトを設定できないため、期限を過ぎたら中断する。
				if ((not m_task.isReady()) && (m_timeout <= m_stopwatch.elapsed()))
				{
					m_task.cancel();
				}

				return m_task.isReady();
			}

			std::expected<HTTPResult, TransportError> get() override
			{
				if (m_task.isCanceled())
				{
					return std::unexpected{ TransportError::Timeout };
				}

				if (not m_task.isSucceeded())
				{
					return std::unexpected{ TransportError::Network };
				}

				return HTTPResult{ .status = m_task.getResponse().getStatusCode(), .body = m_task.getBlob() };
			}

		private:

			AsyncHTTPTask m_task;

			Duration m_timeout;

			Stopwatch m_stopwatch{ StartImmediately::Yes };
		};
	}

	std::unique_ptr<IHTTPCall> SimpleHTTPTransport::send(HTTPRequest request)
	{
		return std::make_unique<SimpleHTTPCall>(request);
	}
}
