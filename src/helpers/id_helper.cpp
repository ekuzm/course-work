#include "helpers/id_helper.h"

#include "entities/employee.h"
#include "entities/project.h"

int IdHelper::findMaxEmployeeId(
    const std::vector<std::shared_ptr<Employee>>& employees) {
    if (employees.empty()) {
        return 0;
    }

    int maxId = 0;
    for (const auto& emp : employees) {
        if (emp && emp->getId() > maxId) {
            maxId = emp->getId();
        }
    }
    return maxId;
}

int IdHelper::findMaxProjectId(const std::vector<Project>& projects) {
    if (projects.empty()) {
        return 0;
    }

    int maxId = 0;
    for (const auto& proj : projects) {
        if (proj.getId() > maxId) {
            maxId = proj.getId();
        }
    }
    return maxId;
}

int IdHelper::calculateNextId(int maxId) {
    return maxId + 1;
}


