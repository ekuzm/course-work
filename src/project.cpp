#include "project.h"

#include <algorithm>
#include <utility>

int Project::getId() const { return id; }

QString Project::getName() const { return name; }

QString Project::getDescription() const { return description; }

QString Project::getStatus() const { return status; }

QDate Project::getStartDate() const { return startDate; }

QDate Project::getEndDate() const { return endDate; }

double Project::getBudget() const { return budget; }

QString Project::getClientName() const { return clientName; }

Project::Project(int projectId, const QString& name, const QString& description,
                 const QString& status, const QDate& startDate,
                 const QDate& endDate, double budget, const QString& clientName,
                 int estimatedHours)
    : id(projectId),
      name(name),
      description(description),
      status(status),
      startDate(startDate),
      endDate(endDate),
      budget(budget),
      clientName(clientName),
      initialEstimatedHours(estimatedHours) {
    if (name.isEmpty()) {
        throw ProjectException("Project name cannot be empty");
    }
    if (budget < 0) {
        throw ProjectException("Budget cannot be negative");
    }
    if (endDate < startDate) {
        throw ProjectException("End date cannot be before start date");
    }
    if (initialEstimatedHours < 0) {
        throw ProjectException("Estimated hours cannot be negative");
    }

    if (initialEstimatedHours > 0 && !endDate.isNull()) {
        int daysDuration = startDate.daysTo(endDate);
        int maxHoursInDeadline = daysDuration * 8;
        if (initialEstimatedHours > maxHoursInDeadline) {
            throw ProjectException(
                QString("Estimated hours (%1h) exceeds deadline capacity (%2h) "
                        "for %3 days")
                    .arg(initialEstimatedHours)
                    .arg(maxHoursInDeadline)
                    .arg(daysDuration));
        }
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

bool Project::isActive() const {
    QDate currentDate = QDate::currentDate();

    bool isActiveStatus =
        (status == "Analysis" || status == "Planning" || status == "Design" ||
         status == "Development" || status == "Testing" ||
         status == "Deployment" || status == "Maintenance");
    return isActiveStatus && (currentDate >= startDate &&
                              (endDate.isNull() || currentDate <= endDate));
}

int Project::getEstimatedHours() const {
    int tasksTotal = getTasksEstimatedTotal();
    return std::max(initialEstimatedHours, tasksTotal);
}

int Project::getInitialEstimatedHours() const { return initialEstimatedHours; }

int Project::getAllocatedHours() const { return allocatedHours; }

double Project::getEmployeeCosts() const { return employeeCosts; }

const std::vector<Task>& Project::getTasks() const { return tasks; }

std::vector<Task>& Project::getTasks() { return tasks; }

void Project::setEstimatedHours(int hours) {
    if (hours < 0) {
        throw ProjectException("Estimated hours cannot be negative");
    }
    initialEstimatedHours = hours;

    if (initialEstimatedHours > 0 && !endDate.isNull()) {
        int daysDuration = startDate.daysTo(endDate);
        int maxHoursInDeadline = daysDuration * 8;

        int currentEstimated = getEstimatedHours();
        if (currentEstimated > maxHoursInDeadline) {
            throw ProjectException(
                QString("Estimated hours (%1h) exceeds deadline capacity (%2h)")
                    .arg(currentEstimated)
                    .arg(maxHoursInDeadline));
        }
    }
}

void Project::setAllocatedHours(int hours) {
    if (hours < 0) {
        throw ProjectException("Allocated hours cannot be negative");
    }
    allocatedHours = hours;
}

void Project::addEmployeeCost(double cost) {
    if (cost < 0) {
        throw ProjectException("Employee cost cannot be negative");
    }
    employeeCosts += cost;
}

void Project::removeEmployeeCost(double cost) {
    if (cost < 0) {
        throw ProjectException("Employee cost cannot be negative");
    }
    if (employeeCosts < cost) {
        throw ProjectException("Cannot remove more cost than allocated");
    }
    employeeCosts -= cost;
}

void Project::addTask(const Task& task) {
    int currentTasksTotal = getTasksEstimatedTotal();
    int newTasksTotal = currentTasksTotal + task.getEstimatedHours();
    int newEstimated = std::max(initialEstimatedHours, newTasksTotal);

    if (newEstimated > 0 && !endDate.isNull()) {
        int daysDuration = startDate.daysTo(endDate);
        int maxHoursInDeadline = daysDuration * 8;
        if (newEstimated > maxHoursInDeadline) {
            throw ProjectException(
                QString("Adding task '%1' (%2h) would exceed deadline capacity "
                        "(%3h). "
                        "Current estimated: %4h, After adding: %5h")
                    .arg(task.getName())
                    .arg(task.getEstimatedHours())
                    .arg(maxHoursInDeadline)
                    .arg(getEstimatedHours())
                    .arg(newEstimated));
        }
    }

    tasks.push_back(task);
    recomputeTotalsFromTasks();
}

void Project::clearTasks() {
    tasks.clear();
    recomputeTotalsFromTasks();
}

int Project::getTasksEstimatedTotal() const {
    int total = 0;
    for (const auto& task : tasks) total += task.getEstimatedHours();
    return total;
}

int Project::getTasksAllocatedTotal() const {
    int total = 0;
    for (const auto& task : tasks) total += task.getAllocatedHours();
    return total;
}

void Project::recomputeTotalsFromTasks() {
    allocatedHours = getTasksAllocatedTotal();
}

int Project::getNextTaskId() const {
    int maxId = 0;
    for (const auto& task : tasks) {
        if (task.getId() > maxId) maxId = task.getId();
    }
    return maxId + 1;
}
