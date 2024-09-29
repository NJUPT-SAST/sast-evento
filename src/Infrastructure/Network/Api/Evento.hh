#pragma once

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/url/url_view.hpp>
#include <format>
#include <initializer_list>
#include <optional>

namespace evento::api {

namespace http = boost::beast::http;
namespace urls = boost::urls;

constexpr const char MIME_FORM_URL_ENCODED[] = "application/x-www-form-urlencoded";

struct Evento {
    Evento() = delete;
    ~Evento() = delete;

    // make http request
    // @param `token`: optional, if exists, set it in the header
    // @param `formParams`: default is empty, set it in the body as form url encoded
    static http::request<http::string_body> makeRequest(
        http::verb verb,
        boost::urls::url_view url,
        std::optional<std::string> const& token = std::nullopt,
        std::initializer_list<urls::param> const& formParams = {}) {
        http::request<http::string_body> req{verb,
                                             std::format("{}{}{}",
                                                         url.path(),
                                                         url.has_query() ? "?" : "",
                                                         url.query()),
                                             11};

        req.set(http::field::host, url.host());
        req.set(http::field::user_agent, "SAST-Evento-Desktop/2");
        if (token) // set token if exists
            req.set("TOKEN", *token);

        if (verb == http::verb::post) {
            req.set(http::field::content_type, MIME_FORM_URL_ENCODED);
            std::ostringstream params_stream;
            for (auto const& param : formParams) {
                params_stream << param.key << "=" << param.value;
                if (&param != formParams.end() - 1) {
                    params_stream << "&";
                }
            }
            req.body() = params_stream.str();
        }

        return req;
    }
};

} // namespace evento::api
