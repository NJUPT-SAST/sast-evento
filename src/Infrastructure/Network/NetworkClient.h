#pragma once

#include "Infrastructure/Utils/Debug.h"
#include <Infrastructure/Network/Api/Evento.hpp>
#include <Infrastructure/Network/Api/Github.hpp>
#include <Infrastructure/Network/HttpsAccessManager.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>
#include <concepts>
#include <initializer_list>
#include <nlohmann/json.hpp>

namespace evento {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace urls = boost::urls;   // from <boost/url.hpp>

using JsonResult = Result<nlohmann::basic_json<>>;
template<typename T>
using Task = net::awaitable<T>;

class NetworkClient {
public:
    NetworkClient(net::ssl::context& ctx);

    Task<Result<LoginResEntity>> loginViaSastLink(const std::string& code);

    Task<Result<UserInfoEntity>> getUserInfo(const std::string& userId);

    std::optional<std::string> tokenBytes;

private:
    // http request
    template<std::same_as<api::Evento> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        debug(), url;
        auto req = Api::makeRequest(verb, url, tokenBytes, params);

        auto reply = co_await _manager->makeReply(url.host(), req);

        if (reply.isErr())
            co_return reply.unwrapErr();

        co_return handleResponse(reply.unwrap());
    }

    template<std::same_as<api::Github> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        auto req = Api::makeRequest(verb, url, params);
        auto reply = co_await _manager->makeReply(url.host(), req);
        if (reply.isErr())
            co_return reply.unwrapErr();
        co_return reply.unwrap();
    }

    // url builder
    static urls::url endpoint(std::string_view endpoint); // url has no query params
    static urls::url endpoint(std::string_view endpoint,  // url has query params
                              std::initializer_list<urls::param> const& queryParams);
    // response handler
    static JsonResult handleResponse(http::response<http::dynamic_body> response);

private:
    net::ssl::context& _ctx;
    std::unique_ptr<HttpsAccessManager> _manager;
};

} // namespace evento