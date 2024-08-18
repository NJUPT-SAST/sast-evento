// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace evento {

enum class State {
    SigningUp, // 报名中
    Active,    // 正在进行
    Completed, // 已结束
    Cancelled, // 已取消
};

NLOHMANN_JSON_SERIALIZE_ENUM(State,
                             {{State::SigningUp, "SIGNING_UP"},
                              {State::Active, "ACTIVE"},
                              {State::Completed, "COMPLETED"},
                              {State::Cancelled, "CANCELLED"}})

} // namespace evento