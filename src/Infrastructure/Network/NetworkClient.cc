#include "NetworkClient.h"
#include <Infrastructure/Network/NetworkAccessManager.h>
#include <Infrastructure/Utils/Debug.h>
#include <boost/url/params_view.hpp>
#include <cassert>
#include <memory>
#include <nlohmann/json.hpp>

namespace evento {

static const std::string EVENTO_API_GATEWAY = "https://evento.sast.fun/api";
static const std::string GITHUB_API_GATEWAY = "https://api.github.com/repo";

constexpr const char MIME_JSON[] = "application/json";
constexpr const char MIME_FORM_URL_ENCODED[] = "application/x-www-form-urlencoded";

NetworkClient::NetworkClient(net::ssl::context& ctx)
    : _ctx(ctx)
    , _manager(std::make_unique<NetworkAccessManager>(_ctx, true)) {}

NetworkClient::~NetworkClient() = default;

Task<Result<LoginResEntity>> NetworkClient::loginViaSastLink(const std::string& code) {
    auto result = co_await this->post(endpoint("/login/link"), {{"code", code}, {"type", "0"}});

    if (result.isErr())
        co_return Err(result.unwrapErr());

    LoginResEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    debug(), entity;

    co_return Ok(entity);
}

urls::url NetworkClient::endpoint(std::string_view endpoint) {
    return urls::url(EVENTO_API_GATEWAY + endpoint.data());
}

urls::url NetworkClient::endpoint(std::string_view endpoint, urls::params_view params) {
    auto r = urls::url(EVENTO_API_GATEWAY + endpoint.data());
    r.params().append(params.begin(), params.end());
    return r;
}

JsonResult NetworkClient::handleResponse(http::response<http::dynamic_body> response) {
    if (response.result() != http::status::ok) {
        return Err(Error(Error::Network, std::to_string(response.result_int())));
    }

    nlohmann::basic_json<> res;
    debug(), res.dump();
    try {
        res = nlohmann::json::parse(beast::buffers_to_string(response.body().data()));
    } catch (const nlohmann::json::parse_error& e) {
        return Err(Error(Error::JsonDes, e.what()));
    }

    auto err = Error(Error::Data);

    if (!res.contains("success")) {
        return Err(err);
    }

    if (!res["success"].is_boolean() || !res["success"].get<bool>()) {
        if (res.contains("message")) {
            return Err(Error(Error::Data, res["message"].get<std::string>()));
        }
        return Err(err);
    }

    // Here we are sure that the data is valid
    assert(res["success"].get<bool>());

    auto data = res.contains("data") ? res["data"] : nlohmann::json::object();

    return Ok(data);
}

Task<JsonResult> NetworkClient::get(urls::url_view url) {
    auto result = co_await _manager->get(url, MIME_JSON);

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::post(urls::url_view url, urls::params_view params) {
    auto result = co_await _manager->post(url,
                                          MIME_JSON,
                                          MIME_FORM_URL_ENCODED,
                                          params.buffer().data());

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::post(urls::url_view url, const nlohmann::json& body) {
    auto result = co_await _manager->post(url, MIME_JSON, MIME_JSON, body.dump());

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::put(urls::url_view url) {
    auto result = co_await _manager->put(url, MIME_JSON, MIME_JSON, "");

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::put(urls::url_view url, const nlohmann::json& body) {
    auto result = co_await _manager->put(url, MIME_JSON, MIME_JSON, body.dump());

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::deleteResource(urls::url_view url) {
    auto result = co_await _manager->deleteResource(url, MIME_JSON);

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::patch(urls::url_view url, const nlohmann::json& body) {
    auto result = co_await _manager->patch(url, MIME_JSON, MIME_JSON, body.dump());

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    co_return handleResponse(response);
}

Task<JsonResult> NetworkClient::getFromGithub(urls::url_view url) {
    auto result = co_await _manager->get(url, "application/vnd.github+json");

    if (result.isErr())
        co_return result.unwrapErr();

    auto response = result.unwrap();

    // FIXME: handle github response
    co_return handleResponse(response);
}

} // namespace evento