#include "entities/company_task_operations.h"

#include "entities/company.h"

void CompanyTaskOperations::assignEmployeeToTask(Company* company,
                                                 int employeeId, int projectId,
                                                 int taskId, int hours) {
    if (!company) return;
    company->getTaskManager().assignEmployeeToTask(employeeId, projectId,
                                                   taskId, hours);
}

void CompanyTaskOperations::restoreTaskAssignment(Company* company,
                                                  int employeeId, int projectId,
                                                  int taskId, int hours) {
    if (!company) return;
    company->getTaskManager().restoreTaskAssignment(employeeId, projectId,
                                                    taskId, hours);
}

void CompanyTaskOperations::removeEmployeeTaskAssignments(Company* company,
                                                          int employeeId) {
    if (!company) return;
    company->getTaskManager().removeEmployeeTaskAssignments(employeeId);
}

void CompanyTaskOperations::recalculateEmployeeHours(Company* company) {
    if (!company) return;
    company->getTaskManager().recalculateEmployeeHours();
}

void CompanyTaskOperations::recalculateTaskAllocatedHours(Company* company) {
    if (!company) return;
    company->getTaskManager().recalculateTaskAllocatedHours();
}

void CompanyTaskOperations::fixTaskAssignmentsToCapacity(Company* company) {
    if (!company) return;
    company->getTaskManager().fixTaskAssignmentsToCapacity();
}

void CompanyTaskOperations::autoAssignEmployeesToProject(Company* company,
                                                         int projectId) {
    if (!company) return;
    company->getTaskManager().autoAssignEmployeesToProject(projectId);
}

int CompanyTaskOperations::getEmployeeProjectHours(const Company* company,
                                                   int employeeId,
                                                   int projectId) {
    if (!company) return 0;
    return company->getTaskManager().getEmployeeProjectHours(employeeId,
                                                             projectId);
}

int CompanyTaskOperations::getEmployeeTaskHours(const Company* company,
                                                int employeeId, int projectId,
                                                int taskId) {
    if (!company) return 0;
    return company->getTaskManager().getEmployeeTaskHours(employeeId, projectId,
                                                          taskId);
}

void CompanyTaskOperations::scaleEmployeeTaskAssignments(Company* company,
                                                         int employeeId,
                                                         double scaleFactor) {
    if (!company) return;
    company->getTaskManager().scaleEmployeeTaskAssignments(employeeId,
                                                           scaleFactor);
}

int CompanyTaskOperations::getTaskAssignment(const Company* company,
                                             int employeeId, int projectId,
                                             int taskId) {
    if (!company) return 0;
    return company->getTaskManager().getTaskAssignment(employeeId, projectId,
                                                       taskId);
}

void CompanyTaskOperations::setTaskAssignment(Company* company, int employeeId,
                                              int projectId, int taskId,
                                              int hours) {
    if (!company) return;
    company->getTaskManager().setTaskAssignment(employeeId, projectId, taskId,
                                                hours);
}

void CompanyTaskOperations::addTaskAssignment(Company* company, int employeeId,
                                              int projectId, int taskId,
                                              int hours) {
    if (!company) return;
    company->getTaskManager().addTaskAssignment(employeeId, projectId, taskId,
                                                hours);
}

void CompanyTaskOperations::removeTaskAssignment(Company* company,
                                                 int employeeId, int projectId,
                                                 int taskId) {
    if (!company) return;
    company->getTaskManager().removeTaskAssignment(employeeId, projectId,
                                                   taskId);
}

std::map<std::tuple<int, int, int>, int>
CompanyTaskOperations::getAllTaskAssignments(const Company* company) {
    if (!company) return {};
    return company->getTaskManager().getAllTaskAssignments();
}
