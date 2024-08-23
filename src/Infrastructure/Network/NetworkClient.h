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
#include <iostream>
#include <list>
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
    std::size_t size; //cache size
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
    void updateCache(const std::string& key, CacheEntry entry) {
        try {
            auto it = cacheMap.find(key);
            if (it != cacheMap.end()) {
                cacheList.erase(it->second);
                currentCacheSize -= it->second->second.size;
            }
            cacheList.push_front({key, std::move(entry)});
            cacheMap[key] = cacheList.begin();
            currentCacheSize += cacheList.begin()->second.size;

            while (currentCacheSize > maxCacheSize) {
                auto last = cacheList.end();
                --last;
                currentCacheSize -= last->second.size;
                cacheMap.erase(last->first);
                cacheList.pop_back();
            }
        } catch (const std::exception& e) {
            std::cerr << "Error occurred: " << e.what() << std::endl;
        }
    }
    // - success => return the `data` field from response json
    //            maybe json object or json array
    // - error => return error message
    template<std::same_as<api::Evento> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        try {
            //Generate cache
            std::string cacheKey = generateCacheKey(verb, url, params);

            //Check cache
            auto it = cacheMap.find(cacheKey);
            if (it != cacheMap.end() && !isCacheEntryExpired(it->second->second)) {
                cacheList.splice(cacheList.begin(), cacheList, it->second);
                co_return it->second->second.result;
            }
            debug(), url;
            auto req = Api::makeRequest(verb, url, tokenBytes, params);

            auto reply = co_await _manager->makeReply(url.host(), req);

            if (reply.isErr())
                co_return reply.unwrapErr();

            auto result = handleResponse(reply.unwrap());

            // Update cache
            size_t entrySize = result.unwrap().dump().size();
            updateCache(cacheKey,
                        CacheEntry{std::move(result), std::chrono::steady_clock::now(), entrySize});

            co_return cacheMap[cacheKey]->second.result;
        } catch (const std::exception& e) {
            std::cerr << "Request error occurred: " << e.what() << std::endl;
            co_return JsonResult::Err(e.what());
        }
    }

    template<std::same_as<api::Github> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        try {
            std::string cacheKey = generateCacheKey(verb, url, params);
            auto it = cacheMap.find(cacheKey);
            if (it != cacheMap.end() && !isCacheEntryExpired(it->second->second)) {
                cacheList.splice(cacheList.begin(), cacheList, it->second);
                co_return it->second->second.result;
            }
            auto req = Api::makeRequest(verb, url, params);
            auto reply = co_await _manager->makeReply(url.host(), req);
            if (reply.isErr())
                co_return reply.unwrapErr();
            auto result = reply.unwrap();
            size_t entrySize = result.dump().size();
            updateCache(cacheKey,
                        CacheEntry{std::move(result), std::chrono::steady_clock::now(), entrySize});

            co_return cacheMap[cacheKey]->second.result;
        } catch (const std::exception& e) {
            std::cerr << "Request error occurred: " << e.what() << std::endl;
            co_return JsonResult::Err(e.what());
        }
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
    std::list<std::pair<std::string, CacheEntry>> cacheList;               // LRU 缓存列表
    std::unordered_map<std::string, decltype(cacheList.begin())> cacheMap; // 缓存映射(changed,TBD)
    size_t currentCacheSize = 0;                                           // 当前缓存大小
    const size_t maxCacheSize = 64 * 1024 * 1024;
    friend NetworkClient* networkClient();
};

NetworkClient* networkClient();

} // namespace evento