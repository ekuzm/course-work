#include "../include/Project.h"
#include <ostream>

Project::Project(int id, const QString& name, const QString& description,
                 const QString& status, const QDate& startDate, const QDate& endDate,
                 double budget, const QString& clientName)
    : id(id), name(name), description(description), status(status),
      startDate(startDate), endDate(endDate), budget(budget), clientName(clientName) {
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

bool Project::operator==(const Project& other) const {
    return id == other.id && name == other.name;
}

bool Project::operator<(const Project& other) const {
    if (budget != other.budget) {
        return budget < other.budget;
    }
    return id < other.id;
}

std::ostream& operator<<(std::ostream& os, const Project& proj) {
    os << proj.id << " - " << proj.name.toStdString() 
       << " (" << proj.status.toStdString() << ")";
    return os;
}

int Project::getDaysDuration() const {
    return startDate.daysTo(endDate);
}

bool Project::isActive() const {
    QDate today = QDate::currentDate();
    return (status == "Active" || status == "In Progress") && 
           (today >= startDate && (endDate.isNull() || today <= endDate));
}


