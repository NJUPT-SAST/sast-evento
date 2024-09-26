// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "EventState.h"
#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>
namespace evento {

struct EventEntity {
    int id;
    std::string summary;
    std::string description;
    std::string start;
    std::string end;
    std::optional<std::string> location;
    std::string tag;
    std::optional<std::string> larkMeetingRoomName;
    std::string larkDepartmentName;
    State state;
    bool isSubscribed; // 已订阅
    bool isCheckedIn;  // 已签到

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EventEntity,
                                   id,
                                   summary,
                                   description,
                                   start,
                                   end,
                                   location,
                                   tag,
                                   larkMeetingRoomName,
                                   larkDepartmentName,
                                   state,
                                   isSubscribed,
                                   isCheckedIn);
};

} // namespace evento