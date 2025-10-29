#ifndef COMPANY_H
#define COMPANY_H

#include "Employee.h"
#include "DerivedEmployees.h"
#include "Project.h"
#include <vector>
#include <memory>
#include <QString>

// Forward declarations
class MainWindow;

// Container for employees
class EmployeeContainer {
private:
    std::vector<std::shared_ptr<Employee>> employees;

public:
    void add(std::shared_ptr<Employee> emp) {
        employees.push_back(emp);
    }

    void remove(int id) {
        employees.erase(
            std::remove_if(employees.begin(), employees.end(),
                [id](const std::shared_ptr<Employee>& emp) {
                    return emp && emp->getId() == id;
                }),
            employees.end()
        );
    }

    std::shared_ptr<Employee> find(int id) const {
        for (const auto& emp : employees) {
            if (emp && emp->getId() == id) return emp;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Employee>> getAll() const {
        return employees;
    }

    size_t size() const { return employees.size(); }
    void clear() { employees.clear(); }

    using iterator = std::vector<std::shared_ptr<Employee>>::iterator;
    using const_iterator = std::vector<std::shared_ptr<Employee>>::const_iterator;

    iterator begin() { return employees.begin(); }
    iterator end() { return employees.end(); }
    const_iterator begin() const { return employees.begin(); }
    const_iterator end() const { return employees.end(); }
};

// Container for projects
class ProjectContainer {
private:
    std::vector<std::shared_ptr<Project>> projects;

public:
    void add(std::shared_ptr<Project> proj) {
        projects.push_back(proj);
    }

    void remove(int id) {
        projects.erase(
            std::remove_if(projects.begin(), projects.end(),
                [id](const std::shared_ptr<Project>& proj) {
                    return proj && proj->getId() == id;
                }),
            projects.end()
        );
    }

    std::shared_ptr<Project> find(int id) const {
        for (const auto& proj : projects) {
            if (proj && proj->getId() == id) return proj;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Project>> getAll() const {
        return projects;
    }

    size_t size() const { return projects.size(); }
    void clear() { projects.clear(); }
};

// Company class with friend functions
class Company {
private:
    QString name;
    QString industry;
    QString location;
    int foundedYear;
    
    EmployeeContainer employees;
    ProjectContainer projects;

public:
    Company(const QString& name, const QString& industry, 
            const QString& location, int foundedYear);

    // Getters
    QString getName() const { return name; }
    QString getIndustry() const { return industry; }
    QString getLocation() const { return location; }
    int getFoundedYear() const { return foundedYear; }

    // Employee management
    void addEmployee(std::shared_ptr<Employee> employee);
    void removeEmployee(int employeeId);
    std::shared_ptr<Employee> getEmployee(int employeeId) const;
    std::vector<std::shared_ptr<Employee>> getAllEmployees() const;
    
    // Project management
    void addProject(const Project& project);
    void removeProject(int projectId);
    Project* getProject(int projectId) const;
    std::vector<Project> getAllProjects() const;

    // Statistics
    int getEmployeeCount() const;
    int getProjectCount() const;
    double getTotalSalaries() const;
    double getTotalBudget() const;
    QString getCompanyInfo() const;

    // Operator overloading
    bool operator==(const Company& other) const;
    Company& operator+=(const Project& project);
    friend std::ostream& operator<<(std::ostream& os, const Company& company);
    friend class MainWindow;

    // STL iterator access
    EmployeeContainer::iterator employeeBegin() { return employees.begin(); }
    EmployeeContainer::iterator employeeEnd() { return employees.end(); }
    const EmployeeContainer& getEmployeeContainer() const { return employees; }
    const ProjectContainer& getProjectContainer() const { return projects; }
};

#endif // COMPANY_H
