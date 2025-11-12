#include "project.h"

#include <algorithm>
#include <utility>

int Project::getId() const { return id; }

QString Project::getName() const { return name; }

QString Project::getDescription() const { return description; }

QString Project::getPhase() const { return phase; }

QDate Project::getStartDate() const { return startDate; }

QDate Project::getEndDate() const { return endDate; }

double Project::getBudget() const { return budget; }

QString Project::getClientName() const { return clientName; }

Project::Project(int projectId, const QString& name, const QString& description,
                 const QString& phase, const QDate& startDate,
                 const QDate& endDate, double budget, const QString& clientName,
                 int estimatedHours)
    : id(projectId),
      name(name),
      description(description),
      phase(phase),
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

int Project::getPhaseOrder(const QString& phaseName) {
    if (phaseName == "Analysis") return 0;
    if (phaseName == "Planning") return 1;
    if (phaseName == "Design") return 2;
    if (phaseName == "Development") return 3;
    if (phaseName == "Testing") return 4;
    if (phaseName == "Deployment") return 5;
    if (phaseName == "Maintenance") return 6;
    if (phaseName == "Completed") return 7;
    return -1;
}

void Project::setPhase(const QString& newPhase) {
    if (newPhase.isEmpty()) {
        throw ProjectException("phase cannot be empty");
    }
    int currentPhaseOrder = getPhaseOrder(phase);
    int newPhaseOrder = getPhaseOrder(newPhase);

    if (currentPhaseOrder >= 0 && newPhaseOrder >= 0 &&
        newPhaseOrder < currentPhaseOrder) {
        throw ProjectException(
            QString("Cannot set phase to '%1' because current phase '%2' is "
                    "already later in the project lifecycle.\n\n"
                    "Phase order: Analysis → Planning → Design → Development → "
                    "Testing → Deployment → Maintenance → Completed")
                .arg(newPhase, phase));
    }
    phase = newPhase;
}

void Project::setBudget(double newBudget) {
    if (newBudget < 0) {
        throw ProjectException("Budget cannot be negative");
    }
    budget = newBudget;
}

bool Project::isActive() const {
    QDate currentDate = QDate::currentDate();

    bool isActivePhase =
        (phase == "Analysis" || phase == "Planning" || phase == "Design" ||
         phase == "Development" || phase == "Testing" ||
         phase == "Deployment" || phase == "Maintenance");
    return isActivePhase && (currentDate >= startDate &&
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
