// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct ReleaseEntity{
    std::string tag_name;
    std::string name;
    std::string body;
    std::string html_url;
    std::string published_at;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ReleaseEntity, 
                                                tag_name, 
                                                name, 
                                                body, 
                                                html_url, 
                                                published_at);
};

}// namespace evento
