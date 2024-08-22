// IWYU pragma: private, include <ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct DepartmentEntity {
    std::string id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DepartmentEntity, id, name);
};

} // namespace evento
