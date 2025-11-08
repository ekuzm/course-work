#pragma once

#include <QString>
#include <memory>
#include <vector>

class Project;
class Employee;
class Company;

class HtmlGenerator {
   public:
    static QString generateProjectDetailHtml(const Project& project, const Company* company);
    static QString generateProjectAssignmentsHtml(const Project& project, const Company* company);
    static QString generateEmployeeHistoryHtml(const Employee& employee, const Company* company,
                                              const std::vector<const Project*>& employeeProjects);
};





