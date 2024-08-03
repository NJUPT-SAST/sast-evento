// IWYU pragma: private, include "ResponseStructure.h"
#pragma once

#include "UserInfoEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntity {
    std::string accessToken;  // expires in 1 hour
    std::string refreshToken; // expires in 7 days, save it!
    UserInfoEntity user;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntity, accessToken, refreshToken, user);
};

} // namespace evento