#ifndef COMPANY_H
#define COMPANY_H

#include <QString>
#include <memory>
#include <vector>

#include "derived_employees.h"
#include "employee.h"
#include "project.h"

class MainWindow;

class EmployeeContainer {
   private:
    std::vector<std::shared_ptr<Employee>> employees;

   public:
    void add(std::shared_ptr<Employee> employee);
    void remove(int employeeId);
    std::shared_ptr<Employee> find(int employeeId) const;
    std::vector<std::shared_ptr<Employee>> getAll() const;
    size_t size() const;
    void clear();

    using iterator = std::vector<std::shared_ptr<Employee>>::iterator;
    using const_iterator =
        std::vector<std::shared_ptr<Employee>>::const_iterator;

    iterator begin();
    iterator end();
    const_iterator begin() const;
    const_iterator end() const;
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
    void clear();
};

class Company {
   private:
    QString name;
    QString industry;
    QString location;
    int foundedYear;

    EmployeeContainer employees;
    ProjectContainer projects;

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
    Project* getProject(int projectId) const;
    std::vector<Project> getAllProjects() const;

    int getEmployeeCount() const;
    int getProjectCount() const;
    double getTotalSalaries() const;
    double getTotalBudget() const;
    QString getCompanyInfo() const;

    bool operator==(const Company& otherCompany) const;
    Company& operator+=(const Project& project);
    friend std::ostream& operator<<(std::ostream& outputStream,
                                    const Company& company);
    friend class MainWindow;

    EmployeeContainer::iterator employeeBegin();
    EmployeeContainer::iterator employeeEnd();
    const EmployeeContainer& getEmployeeContainer() const;
    const ProjectContainer& getProjectContainer() const;
};

#endif  // COMPANY_H
