#include "../include/company.h"

#include <algorithm>
#include <ostream>
#include <ranges>
#include <utility>

CompanyException::CompanyException(QString msg) : message(std::move(msg)) {}

const char* CompanyException::what() const noexcept {
    return message.toLocal8Bit().constData();
}

// EmployeeContainer implementation
void EmployeeContainer::add(std::shared_ptr<Employee> employee) {
    employees.push_back(employee);
}

void EmployeeContainer::remove(int employeeId) {
    auto removed = std::ranges::remove_if(
        employees,
        [employeeId](const std::shared_ptr<Employee>& employee) {
            return employee && employee->getId() == employeeId;
        });
    employees.erase(removed.begin(), removed.end());
}

std::shared_ptr<Employee> EmployeeContainer::find(int employeeId) const {
    for (const auto& employee : employees) {
        if (employee && employee->getId() == employeeId) return employee;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Employee>> EmployeeContainer::getAll() const {
    return employees;
}

size_t EmployeeContainer::size() const { return employees.size(); }

void EmployeeContainer::clear() { employees.clear(); }

EmployeeContainer::iterator EmployeeContainer::begin() {
    return employees.begin();
}

EmployeeContainer::iterator EmployeeContainer::end() { return employees.end(); }

EmployeeContainer::const_iterator EmployeeContainer::begin() const {
    return employees.begin();
}

EmployeeContainer::const_iterator EmployeeContainer::end() const {
    return employees.end();
}

// ProjectContainer implementation
void ProjectContainer::add(std::shared_ptr<Project> project) {
    projects.push_back(project);
}

void ProjectContainer::remove(int projectId) {
    auto removed = std::ranges::remove_if(
        projects,
        [projectId](const std::shared_ptr<Project>& project) {
            return project && project->getId() == projectId;
        });
    projects.erase(removed.begin(), removed.end());
}

std::shared_ptr<Project> ProjectContainer::find(int projectId) const {
    for (const auto& project : projects) {
        if (project && project->getId() == projectId) return project;
    }
    return nullptr;
}

std::vector<std::shared_ptr<Project>> ProjectContainer::getAll() const {
    return projects;
}

size_t ProjectContainer::size() const { return projects.size(); }

void ProjectContainer::clear() { projects.clear(); }

// Company implementation
QString Company::getName() const { return name; }

QString Company::getIndustry() const { return industry; }

QString Company::getLocation() const { return location; }

int Company::getFoundedYear() const { return foundedYear; }

EmployeeContainer::iterator Company::employeeBegin() {
    return employees.begin();
}

EmployeeContainer::iterator Company::employeeEnd() { return employees.end(); }

const EmployeeContainer& Company::getEmployeeContainer() const {
    return employees;
}

const ProjectContainer& Company::getProjectContainer() const {
    return projects;
}

Company::Company(QString companyName, QString companyIndustry,
                 QString companyLocation, int companyFoundedYear)
    : name(std::move(companyName)),
      industry(std::move(companyIndustry)),
      location(std::move(companyLocation)),
      foundedYear(companyFoundedYear) {}

void Company::addEmployee(std::shared_ptr<Employee> employee) {
    // Check if employee already exists
    if (auto existing = getEmployee(employee->getId()); existing) {
        throw CompanyException("Employee with this ID already exists");
    }
    employees.add(employee);
}

void Company::removeEmployee(int employeeId) { employees.remove(employeeId); }

std::shared_ptr<Employee> Company::getEmployee(int employeeId) const {
    return employees.find(employeeId);
}

std::vector<std::shared_ptr<Employee>> Company::getAllEmployees() const {
    return employees.getAll();
}

void Company::addProject(const Project& project) {
    if (const auto* existing = getProject(project.getId());
        existing != nullptr) {
        throw CompanyException("Project with this ID already exists");
    }
    projects.add(std::make_shared<Project>(project));
}

void Company::removeProject(int projectId) { projects.remove(projectId); }

const Project* Company::getProject(int projectId) const {
    auto result = projects.find(projectId);
    if (result) {
        // WARNING: Returning raw pointer from shared_ptr - caller should not
        // delete this This is necessary for backward compatibility with
        // existing code
        return result.get();
    }
    return nullptr;
}

std::vector<Project> Company::getAllProjects() const {
    std::vector<Project> projectList;
    for (const auto& projectItem : projects.getAll()) {
        if (projectItem) {
            projectList.push_back(*projectItem);
        }
    }
    return projectList;
}

int Company::getEmployeeCount() const {
    return static_cast<int>(employees.size());
}

int Company::getProjectCount() const {
    return static_cast<int>(projects.size());
}

double Company::getTotalSalaries() const {
    double total = 0.0;
    auto employeeList = getAllEmployees();
    for (const auto& employee : employeeList) {
        if (employee) {
            total += employee->getSalary();
        }
    }
    return total;
}

double Company::getTotalBudget() const {
    double total = 0.0;
    auto projectList = getAllProjects();
    for (const auto& project : projectList) {
        total += project.getBudget();
    }
    return total;
}

QString Company::getCompanyInfo() const {
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

bool Company::operator==(const Company& otherCompany) const {
    return name == otherCompany.name && location == otherCompany.location;
}

Company& Company::operator+=(const Project& project) {
    addProject(project);
    return *this;
}

