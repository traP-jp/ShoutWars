# pragma once
# include "HTTPTransport.hpp"

namespace Multiplay
{
	namespace detail
	{
		struct InFlightRequests;
	}

	class SimpleHTTPTransport final : public IHTTPTransport
	{
	public:

		SimpleHTTPTransport();

		/// @brief 送信中のリクエストが終わるまで待つ (最大 3 秒)。エンジンの終了処理と curl が競合して落ちるのを防ぐ
		~SimpleHTTPTransport() override;

		[[nodiscard]]
		std::unique_ptr<IHTTPCall> send(HTTPRequest request) override;

	private:

		std::shared_ptr<detail::InFlightRequests> m_inFlight;
	};
}
