#include "NetworkClient.h"
#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url/param.hpp>
#include <cstdint>
#include <filesystem>
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
    , _httpsAccessManager(std::make_unique<HttpsAccessManager>(_ctx, true))
    , _cacheManager(std::make_unique<CacheManager>()) {}

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

Task<Result<EventQueryRes>> NetworkClient::getActiveEventList(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"active", "true"}}),
                                                      {},
                                                      cacheTtl);
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

Task<Result<EventQueryRes>> NetworkClient::getLatestEventList(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"start", "now"}}),
                                                      {},
                                                      cacheTtl);
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

Task<Result<EventQueryRes>> NetworkClient::getHistoryEventList(
    int page, int size, std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"page", std::to_string(page)},
                                                                {"size", std::to_string(size)},
                                                                {"end", "now"}}),
                                                      {},
                                                      cacheTtl);
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

Task<Result<EventQueryRes>> NetworkClient::getDepartmentEventList(
    std::string const& larkDepartment,
    int page,
    int size,
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"page", std::to_string(page)},
                                                                {"size", std::to_string(size)},
                                                                {"larkDepartmentName",
                                                                 larkDepartment}}),
                                                      {},
                                                      cacheTtl);
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

Task<Result<EventQueryRes>> NetworkClient::getEventList(
    std::initializer_list<urls::param> params, std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query", params),
                                                      {},
                                                      cacheTtl);
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

Task<Result<std::optional<FeedbackEntity>>> NetworkClient::getUserFeedback(
    int eventId, std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this
                      ->request<api::Evento>(http::verb::get,
                                             endpoint(std::format("/v2/client/event/{}/feedback",
                                                                  eventId)),
                                             {},
                                             cacheTtl);
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

Task<Result<EventEntityList>> NetworkClient::getParticipatedEvent(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/participated"),
                                                      {},
                                                      cacheTtl);
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

Task<Result<EventEntityList>> NetworkClient::getSubscribedEvent(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/subscribed"),
                                                      {},
                                                      cacheTtl);
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

Task<Result<SlideEntityList>> NetworkClient::getHomeSlide(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/slide"),
                                                      {},
                                                      cacheTtl);
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

Task<Result<SlideEntityList>> NetworkClient::getEventSlide(
    int eventId, std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint(
                                                          std::format("/v2/client/event/{}/slide",
                                                                      eventId)),
                                                      {},
                                                      cacheTtl);
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

Task<Result<DepartmentEntityList>> NetworkClient::getDepartmentList(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/lark/department"),
                                                      {},
                                                      cacheTtl);
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

Task<Result<std::filesystem::path>> NetworkClient::getFile(urls::url_view url) {
    http::request<http::string_body> req{http::verb::get, url.path(), 11};
    // use cache first
    auto cacheDir = CacheManager::cacheDir();
    if (!cacheDir) {
        co_return Err(Error(Error::Data, "cache dir not found"));
    }

    auto stem = CacheManager::generateStem(url);

    std::filesystem::directory_iterator iter(*cacheDir);
    for (const auto& file : iter) {
        if (file.path().filename().stem().string() == stem) {
            co_return Ok(std::filesystem::absolute(file.path()));
        }
    }

    req.set(http::field::host, url.host_name());
    req.set(http::field::user_agent, "SAST-Evento-Desktop/2");
    req.set(http::field::accept, "*/*");

    // if cache not exists, download file
    auto reply = co_await _httpsAccessManager->makeReply(url.host(), req);
    if (reply.isErr())
        co_return Err(reply.unwrapErr());

    auto response = reply.unwrap();

    if (response.result() != http::status::ok) {
        co_return Err(Error(Error::Network, std::to_string(response.result_int())));
    }

    auto type = response.find(http::field::content_type);
    if (type == response.end()) {
        co_return Err(Error(Error::Data, "file type error"));
    }

    auto value = type->value();
    stem += '.';
    stem += value.substr(value.find('/') + 1);
    auto path = *cacheDir / stem;

    if (!CacheManager::saveToDisk(beast::buffers_to_string(response.body().data()), path)) {
        co_return Err(Error(Error::Data, "save file failed"));
    }
    co_return Ok(path);
}

void NetworkClient::clearCache() {
    _cacheManager->clear();
}

std::string NetworkClient::getTotalCacheSizeFormatString() {
    if (auto dir = _cacheManager->cacheDir()) {
        std::uintmax_t size = 0;
        for (const auto& file : std::filesystem::directory_iterator(*dir)) {
            size += file.file_size();
        }

        if (size < 1024) {
            return std::format("{}B", size);
        } else if (size < 1024 * 1024) {
            return std::format("{:.2f}KiB", static_cast<double>(size) / 1024);
        } else if (size < 1024 * 1024 * 1024) {
            return std::format("{:.2f}MiB", static_cast<double>(size) / 1024 / 1024);
        } else {
            return std::format("{:.2f}GiB", static_cast<double>(size) / 1024 / 1024 / 1024);
        }
    }

    return "0B";
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