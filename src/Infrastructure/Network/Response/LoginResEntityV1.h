// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "UserInfoEntityV1.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntityV1 {
    std::string token; // expires in 30 days
    UserInfoEntityV1 userInfo;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntityV1, token, userInfo);
};

} // namespace evento