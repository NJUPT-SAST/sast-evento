#pragma once

// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

using eventId_t = std::string;

struct SlideEntity {
    int id;
    eventId_t eventId;
    std::string url;
    std::string link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SlideEntity, id, eventId, url, link);
};

} // namespace evento