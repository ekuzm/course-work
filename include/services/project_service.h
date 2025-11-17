#pragma once

#include <memory>
#include <vector>

#include "entities/company.h"
#include "entities/project.h"
#include "entities/task.h"

class ProjectService {
   private:
    Company* company;

   public:
    explicit ProjectService(Company* company);

    void addTaskToProject(int projectId, const Task& task);
    std::vector<Task> getProjectTasks(int projectId) const;
    void recomputeProjectTotals(int projectId);
};
