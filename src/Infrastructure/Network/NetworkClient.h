#pragma once

#include <Infrastructure/Network/Api/Evento.hh>
#include <Infrastructure/Network/Api/Github.hh>
#include <Infrastructure/Network/HttpsAccessManager.h>
#include <Infrastructure/Network/ResponseStruct.h>
#include <Infrastructure/Utils/Debug.h>
#include <Infrastructure/Utils/Result.h>
#include <boost/asio/awaitable.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <concepts>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>

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

struct CacheEntry {
    JsonResult result;
    std::chrono::steady_clock::time_point insertTime;
};

class NetworkClient {
public:
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    Task<Result<LoginResEntity>> loginViaSastLink(std::string const& code);

    Task<Result<UserInfoEntity>> getUserInfo();

    Task<Result<void>> refreshAccessToken(std::string const& refreshToken);

    Task<Result<EventQueryRes>> getActiveEventList();

    Task<Result<EventQueryRes>> getLatestEventList();

    Task<Result<EventQueryRes>> getHistoryEventList(int page, int size = 10);

    Task<Result<EventQueryRes>> getDepartmentEventList(std::string const& larkDepartment,
                                                       int page,
                                                       int size = 10);

    Task<Result<EventQueryRes>> getEventList(std::initializer_list<urls::param> params);

    Task<Result<AttachmentEntity>> getAttachment(int eventId);

    Task<Result<std::optional<FeedbackEntity>>> getUserFeedback(int eventId);

    Task<Result<bool>> addUserFeedback(int eventId, int rating, std::string const& content);

    Task<Result<bool>> checkInEvent(int eventId, std::string const& code);

    Task<Result<bool>> subscribeEvent(int eventId, bool subscribe);

    Task<Result<bool>> subscribeDepartment(std::string const& larkDepartment, bool subscribe);

    Task<Result<EventEntityList>> getParticipatedEvent();

    Task<Result<EventEntityList>> getSubscribedEvent();

    Task<Result<SlideEntityList>> getHomeSlide();

    Task<Result<SlideEntityList>> getEventSlide(int eventId);

    Task<Result<DepartmentEntityList>> getDepartmentList();

    Task<Result<ContributorList>> getContributors();

    Task<Result<ReleaseEntity>> getLatestRelease();

    // access token
    // NOTE: `AUTOMATICALLY` added to request header if exists
    std::optional<std::string> tokenBytes;

private:
    NetworkClient(net::ssl::context& ctx);
    static NetworkClient* getInstance();
    //cache data processing
    std::string generateCacheKey(
        http::verb verb,
        const urls::url_view& url,
        const std::initializer_list<urls::param>& params); //auxiliary function to generate cache key

    bool isCacheEntryExpired(const CacheEntry& entry) const {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::hours>(now - entry.insertTime);
        return duration.count() >= 3;
    }
    // - success => return the `data` field from response json
    //            maybe json object or json array
    // - error => return error message
    template<std::same_as<api::Evento> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        //Generate cache
        std::string cacheKey = generateCacheKey(verb, url, params);

        //Check cache
        auto it = cache.find(cacheKey);
        if (it != cache.end() && !isCacheEntryExpired(it->second)) {
            co_return it->second.result;
        }
        debug(), url;
        auto req = Api::makeRequest(verb, url, tokenBytes, params);

        auto reply = co_await _manager->makeReply(url.host(), req);

        if (reply.isErr())
            co_return reply.unwrapErr();

        auto result = handleResponse(reply.unwrap());

        // Update cache
        cache.emplace(cacheKey, CacheEntry{std::move(result), std::chrono::steady_clock::now()});

        co_return cache[cacheKey].result;
    }

    template<std::same_as<api::Github> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        std::string cacheKey = generateCacheKey(verb, url, params);
        auto it = cache.find(cacheKey);
        if (it != cache.end() && !isCacheEntryExpired(it->second)) {
            co_return it->second.result;
        }
        auto req = Api::makeRequest(verb, url, params);
        auto reply = co_await _manager->makeReply(url.host(), req);
        if (reply.isErr())
            co_return reply.unwrapErr();
        auto result = reply.unwrap();
        cache.emplace(cacheKey, CacheEntry{std::move(result), std::chrono::steady_clock::now()});

        co_return cache[cacheKey].result;
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

private:
    net::ssl::context& _ctx;
    std::unique_ptr<HttpsAccessManager> _manager;
    std::unordered_map<std::string, CacheEntry> cache; //cache data(To be defined)
    friend NetworkClient* networkClient();
};

NetworkClient* networkClient();

} // namespace evento