#pragma once

#include <QString>
#include <memory>
#include <vector>

class Project;
class Employee;
class Company;
class Task;

class HtmlGenerator {
   public:
    static QString generateProjectDetailHtml(const Project& project,
                                             const Company* company);
    static QString generateProjectAssignmentsHtml(const Project& project,
                                                  const Company* company);
    static QString generateEmployeeHistoryHtml(
        const Employee& employee, const Company* company,
        const std::vector<const Project*>& employeeProjects);

   private:
    static QString generateTeamTableRow(
        const std::shared_ptr<Employee>& employee, const Project& project,
        const Company* company, const std::vector<Task>& tasks, int rowNumber,
        bool projectCompleted);
    static QString formatPercentText(double value);
    static QString getEmployeeStatus(const std::shared_ptr<Employee>& employee,
                                     const Project& project,
                                     bool projectCompleted);
    static QString getEmployeeTasksDisplay(
        const std::shared_ptr<Employee>& employee, const Project& project,
        const Company* company, const std::vector<Task>& tasks);
};
