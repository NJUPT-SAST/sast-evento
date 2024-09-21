#pragma once

#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url/url.hpp>
#include <chrono>
#include <string>

namespace evento {

namespace beast = boost::beast;   // from <boost/beast.hpp>
namespace http = beast::http;     // from <boost/beast/http.hpp>
namespace net = boost::asio;      // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl; // from <boost/asio/ssl.hpp>
namespace urls = boost::urls;     // from <boost/urls.hpp>

template<typename T>
using Task = net::awaitable<T>;
using ResponseResult = Result<http::response<http::dynamic_body>>;

class HttpsAccessManager {
    using executor_with_default = net::use_awaitable_t<>::executor_with_default<net::any_io_executor>;
    using tcp_stream = typename beast::tcp_stream::rebind_executor<executor_with_default>::other;
    using tcp = boost::asio::ip::tcp;

public:
    HttpsAccessManager(net::ssl::context& ctx,
                       bool ignoreSslError = false,
                       std::chrono::seconds timeout = std::chrono::seconds(5))
        : _ctx(ctx)
        , ignoreSslError(ignoreSslError)
        , _timeout(timeout) {}

    // async send request to host and return response
    // `req.prepare_payload()` is called in the function
    Task<ResponseResult> makeReply(std::string host, http::request<http::string_body> req);

    bool ignoreSslError = false;

private:
    net::ssl::context& _ctx;
    std::chrono::seconds _timeout; // respective timeout of ssl handshake & http
};

} // namespace evento