// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "UserInfoEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntity {
    std::string accessToken;  // expires in 1 hour
    std::string refreshToken; // expires in 7 days, save it!
    UserInfoEntity userInfo;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntity, accessToken, refreshToken, userInfo);
};

} // namespace evento