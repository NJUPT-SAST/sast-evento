// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct AttachmentEntity {
    int id;
    int eventId;
    std::string url;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(AttachmentEntity, id, eventId, url);
};

} // namespace evento
