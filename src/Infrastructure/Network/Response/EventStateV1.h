// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>

namespace evento {

enum class StateV1 {
    Uninitialized,
    Before,
    Registration,
    Ongoing,
    Cancelled,
    Over,
};

NLOHMANN_JSON_SERIALIZE_ENUM(StateV1,
                             {{StateV1::Uninitialized, 0},
                              {StateV1::Before, 1},
                              {StateV1::Registration, 2},
                              {StateV1::Ongoing, 3},
                              {StateV1::Cancelled, 4},
                              {StateV1::Over, 5}})

} // namespace evento