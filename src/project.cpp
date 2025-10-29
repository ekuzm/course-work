#include "../include/project.h"

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

Project::Project(int projectId, QString projectName, QString projectDescription,
                 QString projectStatus, QDate projectStartDate,
                 QDate projectEndDate, double projectBudget,
                 QString projectClientName)
    : id(projectId),
      name(std::move(projectName)),
      description(std::move(projectDescription)),
      status(std::move(projectStatus)),
      startDate(std::move(projectStartDate)),
      endDate(std::move(projectEndDate)),
      budget(projectBudget),
      clientName(std::move(projectClientName)) {
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

bool Project::operator<(const Project& otherProject) const {
    if (budget != otherProject.budget) {
        return budget < otherProject.budget;
    }
    return id < otherProject.id;
}

std::ostream& operator<<(std::ostream& outputStream, const Project& project) {
    outputStream << project.id << " - " << project.name.toStdString() << " ("
                 << project.status.toStdString() << ")";
    return outputStream;
}

int Project::getDaysDuration() const { return startDate.daysTo(endDate); }

bool Project::isActive() const {
    QDate currentDate = QDate::currentDate();
    return (status == "Active" || status == "In Progress") &&
           (currentDate >= startDate &&
            (endDate.isNull() || currentDate <= endDate));
}
