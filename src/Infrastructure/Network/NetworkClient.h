#pragma once

#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>
#include <nlohmann/json.hpp>

namespace evento {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace urls = boost::urls;   // from <boost/url.hpp>

class NetworkAccessManager;

using JsonResult = Result<nlohmann::basic_json<>>;
template<typename T>
using Task = net::awaitable<T>;

class NetworkClient {
public:
    NetworkClient(net::ssl::context& ctx);
    ~NetworkClient();

    Task<Result<LoginResEntity>> loginViaSastLink(const std::string& code);

private:
    // evento client help functions
    // url builder
    static urls::url endpoint(std::string_view endpoint);
    static urls::url endpoint(std::string_view endpoint, urls::params_view params);
    // response handler
    static JsonResult handleResponse(http::response<http::dynamic_body> response);
    // http verbs
    Task<JsonResult> get(urls::url_view url);

    Task<JsonResult> post(urls::url_view url, urls::params_view params);
    Task<JsonResult> post(urls::url_view url, const nlohmann::json& body);

    Task<JsonResult> put(urls::url_view url);
    Task<JsonResult> put(urls::url_view url, const nlohmann::json& body);

    [[maybe_unused]] Task<JsonResult> patch(urls::url_view url, const nlohmann::json& body);

    Task<JsonResult> deleteResource(urls::url_view url);

    // github client help functions
    Task<JsonResult> getFromGithub(urls::url_view url);

private:
    net::ssl::context& _ctx;
    std::unique_ptr<NetworkAccessManager> _manager;
};

} // namespace evento