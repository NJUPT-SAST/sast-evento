// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "EventEntity.h"
#include <nlohmann/json.hpp>

namespace evento {

struct EventQueryRes {
    std::vector<EventEntity> elements;
    int current;
    int total;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(EventQueryRes, elements, current, total);
};

} // namespace evento