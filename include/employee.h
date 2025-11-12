#pragma once

#include <QString>
#include <memory>
#include <vector>

#include "exceptions.h"

class Employee {
   private:
    int id;
    QString name;
    QString position;
    double salary;
    QString department;
    bool isActive{true};
    double employmentRate;
    int weeklyHoursCapacity;
    int currentWeeklyHours{0};
    std::vector<int> assignedProjects;
    std::vector<int> projectHistory;

   public:
    Employee(int employeeId, QString name, QString position, double salary,
             QString department, double employmentRate = 1.0,
             int weeklyCapacity = 40);
    virtual ~Employee() = default;

    virtual QString getEmployeeType() const = 0;
    virtual QString getDetails() const;
    virtual double calculateBonus() const = 0;

    int getId() const;
    QString getName() const;
    QString getPosition() const;
    double getSalary() const;
    QString getDepartment() const;
    bool getIsActive() const;
    double getEmploymentRate() const;
    int getWeeklyHoursCapacity() const;
    int getCurrentWeeklyHours() const;

    bool isAvailable(int requestedHours) const;
    int getAvailableHours() const;
    void addWeeklyHours(int hours);
    void removeWeeklyHours(int hours);

    const std::vector<int>& getAssignedProjects() const;
    void addAssignedProject(int projectId);
    void removeAssignedProject(int projectId);
    bool isAssignedToProject(int projectId) const;
    const std::vector<int>& getProjectHistory() const;
    void addToProjectHistory(int projectId);

    void setIsActive(bool active);
};
