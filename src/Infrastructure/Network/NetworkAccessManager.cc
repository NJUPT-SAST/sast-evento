#include "NetworkAccessManager.h"
#include "Infrastructure/Utils/Result.h"
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>

namespace evento {

constexpr const char USER_AGENT[] = "SAST-Evento-Desktop/1";

Task<ResponseResult> NetworkAccessManager::request(std::string_view host,
                                                   http::request<http::string_body> req) {
    auto resolver = net::use_awaitable_t<boost::asio::any_io_executor>::as_default_on(
        tcp::resolver(co_await net::this_coro::executor));

    // We construct the ssl stream from the already rebound tcp_stream.
    beast::ssl_stream<tcp_stream>
        stream{net::use_awaitable_t<boost::asio::any_io_executor>::as_default_on(
                   beast::tcp_stream(co_await net::this_coro::executor)),
               _ctx};

    ASYNC_VOID_TRY(_sslHandshake(std::move(stream), host));

    req.set(http::field::host, host);
    req.set(http::field::user_agent, USER_AGENT);

    // Set token if exits
    if (tokenBytes)
        req.set("TOKEN", *tokenBytes);

    req.prepare_payload();

    // Set the timeout
    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, req);

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    co_await http::async_read(stream, _buffer, res);

    auto sslRes = co_await _sslDisconnect(std::move(stream));

    if (!ignoreSslError && sslRes.isErr()) {
        co_return sslRes.unwrapErr();
    }

    co_return res;
}

Task<ResponseResult> NetworkAccessManager::get(urls::url_view url, std::string_view acceptType) {
    http::request<http::string_body> req{http::verb::get, url.path(), 11};
    req.set(http::field::accept, acceptType);
    return request(url.host(), req);
}

Task<ResponseResult> NetworkAccessManager::post(urls::url_view url,
                                                std::string_view acceptType,
                                                std::string_view contentType,
                                                const std::string& body) {
    http::request<http::string_body> req{http::verb::post, url.path(), 11};
    req.set(http::field::content_type, contentType);
    req.set(http::field::accept, acceptType);
    req.body() = body;
    return request(url.host(), req);
}

Task<ResponseResult> NetworkAccessManager::put(urls::url_view url,
                                               std::string_view acceptType,
                                               std::string_view contentType,
                                               const std::string& body) {
    http::request<http::string_body> req{http::verb::put, url.path(), 11};
    req.set(http::field::content_type, contentType);
    req.set(http::field::accept, acceptType);
    req.body() = body;
    return request(url.host(), req);
}

Task<ResponseResult> NetworkAccessManager::patch(urls::url_view url,
                                                 std::string_view acceptType,
                                                 std::string_view contentType,
                                                 const std::string& body) {
    http::request<http::string_body> req{http::verb::patch, url.path(), 11};
    req.set(http::field::content_type, contentType);
    req.set(http::field::accept, acceptType);
    req.body() = body;
    return request(url.host(), req);
}

Task<ResponseResult> NetworkAccessManager::deleteResource(urls::url_view url,
                                                          std::string_view acceptType) {
    http::request<http::string_body> req{http::verb::delete_, url.path(), 11};
    req.set(http::field::accept, acceptType);
    return request(url.host(), req);
}

Task<Result<void>> NetworkAccessManager::_sslHandshake(beast::ssl_stream<tcp_stream>&& stream,
                                                       std::string_view host) {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.data()))
        co_return Err(
            Error(Error::Ssl,
                  net::error::get_ssl_category().message(static_cast<int>(::ERR_get_error()))));

    // Look up the domain name
    auto resolver = net::use_awaitable_t<boost::asio::any_io_executor>::as_default_on(
        tcp::resolver(co_await net::this_coro::executor));
    auto const results = co_await resolver.async_resolve(host, "https");

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Make the connection on the IP address we get from a lookup
    co_await beast::get_lowest_layer(stream).async_connect(results);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Perform the SSL handshake
    co_await stream.async_handshake(ssl::stream_base::client);

    co_return Ok();
}

Task<Result<void>> NetworkAccessManager::_sslDisconnect(beast::ssl_stream<tcp_stream>&& stream) {
    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Gracefully close the stream - do not threat every error as an exception!
    auto [ec] = co_await stream.async_shutdown(net::as_tuple(net::use_awaitable));
    if (ec == net::error::eof)
        // If we get here then the connection is closed gracefully
        co_return Ok();

    co_return Err(Error(Error::Ssl, ec.message()));
}

} // namespace evento
