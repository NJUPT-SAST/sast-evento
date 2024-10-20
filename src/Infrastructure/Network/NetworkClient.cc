#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <Infrastructure/Network/NetworkClient.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Tools.h>
#include <array>
#include <nlohmann/json.hpp>
#if defined(PLATFORM_APPLE)
#include <fstream>
#endif

namespace evento {

static const std::string EVENTO_API_GATEWAY = "https://evento.sast.fun/api";
static const std::string GITHUB_API_GATEWAY = "https://api.github.com/repos";

constexpr const char MIME_JSON[] = "application/json";
constexpr const char MIME_FORM_URL_ENCODED[] = "application/x-www-form-urlencoded";

NetworkClient::NetworkClient()
    : _httpsAccessManager(std::make_unique<HttpsAccessManager>(true))
    , _cacheManager(std::make_unique<CacheManager>()) {}

NetworkClient* NetworkClient::getInstance() {
    static NetworkClient s_instance;
    return &s_instance;
}

Task<Result<LoginResEntity>> NetworkClient::loginViaSastLink(std::string code) {
    auto result = co_await this->request<api::Evento>(http::verb::post,
                                                      endpoint("/v2/login/link"),
                                                      {{"code", code},
                                                       {"type", "0"},
                                                       {"codeVerifier", "sast_forever"}});

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

Task<Result<void>> NetworkClient::refreshAccessToken(std::string refreshToken) {
    auto result = co_await this->request<api::Evento>(http::verb::post,
                                                      endpoint("/v2/login/refresh-token"),
                                                      nlohmann::basic_json<>{
                                                          {"refreshToken", refreshToken}});
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
    auto result = co_await this->request<api::Evento>(
        http::verb::get,
        endpoint("/v2/client/event/query",
                 {{"start", stdChrono2Iso8601Utc(std::chrono::system_clock::now())}}),
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
    auto result = co_await this->request<api::Evento>(
        http::verb::get,
        endpoint("/v2/client/event/query",
                 {{"page", std::to_string(page)},
                  {"size", std::to_string(size)},
                  {"end", stdChrono2Iso8601Utc(std::chrono::system_clock::now())}}),
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
    std::string larkDepartment, int page, int size, std::chrono::steady_clock::duration cacheTtl) {
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

Task<Result<EventQueryRes>> NetworkClient::getEventById(int eventId) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"id", std::to_string(eventId)}}),
                                                      {},
                                                      0min);
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
    auto result = co_await this
                      ->request<api::Evento>(http::verb::get,
                                             endpoint(std::format("/v2/client/event/{}/attachments",
                                                                  eventId)));
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

Task<Result<bool>> NetworkClient::addUserFeedback(int eventId, int rating, std::string content) {
    auto result = co_await this->request<api::Evento>(
        http::verb::post,
        endpoint(std::format("/v2/client/event/{}/feedback", eventId),
                 {{"rating", std::to_string(rating)}, {"content", content}}));

    if (result.isErr())
        co_return Err(result.unwrapErr());

    if (result.unwrap().is_boolean())
        co_return Ok(result.unwrap().get<bool>());

    co_return Err(Error(Error::Data, "response data type error"));
}

Task<Result<bool>> NetworkClient::checkInEvent(int eventId, std::string code) {
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

Task<Result<bool>> NetworkClient::subscribeDepartment(std::string larkDepartment, bool subscribe) {
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

Task<Result<EventQueryRes>> NetworkClient::getParticipatedEvent(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"isCheckedIn", "true"}}),
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

Task<Result<EventQueryRes>> NetworkClient::getSubscribedEvent(
    std::chrono::steady_clock::duration cacheTtl) {
    auto startTime = firstDateTimeOfWeek();
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/query",
                                                               {{"isSubscribed", "true"},
                                                                {"start", startTime}}),
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

Task<Result<SlideEntityList>> NetworkClient::getHomeSlide(
    std::chrono::steady_clock::duration cacheTtl) {
    auto result = co_await this->request<api::Evento>(http::verb::get,
                                                      endpoint("/v2/client/event/slide"),
                                                      {},
                                                      cacheTtl);
    if (result.isErr())
        co_return Err(result.unwrapErr());

    SlideEntityList list;

    try {
        nlohmann::from_json(result.unwrap(), list);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(list);
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

    DepartmentEntityList list;
    try {
        nlohmann::from_json(result.unwrap(), list);
    } catch (const nlohmann::json::exception& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }

    co_return Ok(list);
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

Task<Result<std::filesystem::path>> NetworkClient::getFile(std::string urlStr,
                                                           std::optional<std::filesystem::path> dir,
                                                           bool useCache) {
    if (urlStr.ends_with('\\')) {
        urlStr.pop_back();
    }
    spdlog::debug("Downloading file: {}", urlStr);
    auto url = urls::url(urlStr);
    http::request<http::string_body> req{http::verb::get,
                                         std::format("{}{}{}",
                                                     url.encoded_path().data(),
                                                     url.has_query() ? "?" : "",
                                                     url.encoded_query().data()),
                                         11};

    if (!dir) {
        co_return Err(Error(Error::Data, "directory not found"));
    }

    auto stem = CacheManager::generateStem(urlStr);

    if (useCache) {
        std::filesystem::directory_iterator iter(*dir);
        for (const auto& file : iter) {
            if (file.path().filename().stem().string() == stem) {
                co_return Ok(std::filesystem::absolute(file.path()));
            }
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

    auto data = beast::buffers_to_string(response.body().data());
    if (data.length() < 4) { // magic number length
        co_return Err(Error(Error::Data, "file data error"));
    }

    auto value = type->value();
    stem += '.';
    if (value.substr(0, value.find('/')) == "image") {
        auto ext = guessImageExtByBytes(std::array<unsigned char, 4>{(unsigned char) data[0],
                                                                     (unsigned char) data[1],
                                                                     (unsigned char) data[2],
                                                                     (unsigned char) data[3]});
        spdlog::debug("Guess image ext: {}", ext);
        stem += ext;
    } else {
        stem += value.substr(value.find('/') + 1);
    }
    auto path = *dir / stem;

    if (!co_await saveToDisk(data, path)) {
        co_return Err(Error(Error::Data, "save file failed"));
    }
    co_return Ok(path);
}

void NetworkClient::clearCache() {
    _cacheManager->clear();
}

void NetworkClient::clearMemoryCache() {
    _cacheManager->clearMemoryCache();
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

JsonResult NetworkClient::handleEventoResponse(http::response<http::dynamic_body> response) {
    if (response.result() != http::status::ok) {
        return Err(Error(response.result_int()));
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

    constexpr auto errorMessageFieldName = "errMsg";

    if (!res["success"].is_boolean() || !res["success"].get<bool>()) {
        if (res.contains(errorMessageFieldName)) {
            return Err(Error(Error::Data, res[errorMessageFieldName].get<std::string>()));
        }
        return Err(err);
    }

    // Here we are sure that the data is valid
    assert(res["success"].get<bool>());

    auto data = res.contains("data") ? res["data"] : nlohmann::json::object();

    return Ok(data);
}

Task<JsonResult> NetworkClient::handleGithubResponse(http::response<http::dynamic_body> response) {
    auto status = response.result();
    std::string data;
    if (status == http::status::found || status == http::status::moved_permanently
        || status == http::status::temporary_redirect) {
        auto location = response.base().at("Location");
        spdlog::info("Redirecting to {}", std::string(location));
        auto redirectUrl = urls::url_view(location);
        co_return co_await this->request<api::Github>(http::verb::get, redirectUrl);
    } else if (status != http::status::ok) {
        co_return Err(Error(response.result_int()));
    }

    data = beast::buffers_to_string(response.body().data());

    nlohmann::basic_json<> res;
    try {
        res = nlohmann::json::parse(data);
        debug(), res.dump();
    } catch (const nlohmann::json::parse_error& e) {
        co_return Err(Error(Error::JsonDes, e.what()));
    }
    co_return Ok(res);
}

Task<bool> NetworkClient::saveToDisk(std::string const& data, std::filesystem::path const& path) {
#if defined(PLATFORM_APPLE)

    auto result = co_await net::co_spawn(
        co_await net::this_coro::executor,
        [data, path]() -> Task<bool> {
            try {
                std::ofstream file(path, std::ios::binary | std::ios::out | std::ios::trunc);
                if (!file.is_open()) {
                    spdlog::warn("Failed to open file: {}", path.string());
                    co_return false;
                }
                file << data;
                file.close();
                co_return true;
            } catch (std::exception const& e) {
                spdlog::warn("Failed to save file: {}", e.what());
                co_return false;
            }
        },
        net::use_awaitable);
    co_return result;

#else

    net::stream_file file(co_await net::this_coro::executor,
                          path.string(),
                          net::stream_file::write_only | net::stream_file::create);
    if (!file.is_open()) {
        co_return false;
    }

    auto totalSize = data.size();
    std::size_t bytesWritten = 0;
    while (bytesWritten < totalSize) {
        bytesWritten += co_await file.async_write_some(net::buffer(data.data() + bytesWritten,
                                                                   totalSize - bytesWritten),
                                                       net::use_awaitable);
    }

    file.close();
    co_return true;

#endif
}

NetworkClient* networkClient() {
    return NetworkClient::getInstance();
}

} // namespace evento
