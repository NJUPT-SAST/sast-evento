#pragma once

// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct SlideEntity {
    int id;
    int eventId;
    std::string url;
    std::string link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SlideEntity, id, eventId, url, link);
};

} // namespace evento