// IWYU pragma: private, include <Infrastructure/Utils/Result.h>
#pragma once

#include <string>
#include <unordered_map>

namespace evento {

class Error {
public:
    enum Kind { Ssl = 0, Network, JsonDes, Data, Unknown, Timeout } kind;

    Error(Kind kind, std::string_view reason)
        : kind(kind)
        , _reason(reason) {}

    Error(Kind kind)
        : kind(kind)
        , _reason(_reasonMap[kind]) {}

    Error(int code)
        : kind(Unknown)
        , _reason(_errorCodeMap.at(code)) {}

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

    inline static std::unordered_map<int, std::string> _errorCodeMap = {
        {100, "Continue"},
        {200, "OK"},
        {400, "Bad Request"},
        {401, "Unauthorized"},
        {403, "Forbidden"},
        {404, "Not Found"},
        {500, "Internal Server Error"},
        {502, "Bad Gateway"},
        {503, "Service Unavailable"},
    };
};

} // namespace evento