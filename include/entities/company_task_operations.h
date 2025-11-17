#pragma once

#include <map>
#include <tuple>
#include <vector>

class Company;
class Task;

class CompanyTaskOperations {
   public:
    static void assignEmployeeToTask(Company* company, int employeeId,
                                     int projectId, int taskId, int hours);
    static void restoreTaskAssignment(Company* company, int employeeId,
                                      int projectId, int taskId, int hours);
    static void removeEmployeeTaskAssignments(Company* company, int employeeId);
    static void recalculateEmployeeHours(Company* company);
    static void recalculateTaskAllocatedHours(Company* company);
    static void fixTaskAssignmentsToCapacity(Company* company);
    static void autoAssignEmployeesToProject(Company* company, int projectId);
    static int getEmployeeProjectHours(const Company* company, int employeeId,
                                       int projectId);
    static int getEmployeeTaskHours(const Company* company, int employeeId,
                                    int projectId, int taskId);
    static void scaleEmployeeTaskAssignments(Company* company, int employeeId,
                                             double scaleFactor);
    static int getTaskAssignment(const Company* company, int employeeId,
                                 int projectId, int taskId);
    static void setTaskAssignment(Company* company, int employeeId,
                                  int projectId, int taskId, int hours);
    static void addTaskAssignment(Company* company, int employeeId,
                                  int projectId, int taskId, int hours);
    static void removeTaskAssignment(Company* company, int employeeId,
                                     int projectId, int taskId);
    static std::map<std::tuple<int, int, int>, int> getAllTaskAssignments(
        const Company* company);
};
