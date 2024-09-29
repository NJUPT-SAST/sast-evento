// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "UserInfoEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntityV1 {
    std::string token; // expires in 30 days
    UserInfoEntity userInfo;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntityV1, token, userInfo);
};

} // namespace evento