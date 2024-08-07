// IWYU pragma: private, include "ResponseStruct.h"
#pragma once

#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct FeedbackEntity {
    int id{};      // feedback id
    int linkId{};  // which user this feedback belongs to
    int eventId{}; // which event this feedback belongs to
    int rating{};  // 0 - 5
    std::optional<std::string> feedback;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        FeedbackEntity, id, linkId, eventId, rating, feedback);
};

} // namespace evento