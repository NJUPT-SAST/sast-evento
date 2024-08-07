// IWYU pragma: private, include "ResponseStruct.h"
#pragma once

#include <nlohmann/json.hpp>
#include <optional>

// Since nlohmann cannot serialize/deserialize `std::optional`,
// we need to provide a custom ADL(Argument-Dependent Lookup) for it.
NLOHMANN_JSON_NAMESPACE_BEGIN

template<typename T>
struct adl_serializer<std::optional<T>> {
    static void to_json(json& j, const std::optional<T>& opt) {
        if (opt == std::nullopt) {
            j = nullptr;
        } else {
            j = *opt;
        }
    }

    static void from_json(const json& j, std::optional<T>& opt) {
        if (j.is_null()) {
            opt = std::nullopt;
        } else {
            opt = j.template get<T>();
        }
    }
};
NLOHMANN_JSON_NAMESPACE_END