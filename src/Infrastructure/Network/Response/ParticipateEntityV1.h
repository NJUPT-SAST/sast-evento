// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>

namespace evento {

struct ParticipateEntity {
    bool isRegistration;
    bool isParticipate;
    bool isSubscribe;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ParticipateEntity,
                                                isRegistration,
                                                isParticipate,
                                                isSubscribe);
};

} // namespace evento