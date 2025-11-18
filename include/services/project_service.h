#pragma once

#include <memory>
#include <vector>

#include "entities/company.h"
#include "entities/project.h"
#include "entities/task.h"

class ProjectService {
   public:
    static void addTaskToProject(Company* company, int projectId, const Task& task);
    static std::vector<Task> getProjectTasks(const Company* company, int projectId);
    static void recomputeProjectTotals(Company* company, int projectId);
};
