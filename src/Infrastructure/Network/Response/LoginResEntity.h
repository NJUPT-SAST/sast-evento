// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "UserInfoEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntity {
    std::string token; // expires in 1 hour
    UserInfoEntity user;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntity, token, user);
};

} // namespace evento