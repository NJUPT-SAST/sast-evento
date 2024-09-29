// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include "StdOptional.h" // IWYU pragma: keep
#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct FeedbackEntityV1 {
    int id{};      // feedback id
    int eventId{}; // which event this feedback belongs to
    int score{};   // 0 - 5
    std::optional<std::string> content;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(FeedbackEntityV1, id, eventId, score, content);
};

} // namespace evento