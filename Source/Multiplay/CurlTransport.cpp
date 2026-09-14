# include "CurlTransport.hpp"
# define CURL_STATICLIB
# include <curl/curl.h>

namespace Multiplay
{
	namespace
	{
		using Result = std::expected<HTTPResult, TransportError>;

		struct CurlJob
		{
			HTTPRequest request;

			std::promise<Result> promise;

			std::atomic_bool abandoned = false;
		};

		/// @brief 通信スレッドだけが触る、1 リクエスト分の libcurl の状態
		struct Transfer
		{
			std::shared_ptr<CurlJob> job;

			::curl_slist* headers = nullptr;

			std::string url;

			std::string response;

			Stopwatch stopwatch{ StartImmediately::No };
		};

		size_t WriteResponse(const char* data, const size_t size, const size_t count, void* userdata)
		{
			static_cast<std::string*>(userdata)->append(data, (size * count));
			return (size * count);
		}
	}

	class detail::CurlWorker
	{
	public:

		CurlWorker()
		{
			if (::curl_global_init(CURL_GLOBAL_ALL) != ::CURLE_OK)
			{
				throw Error{ U"curl_global_init() failed." };
			}

			m_multi = ::curl_multi_init();

			if (not m_multi)
			{
				::curl_global_cleanup();
				throw Error{ U"curl_multi_init() failed." };
			}

			m_thread = std::thread{ [this] { run(); } };
		}

		CurlWorker(const CurlWorker&) = delete;

		CurlWorker& operator =(const CurlWorker&) = delete;

		/// @remark 送信中のリクエストは中断する
		~CurlWorker()
		{
			{
				const std::lock_guard lock{ m_mutex };
				m_stopping = true;
			}

			wakeup();
			m_thread.join();
			::curl_multi_cleanup(m_multi);
			::curl_global_cleanup();
		}

		void submit(std::shared_ptr<CurlJob> job)
		{
			{
				const std::lock_guard lock{ m_mutex };
				m_submitted << std::move(job);
			}

			wakeup();
		}

		void wakeup()
		{
			::curl_multi_wakeup(m_multi);
		}

	private:

		::CURLM* m_multi = nullptr;

		std::mutex m_mutex;

		Array<std::shared_ptr<CurlJob>> m_submitted;

		bool m_stopping = false;

		std::thread m_thread;

		void run()
		{
			// 接続は multi handle に残るため、easy handle はリクエストごとに作って捨ててよい。
			std::unordered_map<::CURL*, Transfer> transfers;

			while (true)
			{
				Array<std::shared_ptr<CurlJob>> submitted;
				{
					const std::lock_guard lock{ m_mutex };

					if (m_stopping)
					{
						break;
					}

					submitted.swap(m_submitted);
				}

				for (auto& job : submitted)
				{
					start(transfers, std::move(job));
				}

				std::erase_if(transfers, [this](auto& entry)
				{
					if (not entry.second.job->abandoned)
					{
						return false;
					}

					close(entry.first, entry.second);
					return true;
				});

				int running = 0;
				::curl_multi_perform(m_multi, &running);

				int left = 0;
				while (const ::CURLMsg* message = ::curl_multi_info_read(m_multi, &left))
				{
					if (message->msg != ::CURLMSG_DONE)
					{
						continue;
					}

					const ::CURLcode code = message->data.result;
					auto node = transfers.extract(message->easy_handle);

					if (not node.empty())
					{
						complete(node.key(), node.mapped(), code);
					}
				}

				::curl_multi_poll(m_multi, nullptr, 0, 1000, nullptr);
			}

			for (auto& [easy, transfer] : transfers)
			{
				close(easy, transfer);
			}
		}

