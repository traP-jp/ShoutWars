# pragma once
# include "HTTPTransport.hpp"

namespace Multiplay
{
	class SimpleHTTPTransport final : public IHTTPTransport
	{
	public:

		[[nodiscard]]
		std::unique_ptr<IHTTPCall> send(HTTPRequest request) override;
	};
}
