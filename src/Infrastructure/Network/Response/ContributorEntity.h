// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct ContributorEntity{
    std::string login;
    std::string avatar_url;
    std::string html_url;
    int contributions;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ContributorEntity,
                                                login,
                                                avatar_url,
                                                html_url,
                                                contributions);
};

}// namespace evento
