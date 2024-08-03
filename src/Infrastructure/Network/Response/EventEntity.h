// IWYU pragma: private, include "ResponseStructure.h"
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct EventEntity {
    int id;
    std::string summary;
    std::string description;
    std::string start;
    std::string end;
    std::string location;
    std::string tag;
    std::string larkMeetingRoomId;
    std::string larkMeetingRoomName;
    std::string larkDepartmentId;
    std::string larkDepartmentName;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EventEntity,
                                   id,
                                   summary,
                                   description,
                                   start,
                                   end,
                                   location,
                                   tag,
                                   larkMeetingRoomId,
                                   larkMeetingRoomName,
                                   larkDepartmentId,
                                   larkDepartmentName);
};

} // namespace evento