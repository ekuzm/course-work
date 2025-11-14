#include "helpers/employee_dialog_handler.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QtMath>

#include "entities/company.h"
#include "utils/consts.h"
#include "entities/derived_employees.h"
#include "helpers/employee_dialog_helper.h"
#include "helpers/employee_validator.h"
#include "helpers/id_helper.h"
#include "helpers/validation_helper.h"

bool EmployeeDialogHandler::validateEmployeeFields(QDialog* dialog,
                                                   const QLineEdit* nameEdit,
                                                   const QLineEdit* salaryEdit,
                                                   const QLineEdit* deptEdit,
                                                   QString& name,
                                                   double& salary,
                                                   QString& department) {
    if (!dialog || !nameEdit || !salaryEdit || !deptEdit)
        return false;

    name = nameEdit->text().trimmed();
    auto salaryText = salaryEdit->text().trimmed();
    department = deptEdit->text().trimmed();

    if (!ValidationHelper::validateNonEmpty(name, "Employee name", dialog))
        return false;

    if (!ValidationHelper::validateDouble(salaryText, salary, kMinSalary,
                                          kMaxSalary, "Salary", dialog))
        return false;

    if (!ValidationHelper::validateNonEmpty(department, "Department", dialog))
        return false;

    return true;
}

bool EmployeeDialogHandler::processAddEmployee(
    QDialog* dialog, Company* company, int& nextEmployeeId, QLineEdit* nameEdit,
    QLineEdit* salaryEdit, QLineEdit* deptEdit, QComboBox* typeCombo,
    QComboBox* employmentRateCombo, QComboBox* managerProject,
    QLineEdit* devLanguage, QLineEdit* devExperience, QLineEdit* designerTool,
    QLineEdit* designerProjects, QLineEdit* qaTestType, QLineEdit* qaBugs) {
    if (!dialog || !company || !nameEdit || !salaryEdit || !deptEdit ||
        !typeCombo || !employmentRateCombo)
        return false;

    QString name;
    double salary = 0.0;
    QString department;
    
    if (!validateEmployeeFields(dialog, nameEdit, salaryEdit, deptEdit,
                               name, salary, department))
        return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployee(name, company)) {
        QMessageBox::warning(dialog, "Duplicate Error",
                             "Employee with this name already exists!\n\n"
                             "Employee name: \"" +
                                 name +
                                 "\"\n"
                                 "Please choose a different name or edit the "
                                 "existing employee.");
        return false;
    }

    auto employeeType = typeCombo->currentText();
    if (!EmployeeValidator::validateEmployeeTypeFields(
            employeeType, dialog, devLanguage, devExperience, designerTool,
            designerProjects, qaTestType, qaBugs)) {
        return false;
    }

    int employeeId = nextEmployeeId;
    nextEmployeeId++;

    auto employee = EmployeeDialogHelper::createEmployeeFromType(
        employeeType, employeeId, name, salary, department, employmentRateCombo,
        managerProject, devLanguage, devExperience, designerTool,
        designerProjects, qaTestType, qaBugs);
    if (employee == nullptr) {
        QMessageBox::warning(
            dialog, "Validation Error",
            "Failed to create employee!\n\n"
            "Please check that all required fields are filled correctly.");
        return false;
    }

    company->addEmployee(employee);
    auto employees = company->getAllEmployees();
    nextEmployeeId =
        IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));
    return true;
}

