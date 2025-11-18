#include "services/project_service.h"

#include "exceptions/exceptions.h"

void ProjectService::addTaskToProject(const Company* company, int projectId, const Task& task) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
    Project* project = company->getProject(projectId);
    if (!project) {
        throw CompanyException("Project not found");
    }
    project->addTask(task);
}

std::vector<Task> ProjectService::getProjectTasks(const Company* company, int projectId) {
    if (!company) {
        return {};
    }
    return company->getProjectTasks(projectId);
}

void ProjectService::recomputeProjectTotals(const Company* company, int projectId) {
    if (!company) {
        return;
    }
    Project* project = company->getProject(projectId);
    if (!project) {
        return;
    }
    project->recomputeTotalsFromTasks();
}
