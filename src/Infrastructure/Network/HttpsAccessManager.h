#pragma once

#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url/url.hpp>
#include <chrono>
#include <functional> // for std::hash
#include <mutex>
#include <string>
#include <unordered_map>

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
    beast::flat_buffer _buffer;    // http buffer is used for reading and must be persisted

    // Cache for storing downloaded images
    std::unordered_map<std::string, http::response<http::dynamic_body>> _cache;
    std::mutex _cacheMutex;

    // Helper function to save response body to a file
    void saveToFile(const std::string& filename, const beast::flat_buffer& buffer);

    // Helper function to generate a unique filename based on URL
    std::string generateFilename(const std::string& url);
};

} // namespace evento