bool EmployeeDialogHandler::processEditEmployee(
    QDialog* dialog, Company* company, int employeeId, int& nextEmployeeId,
    QLineEdit* nameEdit, QLineEdit* salaryEdit, QLineEdit* deptEdit,
    QComboBox* employmentRateCombo, QComboBox* managerProject,
    QLineEdit* devLanguage, QLineEdit* devExperience, QLineEdit* designerTool,
    QLineEdit* designerProjects, QLineEdit* qaTestType, QLineEdit* qaBugs,
    const QString& currentType) {
    if (!dialog || !company || !nameEdit || !salaryEdit || !deptEdit ||
        !employmentRateCombo)
        return false;

    QString name;
    double salary = 0.0;
    QString department;
    
    if (!validateEmployeeFields(dialog, nameEdit, salaryEdit, deptEdit,
                               name, salary, department))
        return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(name, employeeId,
                                                            company)) {
        QMessageBox::warning(dialog, "Duplicate Error",
                             "Employee with this name already exists!\n\n"
                             "Employee name: \"" +
                                 name +
                                 "\"\n"
                                 "Employee ID: " +
                                 QString::number(employeeId) +
                                 "\n"
                                 "Please choose a different name or edit the "
                                 "existing employee.");
        return false;
    }

    if (!EmployeeValidator::validateEmployeeTypeFields(
            currentType, dialog, devLanguage, devExperience, designerTool,
            designerProjects, qaTestType, qaBugs)) {
        return false;
    }

    auto oldEmployee = company->getEmployee(employeeId);
    if (!oldEmployee) {
        QMessageBox::warning(dialog, "Error", "Employee not found!");
        return false;
    }

    bool hasChanges = false;

    if (oldEmployee->getName() != name || oldEmployee->getSalary() != salary ||
        oldEmployee->getDepartment() != department ||
        oldEmployee->getEmploymentRate() !=
            employmentRateCombo->currentData().toDouble()) {
        hasChanges = true;
    }

    if (currentType == "Manager") {
        if (const auto* manager =
                dynamic_cast<const Manager*>(oldEmployee.get())) {
            int newProjectId = managerProject->currentData().toInt();
            if (manager->getManagedProjectId() != newProjectId) {
                hasChanges = true;
            }
        }
    } else if (currentType == "Developer") {
        if (const auto* developer =
                dynamic_cast<const Developer*>(oldEmployee.get())) {
            auto newLanguage = devLanguage->text().trimmed();
            double newExperience = devExperience->text().toDouble();
            if (developer->getProgrammingLanguage() != newLanguage ||
                qAbs(developer->getYearsOfExperience() - newExperience) >
                    0.01) {
                hasChanges = true;
            }
        }
    } else if (currentType == "Designer") {
        if (const auto* designer =
                dynamic_cast<const Designer*>(oldEmployee.get())) {
            auto newTool = designerTool->text().trimmed();
            int newProjects = designerProjects->text().toInt();
            if (designer->getDesignTool() != newTool ||
                designer->getNumberOfProjects() != newProjects) {
                hasChanges = true;
            }
        }
    } else if (currentType == "QA") {
        if (const auto* qa = dynamic_cast<const QA*>(oldEmployee.get())) {
            auto newTestType = qaTestType->text().trimmed();
            int newBugs = qaBugs->text().toInt();
            if (qa->getTestingType() != newTestType ||
                qa->getBugsFound() != newBugs) {
                hasChanges = true;
            }
        }
    }

    if (!hasChanges) {
        QMessageBox::information(
            dialog, "No Changes",
            "No changes were made to the employee.\n\n"
            "Please modify at least one field before saving.");
        return false;
    }

    std::vector<int> savedAssignedProjects = oldEmployee->getAssignedProjects();
    std::vector<int> savedProjectHistory = oldEmployee->getProjectHistory();
    int savedCurrentWeeklyHours = oldEmployee->getCurrentWeeklyHours();
    bool savedIsActive = oldEmployee->getIsActive();
    double oldEmploymentRate = oldEmployee->getEmploymentRate();
    double newEmploymentRate = employmentRateCombo->currentData().toDouble();

    
    double scaleFactor = 1.0;
    bool employmentRateChanged = (qAbs(oldEmploymentRate - newEmploymentRate) > 0.001);
    if (employmentRateChanged && oldEmploymentRate > 0) {
        scaleFactor = newEmploymentRate / oldEmploymentRate;
    }

    std::vector<std::tuple<int, int, int, int>> savedTaskAssignments;
    for (int projectId : savedAssignedProjects) {
        auto tasks = company->getProjectTasks(projectId);
        for (const auto& task : tasks) {
            int hours = company->getEmployeeTaskHours(employeeId, projectId,
                                                      task.getId());
            if (hours > 0) {
                savedTaskAssignments.push_back(std::make_tuple(
                    employeeId, projectId, task.getId(), hours));
            }
        }
    }

    auto updatedEmployee = EmployeeDialogHelper::createEmployeeFromType(
        currentType, employeeId, name, salary, department, employmentRateCombo,
        managerProject, devLanguage, devExperience, designerTool,
        designerProjects, qaTestType, qaBugs);
    if (updatedEmployee == nullptr) {
        QMessageBox::warning(
            dialog, "Validation Error",
            "Failed to update employee!\n\n"
            "Please check that all required fields are filled correctly.");
        return false;
    }

    company->removeEmployee(employeeId);

    company->addEmployee(updatedEmployee);

    for (int projectId : savedProjectHistory) {
        updatedEmployee->addToProjectHistory(projectId);
    }

    for (int projectId : savedAssignedProjects) {
        updatedEmployee->addAssignedProject(projectId);
    }

    
    if (employmentRateChanged && scaleFactor > 0) {
        company->scaleEmployeeTaskAssignments(employeeId, scaleFactor);
    } else {
        
        for (const auto& assignment : savedTaskAssignments) {
            const auto [empId, projId, taskId, hours] = assignment;
            try {
                company->restoreTaskAssignment(employeeId, projId, taskId, hours);
            } catch (const std::exception& e) {
                (void)e;
            }
        }
    }

    try {
        updatedEmployee->setIsActive(savedIsActive);
    } catch (const EmployeeException& e) {
        (void)e;
        if (!savedIsActive && updatedEmployee->getCurrentWeeklyHours() > 0) {
            try {
                int currentHours = updatedEmployee->getCurrentWeeklyHours();
                updatedEmployee->removeWeeklyHours(currentHours);
                updatedEmployee->setIsActive(false);
            } catch (const EmployeeException&) {
            }
        }
    }

    auto employees = company->getAllEmployees();
    nextEmployeeId =
        IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));
    return true;
}
