#include "../include/project.h"

#include <compare>
#include <ostream>
#include <utility>

ProjectException::ProjectException(QString msg) : message(std::move(msg)) {}

const char* ProjectException::what() const noexcept {
    return message.toLocal8Bit().constData();
}

int Project::getId() const { return id; }

QString Project::getName() const { return name; }

QString Project::getDescription() const { return description; }

QString Project::getStatus() const { return status; }

QDate Project::getStartDate() const { return startDate; }

QDate Project::getEndDate() const { return endDate; }

double Project::getBudget() const { return budget; }

QString Project::getClientName() const { return clientName; }

Project::Project(int projectId, const ProjectParams& params)
    : id(projectId),
      name(params.name),
      description(params.description),
      status(params.status),
      startDate(params.startDate),
      endDate(params.endDate),
      budget(params.budget),
      clientName(params.clientName) {
    if (name.isEmpty()) {
        throw ProjectException("Project name cannot be empty");
    }
    if (budget < 0) {
        throw ProjectException("Budget cannot be negative");
    }
    if (endDate < startDate) {
        throw ProjectException("End date cannot be before start date");
    }
}

void Project::setStatus(const QString& newStatus) {
    if (newStatus.isEmpty()) {
        throw ProjectException("Status cannot be empty");
    }
    status = newStatus;
}

void Project::setBudget(double newBudget) {
    if (newBudget < 0) {
        throw ProjectException("Budget cannot be negative");
    }
    budget = newBudget;
}

bool Project::operator==(const Project& otherProject) const {
    return id == otherProject.id && name == otherProject.name;
}

std::partial_ordering Project::operator<=>(
    const Project& otherProject) const {
    if (auto cmp = budget <=> otherProject.budget; cmp != 0) {
        return cmp;
    }
    return id <=> otherProject.id;
}

int Project::getDaysDuration() const { return startDate.daysTo(endDate); }

bool Project::isActive() const {
    QDate currentDate = QDate::currentDate();
    return (status == "Active" || status == "In Progress") &&
           (currentDate >= startDate &&
            (endDate.isNull() || currentDate <= endDate));
}
