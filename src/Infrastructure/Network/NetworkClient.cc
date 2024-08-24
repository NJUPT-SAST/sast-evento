#include "NetworkClient.h"
#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <Infrastructure/Utils/Debug.h>
#include <boost/url/param.hpp>
#include <boost/url/params_view.hpp>
#include <initializer_list>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>

namespace evento {

static const std::string EVENTO_API_GATEWAY = "https://evento.sast.fun/api";
static const std::string GITHUB_API_GATEWAY = "https://api.github.com/repos";

constexpr const char MIME_JSON[] = "application/json";
constexpr const char MIME_FORM_URL_ENCODED[] = "application/x-www-form-urlencoded";

NetworkClient::NetworkClient(net::ssl::context& ctx)
    : _ctx(ctx)
    , _manager(std::make_unique<HttpsAccessManager>(_ctx, true)) {}

NetworkClient* NetworkClient::getInstance() {
    static ssl::context ctx(ssl::context::sslv23);
    ctx.set_default_verify_paths();
    static NetworkClient s_instance(ctx);
    return &s_instance;
}

Task<Result<LoginResEntity>> NetworkClient::loginViaSastLink(const std::string& code) {
    auto result = co_await this->request<api::Evento>(http::verb::post,
                                                      endpoint("/login/link"),
                                                      {{"code", code}, {"type", "0"}});

    if (result.isErr())
        co_return Err(result.unwrapErr());

    LoginResEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<UserInfoEntity>> NetworkClient::getUserInfo() {
    auto result = co_await this->request<api::Evento>(http::verb::get, endpoint("/v2/user/profile"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    UserInfoEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}
Task<Result<void>> NetworkClient::refreshAccessToken(std::string const& refreshToken) {
    auto result = co_await this->request<api::Evento>(http::verb::post,
                                                      endpoint("/refresh-token"),
                                                      {{"refreshtoken", refreshToken}});
    if (result.isErr())
        co_return Err(result.unwrapErr());

    try {
        this->tokenBytes = result.unwrap()["accessToken"].get<std::string>();
    } catch (const nlohmann::json::exception& e) {
        this->tokenBytes = std::nullopt;
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok();
}

Task<Result<EventQueryRes>> NetworkClient::getActiveEventList() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"active", "true"}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventQueryRes entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventQueryRes>> NetworkClient::getLatestEventList() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"start", "now"}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventQueryRes entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventQueryRes>> NetworkClient::getHistoryEventList(int page, int size) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"page", std::to_string(page)},
                                                                {"size", std::to_string(size)},
                                                                {"end", "now"}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventQueryRes entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventQueryRes>> NetworkClient::getDepartmentEventList(std::string const& larkDepartment,
                                                                  int page,
                                                                  int size) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"page", std::to_string(page)},
                                                                {"size", std::to_string(size)},
                                                                {"larkDepartmentName",
                                                                 larkDepartment}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventQueryRes entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventQueryRes>> NetworkClient::getEventList(std::initializer_list<urls::param> params) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query", params));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventQueryRes entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<AttachmentEntity>> NetworkClient::getAttachment(int eventId) {
    auto result = co_await this->request<api::Evento>(
        http::verb::get, endpoint(std::format("api/v2/client/event/{}/attachments", eventId)));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    AttachmentEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<std::optional<FeedbackEntity>>> NetworkClient::getUserFeedback(int eventId) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::get,
                                             endpoint(std::format("/v2/client/event/{}/feedback",
                                                                  eventId)));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    std::optional<FeedbackEntity> entity = std::nullopt;

    if (result.unwrap().is_null()) {
        co_return Ok(entity);
    }

    try {
        entity = result.unwrap().get<FeedbackEntity>();
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<bool>> NetworkClient::addUserFeedback(int eventId,
                                                  int rating,
                                                  std::string const& content) {
    auto result = co_await this->request<api::Evento>(
        http::verb::post,
        endpoint(std::format("/v2/client/event/{}/feedback", eventId),
                 {{"rating", std::to_string(rating)}, {"content", content}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    co_return Ok(true);
}

Task<Result<bool>> NetworkClient::checkInEvent(int eventId, std::string const& code) {
    auto result = co_await this->request<api::Evento>(
        http::verb::post,
        endpoint(std::format("/v2/client/event/{}/check-in", eventId), {{"code", code}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    if (result.unwrap().is_boolean())
        co_return Ok(result.unwrap().get<bool>());

    co_return Err(Error(Error::Data, "response data type error"));
}

Task<Result<bool>> NetworkClient::subscribeEvent(int eventId, bool subscribe) {
    std::string subscribeStr = subscribe ? "true" : "false";
    auto result = co_await this
                      ->request<api::Evento>(http::verb::post,
                                             endpoint(std::format("/v2/client/event/{}/subscribe",
                                                                  eventId),
                                                      {{"subscribe", subscribeStr}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    if (result.unwrap().is_boolean())
        co_return Ok(result.unwrap().get<bool>());

    co_return Err(Error(Error::Data, "response data type error"));
}

Task<Result<bool>> NetworkClient::subscribeDepartment(std::string const& larkDepartment,
                                                      bool subscribe) {
    std::string subscribeStr = subscribe ? "true" : "false";
    auto result = co_await this
                      ->request<api::Evento>(http::verb::post,
                                             endpoint(std::format("/v2/client/event/{}/subscribe",
                                                                  larkDepartment),
                                                      {{"subscribe", subscribeStr}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    if (result.unwrap().is_boolean())
        co_return Ok(result.unwrap().get<bool>());

    co_return Err(Error(Error::Data, "response data type error"));
}

Task<Result<EventEntityList>> NetworkClient::getParticipatedEvent() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/participated"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventEntityList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventEntityList>> NetworkClient::getSubscribedEvent() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/subscribed"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    EventEntityList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<SlideEntityList>> NetworkClient::getHomeSlide() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/slide"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    SlideEntityList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<SlideEntityList>> NetworkClient::getEventSlide(int eventId) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint(
                                                          std::format("/v2/client/event/{}/slide",
                                                                      eventId)));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    SlideEntityList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<DepartmentEntityList>> NetworkClient::getDepartmentList() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("v2/client/lark/department"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    DepartmentEntityList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

std::string NetworkClient::generateCacheKey(http::verb verb,
                                            const urls::url_view& url,
                                            const std::initializer_list<urls::param>& params) {
    std::string key = std::string(url.data(), url.size()) + "|"
                      + std::to_string(static_cast<int>(verb));
    for (const auto& param : params) {
        key += "|" + std::string(param.key) + "=" + std::string(param.value);
    }
    return key;
}

std::string NetworkClient::generateCacheKey(http::verb verb,
                                            const urls::url_view& url,
                                            const std::initializer_list<urls::param>& params) {
    std::string key = std::string(url.data(), url.size()) + "|"
                      + std::to_string(static_cast<int>(verb));
    for (const auto& param : params) {
        key += "|" + std::string(param.key) + "=" + std::string(param.value);
    }
    return key;
}

urls::url NetworkClient::endpoint(std::string_view endpoint) {
    return urls::url(EVENTO_API_GATEWAY + endpoint.data());
}

urls::url NetworkClient::endpoint(std::string_view endpoint,
                                  std::initializer_list<urls::param> const& params) {
    auto r = urls::url(EVENTO_API_GATEWAY + endpoint.data());
    r.params().append(params.begin(), params.end());
    return r;
}

Task<Result<ContributorList>> NetworkClient::getContributors() {
    auto result = co_await this->request<api::Github>(http::verb::get,
                                                      githubEndpoint(
                                                          "/NJUPT-SAST/sast-evento/contributors"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    ContributorList entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<ReleaseEntity>> NetworkClient::getLatestRelease() {
    auto result = co_await this
                      ->request<api::Github>(http::verb::get,
                                             githubEndpoint(
                                                 "/NJUPT-SAST/sast-evento/releases/latest"));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    ReleaseEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }
    co_return Ok(entity);
}

urls::url NetworkClient::githubEndpoint(std::string_view endpoint) {
    return urls::url(GITHUB_API_GATEWAY + endpoint.data());
}

urls::url NetworkClient::githubEndpoint(std::string_view endpoint,
                                        std::initializer_list<urls::param> const& params) {
    auto r = urls::url(GITHUB_API_GATEWAY + endpoint.data());
    r.params().append(params.begin(), params.end());
    return r;
}

JsonResult NetworkClient::handleResponse(http::response<http::dynamic_body> response) {
    if (response.result() != http::status::ok) {
        return Err(Error(Error::Network, std::to_string(response.result_int())));
    }

    nlohmann::basic_json<> res;
    try {
        res = nlohmann::json::parse(beast::buffers_to_string(response.body().data()));
        debug(), res.dump();
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

NetworkClient* networkClient() {
    return NetworkClient::getInstance();
}

} // namespace evento