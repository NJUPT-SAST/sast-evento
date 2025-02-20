// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>

namespace evento {

struct LoginResEntity {
    std::string accessToken;  // expires in 1h
    std::string refreshToken; // expires in 7 days

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(LoginResEntity, accessToken, refreshToken);
};

} // namespace evento