#include "employee.h"

#include <stdexcept>
#include <utility>

int Employee::getId() const { return id; }

QString Employee::getName() const { return name; }

QString Employee::getPosition() const { return position; }

double Employee::getSalary() const { return salary; }

QString Employee::getDepartment() const { return department; }

Employee::Employee(int employeeId, QString employeeName,
                   QString employeePosition, double employeeSalary,
                   QString employeeDepartment, double employmentRateParam,
                   int weeklyCapacity)
    : id(employeeId),
      name(std::move(employeeName)),
      position(std::move(employeePosition)),
      salary(employeeSalary),
      department(std::move(employeeDepartment)),
      employmentRate(employmentRateParam),
      weeklyHoursCapacity(
          static_cast<int>(weeklyCapacity * employmentRateParam)) {
    if (name.isEmpty()) {
        throw EmployeeException("Employee name cannot be empty");
    }
    if (salary < 0) {
        throw EmployeeException("Salary cannot be negative");
    }
    if (weeklyCapacity <= 0 || weeklyCapacity > 168) {
        throw EmployeeException(
            "Weekly capacity must be between 1 and 168 hours");
    }
    if (employmentRateParam <= 0 || employmentRateParam > 1.0) {
        throw EmployeeException("Employment rate must be between 0 and 1.0");
    }
}

QString Employee::getDetails() const {
    return QString("ID: %1, Name: %2, Position: %3, Salary: %4, Department: %5")
        .arg(id)
        .arg(name)
        .arg(position)
        .arg(salary)
        .arg(department);
}

double Employee::calculateBonus() const { return 0.0; }

bool Employee::getIsActive() const { return isActive; }

double Employee::getEmploymentRate() const { return employmentRate; }

int Employee::getWeeklyHoursCapacity() const { return weeklyHoursCapacity; }

int Employee::getCurrentWeeklyHours() const { return currentWeeklyHours; }

bool Employee::isAvailable(int requestedHours) const {
    return isActive &&
           (currentWeeklyHours + requestedHours <= weeklyHoursCapacity);
}

int Employee::getAvailableHours() const {
    if (!isActive) return 0;
    return weeklyHoursCapacity - currentWeeklyHours;
}

void Employee::addWeeklyHours(int hours) {
    if (hours < 0) {
        throw EmployeeException("Cannot add negative hours");
    }
    if (currentWeeklyHours + hours > weeklyHoursCapacity) {
        throw EmployeeException("Cannot exceed weekly capacity");
    }
    currentWeeklyHours += hours;
}

void Employee::removeWeeklyHours(int hours) {
    if (hours < 0) {
        throw EmployeeException("Cannot remove negative hours");
    }
    if (currentWeeklyHours < hours) {
        throw EmployeeException("Cannot remove more hours than allocated");
    }
    currentWeeklyHours -= hours;
}

void Employee::setIsActive(bool active) {
    if (!active && currentWeeklyHours > 0) {
        throw EmployeeException(
            "Cannot deactivate employee with active assignments");
    }
    isActive = active;
}

const std::vector<int>& Employee::getAssignedProjects() const {
    return assignedProjects;
}

void Employee::addAssignedProject(int projectId) {
    if (!isAssignedToProject(projectId)) {
        assignedProjects.push_back(projectId);
    }
}

bool Employee::isAssignedToProject(int projectId) const {
    return std::ranges::find(assignedProjects, projectId) !=
           assignedProjects.end();
}

void Employee::removeAssignedProject(int projectId) {
    auto it = std::ranges::find(assignedProjects, projectId);
    if (it != assignedProjects.end()) {
        assignedProjects.erase(it);
    }
}
