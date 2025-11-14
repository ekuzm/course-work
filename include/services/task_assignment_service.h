#pragma once

#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "entities/company.h"
#include "entities/employee.h"
#include "exceptions/exceptions.h"
#include "entities/project.h"
#include "services/cost_calculation_service.h"
#include "entities/task.h"

class TaskAssignmentService {
   private:
    Company* company;

   public:
    explicit TaskAssignmentService(Company* company);

    void assignEmployeeToTask(int employeeId, int projectId, int taskId,
                              int hours);
    void restoreTaskAssignment(int employeeId, int projectId, int taskId,
                               int hours);
    void removeEmployeeTaskAssignments(int employeeId);
    void scaleEmployeeTaskAssignments(int employeeId, double scaleFactor);
    void fixTaskAssignmentsToCapacity();
    void recalculateTaskAllocatedHours();
    void autoAssignEmployeesToProject(int projectId);

    int getEmployeeProjectHours(int employeeId, int projectId) const;
    int getEmployeeTaskHours(int employeeId, int projectId, int taskId) const;

    static bool roleMatchesSDLCStage(const QString& employeePosition,
                                     const QString& projectPhase);
    static bool taskTypeMatchesEmployeeType(const QString& taskType,
                                            const QString& employeeType);

   private:
    void updateTaskAndProjectCosts(Project* projPtr, int taskId, int oldHours,
                                   int newHours, std::shared_ptr<Employee> employee) const;
    bool validateAssignment(std::shared_ptr<Employee> employee,
                           std::shared_ptr<Project> project, const Task& task,
                           int hours) const;
};

