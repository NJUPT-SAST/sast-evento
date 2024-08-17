// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "EventState.h"
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
    std::string larkMeetingRoomName;
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

struct EventEntityList {
    std::vector<EventEntity> elements;
    int current;
    int total;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EventEntityList, elements, current, total);
};

} // namespace evento