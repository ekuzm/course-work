#include "../include/display_helper.h"

#include <QTableWidgetItem>
#include <algorithm>
#include <map>

void DisplayHelper::displayEmployees(QTableWidget* employeeTable,
                                      Company* currentCompany) {
    if (currentCompany == nullptr || employeeTable == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(employees.size());

    for (size_t index = 0; index < employees.size(); ++index) {
        const auto& employee = employees[index];
        employeeTable->setItem(
            index, 0, new QTableWidgetItem(QString::number(employee->getId())));
        employeeTable->setItem(index, 1,
                               new QTableWidgetItem(employee->getName()));
        employeeTable->setItem(index, 2,
                               new QTableWidgetItem(employee->getPosition()));
        employeeTable->setItem(index, 3,
                               new QTableWidgetItem(employee->getDepartment()));
        employeeTable->setItem(index, 4,
                               new QTableWidgetItem(QString::number(
                                   employee->getSalary(), 'f', 2)));
        employeeTable->setItem(
            index, 5, new QTableWidgetItem(employee->getEmployeeType()));
    }
}

void DisplayHelper::displayProjects(QTableWidget* projectTable,
                                    Company* currentCompany) {
    if (currentCompany == nullptr || projectTable == nullptr) return;

    auto projects = currentCompany->getAllProjects();
    projectTable->setRowCount(projects.size());

    for (size_t index = 0; index < projects.size(); ++index) {
        const auto& project = projects[index];
        projectTable->setItem(
            index, 0, new QTableWidgetItem(QString::number(project.getId())));
        projectTable->setItem(index, 1,
                              new QTableWidgetItem(project.getName()));
        projectTable->setItem(index, 2,
                              new QTableWidgetItem(project.getStatus()));
        projectTable->setItem(
            index, 3,
            new QTableWidgetItem(QString::number(project.getBudget(), 'f', 2)));
        projectTable->setItem(index, 4,
                              new QTableWidgetItem(project.getClientName()));
    }
}

void DisplayHelper::showCompanyInfo(QTextEdit* companyInfoText,
                                    Company* currentCompany) {
    if (currentCompany == nullptr || companyInfoText == nullptr) return;
    companyInfoText->setPlainText(currentCompany->getCompanyInfo());
}

void DisplayHelper::showStatistics(QTextEdit* statisticsText,
                                   Company* currentCompany) {
    if (currentCompany == nullptr || statisticsText == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    auto projects = currentCompany->getAllProjects();

    QString stats;
    stats.append("═══════════════════════════\n")
        .append("Company Statistics\n")
        .append("═══════════════════════════\n")
        .append(QString("\nTotal Employees: %1")
                    .arg(currentCompany->getEmployeeCount()))
        .append(QString("\nTotal Projects: %1")
                    .arg(currentCompany->getProjectCount()))
        .append(QString("\nTotal Salaries: $%1")
                    .arg(currentCompany->getTotalSalaries(), 0, 'f', 2))
        .append(QString("\nTotal Budget: $%1")
                    .arg(currentCompany->getTotalBudget(), 0, 'f', 2))
        .append("\n\n═══════════════════════════\n")
        .append("Employees by Type:\n")
        .append("═══════════════════════════\n");

    std::map<QString, int> employeeTypeCount;
    for (const auto& employee : employees) {
        employeeTypeCount[employee->getEmployeeType()]++;
    }

    for (const auto& [employeeType, count] : employeeTypeCount) {
        stats.append(QString("\n%1: %2")
                         .arg(employeeType, -18)
                         .arg(count));
    }

    statisticsText->setPlainText(stats);
}

