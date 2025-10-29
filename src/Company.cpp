#include "../include/Company.h"
#include <ostream>
#include <algorithm>

Company::Company(const QString& name, const QString& industry,
                const QString& location, int foundedYear)
    : name(name), industry(industry), location(location), foundedYear(foundedYear) {
}

void Company::addEmployee(std::shared_ptr<Employee> employee) {
    // Check if employee already exists
    auto existing = getEmployee(employee->getId());
    if (existing) {
        throw std::runtime_error("Employee with this ID already exists");
    }
    employees.add(employee);
}

void Company::removeEmployee(int employeeId) {
    employees.remove(employeeId);
}

std::shared_ptr<Employee> Company::getEmployee(int employeeId) const {
    return employees.find(employeeId);
}

std::vector<std::shared_ptr<Employee>> Company::getAllEmployees() const {
    return employees.getAll();
}

void Company::addProject(const Project& project) {
    auto existing = getProject(project.getId());
    if (existing) {
        throw std::runtime_error("Project with this ID already exists");
    }
    projects.add(std::make_shared<Project>(project));
}

void Company::removeProject(int projectId) {
    projects.remove(projectId);
}

Project* Company::getProject(int projectId) const {
    auto result = projects.find(projectId);
    if (result) {
        // WARNING: Returning raw pointer from shared_ptr - caller should not delete this
        // This is necessary for backward compatibility with existing code
        return result.get();
    }
    return nullptr;
}

std::vector<Project> Company::getAllProjects() const {
    std::vector<Project> projList;
    for (const auto& item : projects.getAll()) {
        if (item) {
            projList.push_back(*item);
        }
    }
    return projList;
}

int Company::getEmployeeCount() const {
    return static_cast<int>(employees.size());
}

int Company::getProjectCount() const {
    return static_cast<int>(projects.size());
}

double Company::getTotalSalaries() const {
    double total = 0.0;
    auto empList = getAllEmployees();
    for (const auto& emp : empList) {
        if (emp) {
            total += emp->getSalary();
        }
    }
    return total;
}

double Company::getTotalBudget() const {
    double total = 0.0;
    auto projList = getAllProjects();
    for (const auto& proj : projList) {
        total += proj.getBudget();
    }
    return total;
}

QString Company::getCompanyInfo() const {
    return QString("Company: %1\nIndustry: %2\nLocation: %3\nFounded: %4\nEmployees: %5\nProjects: %6")
        .arg(name).arg(industry).arg(location).arg(foundedYear)
        .arg(getEmployeeCount()).arg(getProjectCount());
}

bool Company::operator==(const Company& other) const {
    return name == other.name && location == other.location;
}

Company& Company::operator+=(const Project& project) {
    addProject(project);
    return *this;
}

std::ostream& operator<<(std::ostream& os, const Company& company) {
    os << company.name.toStdString() << " - " << company.industry.toStdString()
       << " (" << company.location.toStdString() << ")";
    return os;
}


