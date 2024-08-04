#pragma once

#include <string>
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
        , _reason(reason) {}

    Error(Kind kind)
        : kind(kind)
        , _reason(_reasonMap[kind]) {}

    [[nodiscard]] std::string what() const { return _reason; }

    operator std::string() const { return what(); }

private:
    std::string _reason;

    inline static std::string _reasonMap[Unknown + 1] = {"SSL error!",
                                                         "Network error!",
                                                         "Json Deserialization error!",
                                                         "Data error",
                                                         "Unknown Error"};
};

} // namespace evento