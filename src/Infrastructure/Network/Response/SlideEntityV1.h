#pragma once

// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct SlideEntityV1 {
    int id;
    std::string title;
    std::string url;
    std::string link;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SlideEntityV1, id, title, url, link);
};

struct SlideEntityListV1 {
    std::vector<SlideEntityV1> slides;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SlideEntityListV1, slides);
};

} // namespace evento