// IWYU pragma: private, include <ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct DepartmentInfoEntity {
    std::string id;
    std::string name;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DepartmentInfoEntity, id, name);
};

} // namespace evento
