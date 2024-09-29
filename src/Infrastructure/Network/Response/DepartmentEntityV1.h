// IWYU pragma: private, include <Infrastructure/Network/ResponseStruct.h>
#pragma once

#include <nlohmann/json.hpp>
#include <string>

namespace evento {

struct DepartmentEntityV1 {
    int id;
    std::string departmentName;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DepartmentEntityV1, id, departmentName);
};

} // namespace evento