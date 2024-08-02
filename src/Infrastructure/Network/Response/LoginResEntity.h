#pragma once

#include "UserInfoEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntity {
    std::string accessToken;
    std::string refreshToken;
    UserInfoEntity user;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntity, accessToken, refreshToken, user);
};

} // namespace evento