#pragma once

#include <QString>
#include <exception>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "derived_employees.h"
#include "employee.h"
#include "exceptions.h"
#include "project.h"
#include "task.h"

class EmployeeContainer {
   private:
    std::vector<std::shared_ptr<Employee>> employees;

   public:
    void add(std::shared_ptr<Employee> employee);
    void remove(int employeeId);
    std::shared_ptr<Employee> find(int employeeId) const;
    std::vector<std::shared_ptr<Employee>> getAll() const;
    size_t size() const;
};

class ProjectContainer {
   private:
    std::vector<std::shared_ptr<Project>> projects;

   public:
    void add(std::shared_ptr<Project> project);
    void remove(int projectId);
    std::shared_ptr<Project> find(int projectId) const;
    std::vector<std::shared_ptr<Project>> getAll() const;
    size_t size() const;
};

class Company {
   private:
    QString name;
    QString industry;
    QString location;
    int foundedYear;

    EmployeeContainer employees;
    ProjectContainer projects;
    std::map<std::tuple<int, int, int>, int> taskAssignments;

   public:
    Company(QString name, QString industry, QString location, int foundedYear);

    QString getName() const;
    QString getIndustry() const;
    QString getLocation() const;
    int getFoundedYear() const;

    void addEmployee(std::shared_ptr<Employee> employee);
    void removeEmployee(int employeeId);
    std::shared_ptr<Employee> getEmployee(int employeeId) const;
    std::vector<std::shared_ptr<Employee>> getAllEmployees() const;

    void addProject(const Project& project);
    void removeProject(int projectId);
    const Project* getProject(int projectId) const;
    std::vector<Project> getAllProjects() const;

    void addTaskToProject(int projectId, const Task& task);
    std::vector<Task> getProjectTasks(int projectId) const;

    void assignEmployeeToTask(int employeeId, int projectId, int taskId,
                              int hours);
    void restoreTaskAssignment(int employeeId, int projectId, int taskId,
                               int hours);
    void removeEmployeeTaskAssignments(int employeeId);
    void recalculateEmployeeHours();
    void autoAssignEmployeesToProject(int projectId);
    int getEmployeeProjectHours(int employeeId, int projectId) const;
    int getEmployeeTaskHours(int employeeId, int projectId, int taskId) const;

    int getEmployeeCount() const;
    int getProjectCount() const;
    double getTotalSalaries() const;
    double getTotalBudget() const;
    QString getCompanyInfo() const;
};
