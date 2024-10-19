// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct UserInfoEntity {
    std::string linkId;
    std::string email;
    std::string nickname;
    std::optional<std::string> avatar;
    std::optional<std::string> bio;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfoEntity, linkId, email, nickname, avatar, bio);
};

} // namespace evento