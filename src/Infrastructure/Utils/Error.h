// IWYU pragma: private, include <Infrastructure/Utils/Result.h>
#pragma once

#include <string>
#include <unordered_map>

namespace evento {

class Error {
public:
    enum Kind {
        Ssl = 0,
        Network,
        JsonDes,
        Data,
        Unknown,
        Timeout,
    } kind;

    Error(Kind kind, std::string_view reason)
        : kind(kind)
        , _reason(reason) {}

    Error(Kind kind)
        : kind(kind)
        , _reason(_reasonMap[kind]) {}

    Error(unsigned int httpStatusCode)
        : kind(Unknown) {
        if (_httpStatusCodeMap.contains(httpStatusCode)) {
            _reason = _httpStatusCodeMap[httpStatusCode];
        } else {
            _reason = _reasonMap[Kind::Network];
        }
    }

    [[nodiscard]] std::string what() const { return _reason; }

    operator std::string() const { return what(); }

private:
    std::string _reason;

    inline static std::string _reasonMap[Timeout + 1] = {"SSL error!",
                                                         "Network error!",
                                                         "Json Deserialization error!",
                                                         "Data error",
                                                         "Unknown Error",
                                                         "Timeout error!"};

    inline static std::unordered_map<unsigned, std::string> _httpStatusCodeMap = {
        {400u, "400 Bad Request"},
        {401u, "401 Unauthorized"},
        {403u, "403 Forbidden"},
        {404u, "404 Not Found"},
        {500u, "500 Internal Server Error"},
        {502u, "502 Bad Gateway"},
        {503u, "503 Service Unavailable"},
    };
};

} // namespace evento