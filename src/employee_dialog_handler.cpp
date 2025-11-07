#include "employee_dialog_handler.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>

#include "consts.h"
#include "company.h"
#include "employee_dialog_helper.h"
#include "employee_validator.h"
#include "validation_helper.h"

bool EmployeeDialogHandler::processAddEmployee(QDialog* dialog, Company* company, int& nextEmployeeId,
                                               QLineEdit* nameEdit, QLineEdit* salaryEdit, QLineEdit* deptEdit,
                                               QComboBox* typeCombo, QComboBox* employmentRateCombo,
                                               QComboBox* managerProject, QLineEdit* devLanguage,
                                               QLineEdit* devExperience, QLineEdit* designerTool,
                                               QLineEdit* designerProjects, QLineEdit* qaTestType,
                                               QLineEdit* qaBugs) {
    if (!dialog || !company || !nameEdit || !salaryEdit || !deptEdit || !typeCombo || !employmentRateCombo) return false;

    QString name = nameEdit->text().trimmed();
    QString salaryText = salaryEdit->text().trimmed();
    QString department = deptEdit->text().trimmed();

    if (!ValidationHelper::validateNonEmpty(name, "Employee name", dialog)) return false;

    double salary = 0.0;
    if (!ValidationHelper::validateDouble(salaryText, salary, kMinSalary, kMaxSalary, "Salary", dialog)) return false;

    if (!ValidationHelper::validateNonEmpty(department, "Department", dialog)) return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployee(name, company)) {
        QMessageBox::warning(dialog, "Duplicate Error",
                            "Employee with this name already exists!\n\n"
                            "Employee name: \"" + name + "\"\n"
                            "Please choose a different name or edit the existing employee.");
        return false;
    }

    QString employeeType = typeCombo->currentText();
    if (!EmployeeValidator::validateEmployeeTypeFields(employeeType, dialog, devLanguage, devExperience,
                                                      designerTool, designerProjects, qaTestType, qaBugs)) {
        return false;
    }

    int employeeId = nextEmployeeId;
    nextEmployeeId++;

    auto employee = EmployeeDialogHelper::createEmployeeFromType(
        employeeType, employeeId, name, salary, department,
        employmentRateCombo, managerProject, devLanguage, devExperience,
        designerTool, designerProjects, qaTestType, qaBugs);
    if (employee == nullptr) {
        QMessageBox::warning(dialog, "Validation Error",
                            "Failed to create employee!\n\n"
                            "Please check that all required fields are filled correctly.");
        return false;
    }

    company->addEmployee(employee);
    if (auto employees = company->getAllEmployees(); !employees.empty()) {
        nextEmployeeId = employees.back()->getId() + 1;
    }
    return true;
}

bool EmployeeDialogHandler::processEditEmployee(QDialog* dialog, Company* company, int employeeId, int& nextEmployeeId,
                                                QLineEdit* nameEdit, QLineEdit* salaryEdit, QLineEdit* deptEdit,
                                                QComboBox* employmentRateCombo,
                                                QComboBox* managerProject, QLineEdit* devLanguage,
                                                QLineEdit* devExperience, QLineEdit* designerTool,
                                                QLineEdit* designerProjects, QLineEdit* qaTestType,
                                                QLineEdit* qaBugs, const QString& currentType) {
    if (!dialog || !company || !nameEdit || !salaryEdit || !deptEdit || !employmentRateCombo) return false;

    QString name = nameEdit->text().trimmed();
    QString salaryText = salaryEdit->text().trimmed();
    QString department = deptEdit->text().trimmed();

    if (!ValidationHelper::validateNonEmpty(name, "Employee name", dialog)) return false;

    double salary = 0.0;
    if (!ValidationHelper::validateDouble(salaryText, salary, kMinSalary, kMaxSalary, "Salary", dialog)) return false;

    if (!ValidationHelper::validateNonEmpty(department, "Department", dialog)) return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(name, employeeId, company)) {
        QMessageBox::warning(dialog, "Duplicate Error",
                            "Employee with this name already exists!\n\n"
                            "Employee name: \"" + name + "\"\n"
                            "Employee ID: " + QString::number(employeeId) + "\n"
                            "Please choose a different name or edit the existing employee.");
        return false;
    }

    if (!EmployeeValidator::validateEmployeeTypeFields(currentType, dialog, devLanguage, devExperience,
                                                      designerTool, designerProjects, qaTestType, qaBugs)) {
        return false;
    }

    auto updatedEmployee = EmployeeDialogHelper::createEmployeeFromType(
        currentType, employeeId, name, salary, department,
        employmentRateCombo, managerProject, devLanguage, devExperience,
        designerTool, designerProjects, qaTestType, qaBugs);
    if (updatedEmployee == nullptr) {
        QMessageBox::warning(dialog, "Validation Error",
                            "Failed to update employee!\n\n"
                            "Please check that all required fields are filled correctly.");
        return false;
    }

    company->removeEmployee(employeeId);
    company->addEmployee(updatedEmployee);

    if (auto employees = company->getAllEmployees(); !employees.empty()) {
        nextEmployeeId = employees.back()->getId() + 1;
    }
    return true;
}

