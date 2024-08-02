#pragma once

#include <string_view>
namespace evento {

class Error {
public:
    enum Kind {
        Ssl = 0,
        Network,
        JsonDes,
        Data,
        Unknown,
    } kind;

    Error(Kind kind, std::string_view reason)
        : kind(kind)
        , _reason(reason.data()) {}

    Error(Kind kind)
        : kind(kind)
        , _reason(_reasonMap[kind]) {}

    [[nodiscard]] char const* what() const { return _reason; }

    operator char const*() const { return what(); }

private:
    char const* _reason;

    inline static char const* _reasonMap[Unknown + 1] = {"SSL error!",
                                                         "Network error!",
                                                         "Json Deserialization error!",
                                                         "Data error",
                                                         "Unknown Error"};
};

} // namespace evento