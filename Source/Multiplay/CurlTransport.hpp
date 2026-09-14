# pragma once
# include "HTTPTransport.hpp"

namespace Multiplay
{
	namespace detail
	{
		class CurlWorker;
	}

	/// @brief libcurl を直接使い、接続を使い回すトランスポート。通信は専用のスレッドで行う
	class CurlTransport final : public IHTTPTransport
	{
	public:

		CurlTransport();

		[[nodiscard]]
		std::unique_ptr<IHTTPCall> send(HTTPRequest request) override;

	private:

		std::shared_ptr<detail::CurlWorker> m_worker;
	};
}
