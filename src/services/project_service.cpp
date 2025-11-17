#include "services/project_service.h"

#include "exceptions/exceptions.h"

ProjectService::ProjectService(Company* company) : company(company) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
}

void ProjectService::addTaskToProject(int projectId, const Task& task) {
    Project* project = company->getProject(projectId);
    if (!project) {
        throw CompanyException("Project not found");
    }
    project->addTask(task);
}

std::vector<Task> ProjectService::getProjectTasks(int projectId) const {
    return company->getProjectTasks(projectId);
}

void ProjectService::recomputeProjectTotals(int projectId) {
    Project* project = company->getProject(projectId);
    if (!project) {
        return;
    }
    project->recomputeTotalsFromTasks();
}
