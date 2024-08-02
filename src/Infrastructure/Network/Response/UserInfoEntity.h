#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace evento {

struct UserInfoEntity {
    std::string id;
    std::string linkId;
    std::string studentId;
    std::string email;
    std::string nickname;
    std::string avatar;
    std::string organization;
    std::string biography;
    std::vector<std::string> link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserInfoEntity,
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