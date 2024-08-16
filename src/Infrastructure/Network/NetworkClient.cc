#include "NetworkClient.h"
#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <Infrastructure/Utils/Debug.h>
#include <boost/url/param.hpp>
#include <boost/url/params_view.hpp>
#include <memory>
#include <nlohmann/json.hpp>

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
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<EventEntityList>> NetworkClient::getActiveEventList() {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<EventEntityList>> NetworkClient::getLatestEventList() {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<EventEntityList>> NetworkClient::getHistoryEventList(int page, int size) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<AttachmentEntity>> NetworkClient::getAttachment(int eventId) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<FeedbackEntity>> NetworkClient::getUserFeedback(int eventId) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<void>> NetworkClient::addUserFeedback(int eventId,
                                                  int rating,
                                                  std::string const& content) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<void>> NetworkClient::checkInEvent(int eventId, std::string const& code) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<void>> NetworkClient::subscribeEvent(int eventId, bool subscribe) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<void>> NetworkClient::subscribeDepartment(std::string const& larkDepartment,
                                                      bool subscribe) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<EventEntityList>> NetworkClient::getParticipatedEvent() {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<EventEntityList>> NetworkClient::getSubscribedEvent() {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<SlideEntityList>> NetworkClient::getHomeSlide() {
    // TODO: implement me!
    co_return Error(Error::Unknown);
}

Task<Result<SlideEntityList>> NetworkClient::getEventSlide(int eventId) {
    // TODO: implement me!
    co_return Error(Error::Unknown);
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
                                                      githubEndpoint("/NJUPT-SAST/sast-evento/contributors"));
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
    auto result = co_await this->request<api::Github>(http::verb::get,
                                                      githubEndpoint("/NJUPT-SAST/sast-evento/releases/latest"));
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