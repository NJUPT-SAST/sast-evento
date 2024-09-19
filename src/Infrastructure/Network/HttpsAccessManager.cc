#include "HttpsAccessManager.h"
#include <Infrastructure/Utils/Debug.h>
#include <Infrastructure/Utils/Result.h>
#include <boost/asio/as_tuple.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/system/system_error.hpp>
#include <boost/url.hpp>

namespace evento {

constexpr const char USER_AGENT[] = "SAST-Evento-Desktop/1";

Task<ResponseResult> HttpsAccessManager::makeReply(std::string host,
                                                   http::request<http::string_body> req) {
    auto resolver = net::use_awaitable_t<boost::asio::any_io_executor>::as_default_on(
        tcp::resolver(co_await net::this_coro::executor));

    // We construct the ssl stream from the already rebound tcp_stream.
    beast::ssl_stream<tcp_stream>
        stream{boost::asio::use_awaitable_t<boost::asio::any_io_executor>::as_default_on(
                   beast::tcp_stream(co_await net::this_coro::executor)),
               _ctx};

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
        throw boost::system::system_error(static_cast<int>(::ERR_get_error()),
                                          net::error::get_ssl_category());

    // Look up the domain name
    auto const results = co_await resolver.async_resolve(host, "443");

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    co_await beast::get_lowest_layer(stream).async_connect(results);

    // Set the timeout.
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

    // Perform the SSL handshake
    try {
        co_await stream.async_handshake(ssl::stream_base::client);
    } catch (const boost::system::system_error& e) {
        co_return Err(Error(Error::Ssl, e.what()));
    }

    req.prepare_payload();

    // Set the timeout
    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Send the HTTP request to the remote host
    co_await http::async_write(stream, req);

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    beast::flat_buffer buffer;

    // Receive the HTTP response
    co_await http::async_read(stream, buffer, res);

    beast::get_lowest_layer(stream).expires_after(_timeout);

    // Gracefully close the stream - do not threat every error as an exception!
    auto [ec] = co_await stream.async_shutdown(net::as_tuple(net::use_awaitable));
    if (!ec || ec == net::error::eof || (ignoreSslError && ec == ssl::error::stream_truncated))
        // If we get here then the connection is closed gracefully
        co_return Ok(res);

    debug(), ec, ec.message();
    co_return Err(Error(Error::Network, ec.message()));
}

} // namespace evento