		void start(std::unordered_map<::CURL*, Transfer>& transfers, std::shared_ptr<CurlJob> job)
		{
			::CURL* easy = ::curl_easy_init();

			if (not easy)
			{
				job->promise.set_value(std::unexpected{ TransportError::Network });
				return;
			}

			Transfer& transfer = transfers.emplace(easy, Transfer{ .job = std::move(job) }).first->second;
			const HTTPRequest& request = transfer.job->request;

			for (const auto& [name, value] : request.headers)
			{
				transfer.headers = ::curl_slist_append(transfer.headers, (name + U": " + value).toUTF8().c_str());
			}

			transfer.url = request.url.toUTF8();
			::curl_easy_setopt(easy, ::CURLOPT_URL, transfer.url.c_str());
			::curl_easy_setopt(easy, ::CURLOPT_HTTPHEADER, transfer.headers);
			::curl_easy_setopt(easy, ::CURLOPT_PROTOCOLS, static_cast<long>(CURLPROTO_HTTP | CURLPROTO_HTTPS));
			::curl_easy_setopt(easy, ::CURLOPT_TIMEOUT_MS, static_cast<long>(request.timeout.count() * 1000));
			::curl_easy_setopt(easy, ::CURLOPT_WRITEFUNCTION, WriteResponse);
			::curl_easy_setopt(easy, ::CURLOPT_WRITEDATA, &transfer.response);

			if (request.method == HTTPMethod::Post)
			{
				::curl_easy_setopt(easy, ::CURLOPT_POST, 1L);
				::curl_easy_setopt(easy, ::CURLOPT_POSTFIELDS, request.body.data());
				::curl_easy_setopt(easy, ::CURLOPT_POSTFIELDSIZE_LARGE, static_cast<::curl_off_t>(request.body.size()));
			}

			::curl_multi_add_handle(m_multi, easy);
			transfer.stopwatch.restart();
		}

		void complete(::CURL* easy, Transfer& transfer, const ::CURLcode code)
		{
			const Duration elapsed = transfer.stopwatch.elapsed();
			long status = 0;
			::curl_easy_getinfo(easy, ::CURLINFO_RESPONSE_CODE, &status);
			long connects = 0;
			::curl_easy_getinfo(easy, ::CURLINFO_NUM_CONNECTS, &connects);
			close(easy, transfer);

			if (code != ::CURLE_OK)
			{
				transfer.job->promise.set_value(std::unexpected{ (code == ::CURLE_OPERATION_TIMEDOUT) ? TransportError::Timeout : TransportError::Network });
				return;
			}

			transfer.job->promise.set_value(HTTPResult{
				.status = static_cast<HTTPStatusCode>(status),
				.body = Blob{ transfer.response.data(), transfer.response.size() },
				.stats = { .elapsed = elapsed, .newConnection = (0 < connects) },
			});
		}

		void close(::CURL* easy, Transfer& transfer)
		{
			::curl_multi_remove_handle(m_multi, easy);
			::curl_easy_cleanup(easy);
			::curl_slist_free_all(transfer.headers);
			transfer.headers = nullptr;
		}
	};

	namespace
	{
		class CurlCall final : public IHTTPCall
		{
		public:

			CurlCall(std::shared_ptr<detail::CurlWorker> worker, std::shared_ptr<CurlJob> job)
				: m_worker{ std::move(worker) }
				, m_job{ std::move(job) }
				, m_result{ m_job->promise.get_future() } {}

			CurlCall(const CurlCall&) = delete;

			CurlCall& operator =(const CurlCall&) = delete;

			~CurlCall() override
			{
				m_job->abandoned = true;
				m_worker->wakeup();
			}

			bool isReady() override
			{
				return (m_result.wait_for(0s) == std::future_status::ready);
			}

			Result get() override
			{
				return m_result.get();
			}

		private:

			std::shared_ptr<detail::CurlWorker> m_worker;

			std::shared_ptr<CurlJob> m_job;

			std::future<Result> m_result;
		};
	}

	CurlTransport::CurlTransport()
		: m_worker{ std::make_shared<detail::CurlWorker>() } {}

	std::unique_ptr<IHTTPCall> CurlTransport::send(HTTPRequest request)
	{
		auto job = std::make_shared<CurlJob>(std::move(request));
		auto call = std::make_unique<CurlCall>(m_worker, job);
		m_worker->submit(std::move(job));
		return call;
	}
}
