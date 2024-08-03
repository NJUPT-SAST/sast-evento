// IWYU pragma: private, include "ResponseStructure.h"
#pragma once

#include "EventEntity.h"
#include <nlohmann/json.hpp>
#include <vector>

namespace evento {

struct EventListEntity {
    std::vector<EventEntity> data;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EventListEntity, data);
};

} // namespace evento