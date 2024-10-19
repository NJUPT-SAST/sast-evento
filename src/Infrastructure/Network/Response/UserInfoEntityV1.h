// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace evento {

struct UserInfoEntityV1 {
    std::string id;
    std::string linkId;
    std::string studentId;
    std::string email;
    std::string nickname;
    std::optional<std::string> avatar;
    std::optional<std::string> organization;
    std::optional<std::string> biography;
    std::optional<std::vector<std::string>> link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UserInfoEntityV1,
                                                id,
                                                linkId,
                                                studentId,
                                                email,
                                                nickname,
                                                avatar,
                                                organization,
                                                biography,
                                                link);
};

} // namespace evento