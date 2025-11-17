#pragma once

#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "entities/employee.h"
#include "entities/project.h"

class EmployeeContainer;
class ProjectContainer;

class TaskAssignmentManager {
   private:
    std::map<std::tuple<int, int, int>, int>& taskAssignments;
    EmployeeContainer& employees;
    ProjectContainer& projects;

   public:
    TaskAssignmentManager(std::map<std::tuple<int, int, int>, int>& assignments,
                          EmployeeContainer& empContainer,
                          ProjectContainer& projContainer);

    void assignEmployeeToTask(int employeeId, int projectId, int taskId,
                              int hours);
    void restoreTaskAssignment(int employeeId, int projectId, int taskId,
                               int hours);
    void removeEmployeeTaskAssignments(int employeeId);
    void recalculateEmployeeHours() const;
    void recalculateTaskAllocatedHours() const;
    void fixTaskAssignmentsToCapacity();
    void autoAssignEmployeesToProject(int projectId);
    int getEmployeeProjectHours(int employeeId, int projectId) const;
    int getEmployeeTaskHours(int employeeId, int projectId, int taskId) const;
    void scaleEmployeeTaskAssignments(int employeeId, double scaleFactor);
    int getTaskAssignment(int employeeId, int projectId, int taskId) const;
    void setTaskAssignment(int employeeId, int projectId, int taskId,
                           int hours);
    void addTaskAssignment(int employeeId, int projectId, int taskId,
                           int hours);
    void removeTaskAssignment(int employeeId, int projectId, int taskId);
    std::map<std::tuple<int, int, int>, int> getAllTaskAssignments() const;
};

class CompanyStatistics {
   private:
    const EmployeeContainer& employees;
    const ProjectContainer& projects;

   public:
    CompanyStatistics(const EmployeeContainer& empContainer,
                      const ProjectContainer& projContainer);

    int getEmployeeCount() const;
    int getProjectCount() const;
    double getTotalSalaries() const;
    double getTotalBudget() const;
};
