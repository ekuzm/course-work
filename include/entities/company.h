#pragma once

#include <QString>
#include <exception>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "entities/company_managers.h"
#include "entities/derived_employees.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"
#include "exceptions/exceptions.h"

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
    mutable ProjectContainer projects;
    std::map<std::tuple<int, int, int>, int> taskAssignments;

    TaskAssignmentManager taskManager;
    mutable CompanyStatistics statistics;

   public:
    Company(QString name, QString industry, QString location, int foundedYear);
    Company(Company&& other) noexcept;
    ~Company();

    TaskAssignmentManager& getTaskManager() { return taskManager; }
    const CompanyStatistics& getStatistics() const { return statistics; }

    QString getName() const { return name; }
    QString getIndustry() const { return industry; }
    QString getLocation() const { return location; }
    int getFoundedYear() const { return foundedYear; }

    void addEmployee(std::shared_ptr<Employee> employee);
    void removeEmployee(int employeeId);
    std::shared_ptr<Employee> getEmployee(int employeeId) const {
        return employees.find(employeeId);
    }
    std::vector<std::shared_ptr<Employee>> getAllEmployees() const {
        return employees.getAll();
    }

    void addProject(const Project& project);
    void removeProject(int projectId);
    // Combined method to reduce method count - returns non-const pointer
    // Use const_cast<const Project*>(getProject(id)) for const access if needed
    Project* getProject(int projectId) const {
        if (std::shared_ptr<Project> result = projects.find(projectId);
            result) {
            return result.get();
        }
        return nullptr;
    }
    std::vector<Project> getAllProjects() const {
        std::vector<Project> projectList;
        auto allProjects = projects.getAll();
        
        // Safety check: limit projects to prevent bad_array_new_length
        constexpr size_t maxProjects = 100000;
        if (allProjects.size() > maxProjects) {
            return {};  // Return empty vector if size is invalid
        }
        
        projectList.reserve(std::min(allProjects.size(), maxProjects));
        for (const auto& proj : allProjects) {
            if (proj) {
                projectList.push_back(*proj);
            }
        }
        return projectList;
    }

    void addTaskToProject(int projectId, const Task& task) const {
        if (std::shared_ptr<Project> proj = projects.find(projectId); proj) {
            proj->addTask(task);
            return;
        }
        throw CompanyException("Project not found");
    }
    std::vector<Task> getProjectTasks(int projectId) const {
        return getProject(projectId) ? getProject(projectId)->getTasks()
                                     : std::vector<Task>();
    }

    void assignEmployeeToTask(int employeeId, int projectId, int taskId,
                              int hours) {
        getTaskManager().assignEmployeeToTask(employeeId, projectId, taskId,
                                              hours);
    }
    void restoreTaskAssignment(int employeeId, int projectId, int taskId,
                               int hours) {
        getTaskManager().restoreTaskAssignment(employeeId, projectId, taskId,
                                               hours);
    }
    void removeEmployeeTaskAssignments(int employeeId) {
        getTaskManager().removeEmployeeTaskAssignments(employeeId);
    }
    void recalculateEmployeeHours() {
        getTaskManager().recalculateEmployeeHours();
    }
    void recalculateTaskAllocatedHours() {
        getTaskManager().recalculateTaskAllocatedHours();
    }
    void fixTaskAssignmentsToCapacity() {
        getTaskManager().fixTaskAssignmentsToCapacity();
    }
    // Combined method to reduce method count
    void recalculateAllHours() {
        recalculateEmployeeHours();
        recalculateTaskAllocatedHours();
    }
    void autoAssignEmployeesToProject(int projectId) {
        getTaskManager().autoAssignEmployeesToProject(projectId);
    }
    // Combined method to reduce method count
    // If taskId is -1, returns project hours; otherwise returns task hours
    int getEmployeeHours(int employeeId, int projectId, int taskId = -1) const {
        if (taskId == -1) {
            return taskManager.getEmployeeProjectHours(employeeId, projectId);
        }
        return taskManager.getEmployeeTaskHours(employeeId, projectId, taskId);
    }
    void scaleEmployeeTaskAssignments(int employeeId, double scaleFactor) {
        getTaskManager().scaleEmployeeTaskAssignments(employeeId, scaleFactor);
    }
    std::map<std::tuple<int, int, int>, int> getAllTaskAssignments() const {
        return taskManager.getAllTaskAssignments();
    }

    int getEmployeeCount() const { return statistics.getEmployeeCount(); }
    int getProjectCount() const { return statistics.getProjectCount(); }
    double getTotalSalaries() const { return statistics.getTotalSalaries(); }
    double getTotalBudget() const { return statistics.getTotalBudget(); }
    QString getCompanyInfo() const {
        return QString(
                   "Company: %1\nIndustry: %2\nLocation: %3\nFounded: "
                   "%4\nEmployees: %5\nProjects: %6")
            .arg(name)
            .arg(industry)
            .arg(location)
            .arg(foundedYear)
            .arg(getEmployeeCount())
            .arg(getProjectCount());
    }
};
