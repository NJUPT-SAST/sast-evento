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
#include <concepts>
#include <initializer_list>
#include <nlohmann/json.hpp>
#include <string>

namespace evento {

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace net = boost::asio;    // from <boost/asio.hpp>
namespace urls = boost::urls;   // from <boost/url.hpp>

using JsonResult = Result<nlohmann::basic_json<>>;
using SlideEntityList = std::vector<SlideEntity>;
using DepartmentInfoEntityList = std::vector<DepartmentInfoEntity>;
template<typename T>
using Task = net::awaitable<T>;

class NetworkClient {
public:
    NetworkClient(const NetworkClient&) = delete;
    NetworkClient& operator=(const NetworkClient&) = delete;

    Task<Result<LoginResEntity>> loginViaSastLink(std::string const& code);

    Task<Result<UserInfoEntity>> getUserInfo();

    Task<Result<void>> refreshAccessToken(std::string const& refreshToken);

    Task<Result<EventEntityList>> getActiveEventList();

    Task<Result<EventEntityList>> getLatestEventList();

    Task<Result<EventEntityList>> getHistoryEventList(int page, int size = 10);

    Task<Result<EventEntityList>> getDepartmentEventList(int page, int size = 10);

    Task<Result<EventEntityList>> getActiveEventList(std::string const& Department);

    Task<Result<EventEntityList>> getLatestEventList(std::string const& Department);

    Task<Result<EventEntityList>> getHistoryEventList(std::string const& Department,
                                                      int page,
                                                      int size = 10);

    Task<Result<AttachmentEntity>> getAttachment(int eventId);

    Task<Result<FeedbackEntity>> getUserFeedback(int eventId);

    Task<Result<bool>> addUserFeedback(int eventId, int rating, std::string const& content);

    Task<Result<bool>> checkInEvent(int eventId, std::string const& code);

    Task<Result<bool>> subscribeEvent(int eventId, bool subscribe);

    Task<Result<bool>> subscribeDepartment(std::string const& larkDepartment, bool subscribe);

    Task<Result<EventEntityList>> getParticipatedEvent();

    Task<Result<EventEntityList>> getSubscribedEvent();

    Task<Result<SlideEntityList>> getHomeSlide();

    Task<Result<SlideEntityList>> getEventSlide(int eventId);

    Task<Result<DepartmentInfoEntityList>> getDepartmentInfo();

    // access token
    // NOTE: `AUTOMATICALLY` added to request header if exists
    std::optional<std::string> tokenBytes;

private:
    NetworkClient(net::ssl::context& ctx);
    static NetworkClient* getInstance();

    // - success => return the `data` field from response json
    //            maybe json object or json array
    // - error => return error message
    template<std::same_as<api::Evento> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        debug(), url;
        auto req = Api::makeRequest(verb, url, tokenBytes, params);

        auto reply = co_await _manager->makeReply(url.host(), req);

        if (reply.isErr())
            co_return reply.unwrapErr();

        co_return handleResponse(reply.unwrap());
    }

    template<std::same_as<api::Github> Api>
    Task<JsonResult> request(http::verb verb,
                             urls::url_view url,
                             std::initializer_list<urls::param> const& params = {}) {
        auto req = Api::makeRequest(verb, url, params);
        auto reply = co_await _manager->makeReply(url.host(), req);
        if (reply.isErr())
            co_return reply.unwrapErr();
        co_return reply.unwrap();
    }

    // url builder
    static urls::url endpoint(std::string_view endpoint); // url has no query params
    static urls::url endpoint(std::string_view endpoint,  // url has query params
                              std::initializer_list<urls::param> const& queryParams);
    // response handler for evento backend
    static JsonResult handleResponse(http::response<http::dynamic_body> response);

private:
    net::ssl::context& _ctx;
    std::unique_ptr<HttpsAccessManager> _manager;

    friend NetworkClient* networkClient();
};

NetworkClient* networkClient();

} // namespace evento