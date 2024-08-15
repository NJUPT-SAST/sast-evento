#include "NetworkClient.h"
#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Utils/Debug.h>
#include <boost/url/param.hpp>
#include <boost/url/params_view.hpp>
#include <memory>
#include <nlohmann/json.hpp>

namespace evento {

static const std::string EVENTO_API_GATEWAY = "https://evento.sast.fun/api";
static const std::string GITHUB_API_GATEWAY = "https://api.github.com/repo";

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

    UserInfoEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<EventEntityList>> NetworkClient::getActiveEventList() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/active"));
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

Task<Result<EventEntityList>> NetworkClient::getLatestEventList() {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/latest"));
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

Task<Result<EventEntityList>> NetworkClient::getHistoryEventList(int page, int size) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/history",
                                                               {{"page", page}, {"size", size}}));
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

Task<Result<FeedbackEntity>> NetworkClient::getUserFeedback(int eventId) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::get,
                                             endpoint(std::format("/v2/client/event/{}/feedback",
                                                                  eventId)));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    FeedbackEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<void>> NetworkClient::addUserFeedback(int eventId,
                                                  int rating,
                                                  std::string const& content) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::post,
                                             endpoint(std::format("/v2/client/event/{}/feedback",
                                                                  eventId),
                                                      {{"rating", rating}, {"content", content}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());
    FeedbackEntity entity;
    try {
        nlohmann::from_json(result.unwrap(), entity);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(entity);
}

Task<Result<void>> NetworkClient::checkInEvent(int eventId, std::string const& code) {
    auto result = co_await this->request<api::Evento>(
        http::verb::post,
        endpoint(std::format("/v2/client/event/{}/check-in", eventId), {{"code", code}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    co_return Ok();
}

Task<Result<void>> NetworkClient::subscribeEvent(int eventId, bool subscribe) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::post,
                                             endpoint(std::format("/v2/client/event/{}/subscribe",
                                                                  eventId),
                                                      {{"subscribe", subscribe}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    co_return Ok();
}

Task<Result<void>> NetworkClient::subscribeDepartment(std::string const& larkDepartment,
                                                      bool subscribe) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::post,
                                             endpoint(std::format("/v2/client/event/{}/subscribe",
                                                                  larkDepartment),
                                                      {{"subscribe", subscribe}}));
    if (result.isErr())
        co_return Err(result.unwrapErr());

    co_return Ok();
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

urls::url NetworkClient::endpoint(std::string_view endpoint) {
    return urls::url(EVENTO_API_GATEWAY + endpoint.data());
}

urls::url NetworkClient::endpoint(std::string_view endpoint,
                                  std::initializer_list<urls::param> const& params) {
    auto r = urls::url(EVENTO_API_GATEWAY + endpoint.data());
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