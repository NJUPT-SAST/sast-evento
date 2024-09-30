// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "DepartmentEntityV1.h"
#include "EventStateV1.h"
#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
namespace evento {

struct EventEntityV1 {
    int id;
    std::string title;
    std::string description;
    std::string gmtEventStart;
    std::string gmtEventEnd;
    std::optional<std::string> location;
    std::string tag;
    StateV1 state;
    std::vector<DepartmentEntityV1> departments;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EventEntityV1,
                                   id,
                                   title,
                                   description,
                                   gmtEventStart,
                                   gmtEventEnd,
                                   location,
                                   tag,
                                   state,
                                   departments);
};

} // namespace evento