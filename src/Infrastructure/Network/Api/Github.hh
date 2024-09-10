#pragma once

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url/url_view.hpp>
#include <optional>

namespace evento::api {

namespace http = boost::beast::http;
namespace urls = boost::urls;

constexpr const char MIME_GITHUB_JSON[] = "application/vnd.github+json";

struct Github {
    Github() = delete;
    ~Github() = delete;

    static http::request<http::string_body> makeRequest(
        http::verb verb,
        boost::urls::url_view url,
        [[maybe_unused]] std::optional<std::string> const& token = std::nullopt,
        [[maybe_unused]] std::initializer_list<urls::param> const& formParams = {}) {
        http::request<http::string_body> req{verb, url.path(), 11};

        req.set(http::field::host, url.host_name());
        req.set(http::field::user_agent, "SAST-Evento-Desktop/2");
        req.set(http::field::accept, MIME_GITHUB_JSON);
        req.set("X-GitHub-Api-Version", "2022-11-28");

        return req;
    }
};

} // namespace evento::api