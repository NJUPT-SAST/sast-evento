#pragma once

#include <Infrastructure/Cache/Cache.h>
#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <Infrastructure/Network/HttpsAccessManager.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Debug.h>
#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <concepts>
#include <filesystem>
#include <initializer_list>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <spdlog/spdlog.h>
#include <string>
#include <unordered_map>

#define EVENTO_API_V1

namespace evento {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace urls = boost::urls;   // from <boost/url.hpp>

using JsonResult = Result<nlohmann::basic_json<>>;
using SlideEntityList = std::vector<SlideEntity>;
using EventEntityList = std::vector<EventEntity>;
using DepartmentEntityList = std::vector<DepartmentEntity>;
using ContributorList = std::vector<ContributorEntity>;

template<typename T>
using Task = net::awaitable<T>;

using namespace std::chrono_literals;

class NetworkClient {
public:
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    Task<Result<LoginResEntity>> loginViaSastLink(std::string code);

    Task<Result<UserInfoEntity>> getUserInfo();

    Task<Result<void>> refreshAccessToken(std::string refreshToken);

    // active: true
    Task<Result<EventQueryRes>> getActiveEventList(
        std::chrono::steady_clock::duration cacheTtl = 1min);

    // start: now
    Task<Result<EventQueryRes>> getLatestEventList(
        std::chrono::steady_clock::duration cacheTtl = 1min);

    // end: now
    Task<Result<EventQueryRes>> getHistoryEventList(
        int page, int size = 10, std::chrono::steady_clock::duration cacheTtl = 1min);

    // larkDepartmentName: larkDepartment
    Task<Result<EventQueryRes>> getDepartmentEventList(
        std::string larkDepartment,
        int page,
        int size = 10,
        std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<EventQueryRes>> getEventById(int eventId);

    Task<Result<EventQueryRes>> getEventList(std::initializer_list<urls::param> params,
                                             std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<AttachmentEntity>> getAttachment(int eventId);

    Task<Result<std::optional<FeedbackEntity>>> getUserFeedback(
        int eventId, std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<bool>> addUserFeedback(int eventId, int rating, std::string content);

    Task<Result<bool>> checkInEvent(int eventId, std::string code);

    Task<Result<bool>> subscribeEvent(int eventId, bool subscribe);

    Task<Result<bool>> subscribeDepartment(std::string larkDepartment, bool subscribe);

#ifdef EVENTO_API_V1
    Task<Result<ParticipateEntity>> getEventParticipate(int eventId);
#endif

    // isCheckedIn: true
    Task<Result<EventQueryRes>> getParticipatedEvent(
        std::chrono::steady_clock::duration cacheTtl = 1min);

    // isSubscribed: true
    // start: first date of this week
    Task<Result<EventQueryRes>> getSubscribedEvent(
        std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<SlideEntityList>> getHomeSlide(std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<SlideEntityList>> getEventSlide(int eventId,
                                                std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<DepartmentEntityList>> getDepartmentList(
        std::chrono::steady_clock::duration cacheTtl = 1min);

    Task<Result<ContributorList>> getContributors();

    Task<Result<ReleaseEntity>> getLatestRelease();

    Task<Result<std::filesystem::path>> getFile(
        std::string url,
        std::optional<std::filesystem::path> dir = CacheManager::cacheDir(),
        bool useCache = true);

    void clearCache();

    std::string getTotalCacheSizeFormatString();

    // access token
    // NOTE: `AUTOMATICALLY` added to request header if exists
    std::optional<std::string> tokenBytes;

private:
    NetworkClient(net::ssl::context& ctx);
    static NetworkClient* getInstance();
    //cache data processing

    // - success => return the `data` field from response json
    //            maybe json object or json array
    // - error => return error message
    template<std::same_as<api::Evento> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {},
                             std::chrono::steady_clock::duration cacheTtl = 1min) {
        spdlog::info("Requesting: {}", url.data());

        auto cacheKey = CacheManager::generateKey(verb, url, params);

        if (cacheTtl != 0s) {
            //Check cache
            auto cacheEntry = _cacheManager->get(cacheKey);

            if (cacheEntry) {
                spdlog::info("Cache hit: {}", cacheKey);
                co_return Ok(cacheEntry->data);
            }
        }

        auto req = Api::makeRequest(verb, url, tokenBytes, params);

        auto reply = co_await _httpsAccessManager->makeReply(url.host(), req);

        if (reply.isErr())
            co_return reply.unwrapErr();

        auto result = handleResponse(reply.unwrap());

        if (cacheTtl != 0s && result.isOk()) {
            // Update cache
            size_t entrySize = result.unwrap().dump().size();

            _cacheManager->insert(cacheKey,
                                  {.data = std::move(result.unwrap()),
                                   .insertTime = std::chrono::steady_clock::now(),
                                   .ttl = cacheTtl,
                                   .size = entrySize});
        }

        co_return result;
    }

    template<std::same_as<api::Github> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        spdlog::info("Requesting: {}", url.data());

        auto req = Api::makeRequest(verb, url, std::nullopt, params);
        auto reply = co_await _httpsAccessManager->makeReply(url.host(), req);
        if (reply.isErr())
            co_return reply.unwrapErr();

        nlohmann::basic_json<> res;
        try {
            res = nlohmann::json::parse(beast::buffers_to_string(reply.unwrap().body().data()));
            debug(), res.dump();
        } catch (const nlohmann::json::parse_error& e) {
            co_return Err(Error(Error::JsonDes, e.what()));
        }

        co_return res;
    }

    // url builder
    static urls::url endpoint(std::string_view endpoint); // url has no query params
    static urls::url endpoint(std::string_view endpoint,  // url has query params
                              std::initializer_list<urls::param> const& queryParams);
    // response handler for evento backend
    static urls::url githubEndpoint(std::string_view endpoint);
    static urls::url githubEndpoint(std::string_view endpoint,
                                    std::initializer_list<urls::param> const& queryParams);
    //response handler for github api
    static JsonResult handleResponse(http::response<http::dynamic_body> response);

    static Task<bool> saveToDisk(std::string const& data, std::filesystem::path const& path);

private:
    net::ssl::context& _ctx;
    std::unique_ptr<HttpsAccessManager> _httpsAccessManager;
    std::unique_ptr<CacheManager> _cacheManager;
    friend NetworkClient* networkClient();

#ifdef EVENTO_API_V1
    std::unordered_map<std::string, int> departmentIdMap;
#endif
};

NetworkClient* networkClient();

} // namespace evento