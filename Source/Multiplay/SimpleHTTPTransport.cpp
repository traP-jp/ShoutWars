# include "SimpleHTTPTransport.hpp"
# include <Siv3D/Windows/Windows.hpp>

namespace Multiplay
{
	struct detail::InFlightRequests
	{
		std::mutex mutex;

		std::condition_variable finished;

		size_t count = 0;
	};

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

			return HTTPResult{ .status = response.getStatusCode(), .body = writer.retrieve(), .stats = { .elapsed = elapsed, .newConnection = true } };
		}

		/// @brief 送信中の数を数える。リクエストの寿命と合わせて増減させる
		class InFlightToken
		{
		public:

			explicit InFlightToken(std::shared_ptr<detail::InFlightRequests> requests)
				: m_requests{ std::move(requests) }
			{
				const std::lock_guard lock{ m_requests->mutex };
				++m_requests->count;
			}

			InFlightToken(const InFlightToken&) = delete;

			InFlightToken& operator =(const InFlightToken&) = delete;

			~InFlightToken()
			{
				const std::lock_guard lock{ m_requests->mutex };
				--m_requests->count;
				m_requests->finished.notify_all();
			}

		private:

			std::shared_ptr<detail::InFlightRequests> m_requests;
		};

		struct Job
		{
			HTTPRequest request;

			std::promise<Result> promise;

			InFlightToken token;
		};

		void CALLBACK RunJob(PTP_CALLBACK_INSTANCE, void* context)
		{
			const std::unique_ptr<Job> job{ static_cast<Job*>(context) };
			job->promise.set_value(Send(job->request));
		}

		class SimpleHTTPCall final : public IHTTPCall
		{
		public:

			SimpleHTTPCall(HTTPRequest request, std::shared_ptr<detail::InFlightRequests> inFlight)
				: m_timeout{ request.timeout }
			{
				std::unique_ptr<Job> job{ new Job{ std::move(request), std::promise<Result>{}, InFlightToken{ std::move(inFlight) } } };
				m_result = job->promise.get_future();

				// 毎回新しいスレッドを作ると往復時間が増えたため、AsyncHTTPTask (std::async) と同じくスレッドプールで使い回す。
				// 同期版の SimpleHTTP は中断できないため、タイムアウトや破棄で見捨てたリクエストも最後まで走らせる。
				// 終了時の競合はトランスポートの破棄で待って防ぐ。
				if (not ::TrySubmitThreadpoolCallback(RunJob, job.get(), nullptr))
				{
					throw Error{ U"Cannot submit an HTTP request to the thread pool." };
				}

				job.release();
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

	SimpleHTTPTransport::SimpleHTTPTransport()
		: m_inFlight{ std::make_shared<detail::InFlightRequests>() } {}

	SimpleHTTPTransport::~SimpleHTTPTransport()
	{
		// DEBT: 3 秒で終わらないリクエストがあると、終了処理と競合して落ちうる。libcurl 版で中断できるようにする
		std::unique_lock lock{ m_inFlight->mutex };
		m_inFlight->finished.wait_for(lock, 3s, [&] { return (m_inFlight->count == 0); });
	}

	std::unique_ptr<IHTTPCall> SimpleHTTPTransport::send(HTTPRequest request)
	{
		return std::make_unique<SimpleHTTPCall>(std::move(request), m_inFlight);
	}
}
