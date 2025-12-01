#include "helpers/employee_dialog_handler.h"

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QtMath>

#include "entities/company.h"
#include "entities/derived_employees.h"
#include "helpers/employee_dialog_helper.h"
#include "helpers/employee_validator.h"
#include "helpers/id_helper.h"
#include "helpers/validation_helper.h"
#include "utils/consts.h"

static bool validateEmployeeFields(QDialog* dialog, const QLineEdit* nameEdit,
                                   const QLineEdit* salaryEdit,
                                   const QLineEdit* deptEdit, QString& name,
                                   double& salary, QString& department);

static bool processAddEmployeeImpl(
    const EmployeeDialogHandler::AddEmployeeParams& params) {
    if (!params.dialog || !params.company || !params.nameEdit ||
        !params.salaryEdit || !params.deptEdit || !params.typeCombo ||
        !params.employmentRateCombo)
        return false;

    QString name;
    double salary = 0.0;
    QString department;

    if (!validateEmployeeFields(params.dialog, params.nameEdit,
                                params.salaryEdit, params.deptEdit, name,
                                salary, department))
        return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployee(name, params.company)) {
        QMessageBox::warning(params.dialog, "Duplicate Error",
                             "Employee with this name already exists!\n\n"
                             "Employee name: \"" +
                                 name +
                                 "\"\n"
                                 "Please choose a different name or edit the "
                                 "existing employee.");
        return false;
    }

    auto employeeType = params.typeCombo->currentText();
    if (EmployeeValidator::ValidateEmployeeTypeFieldsParams validateParams{
            employeeType, params.dialog, params.devLanguage,
            params.devExperience, params.designerTool, params.designerProjects,
            params.qaTestType, params.qaBugs};
        !EmployeeValidator::validateEmployeeTypeFields(validateParams)) {
        return false;
    }

    int employeeId = params.nextEmployeeId;
    params.nextEmployeeId++;

    EmployeeDialogHelper::CreateEmployeeFromTypeParams createParams{
        employeeType,
        employeeId,
        name,
        salary,
        department,
        params.employmentRateCombo,
        params.managerProject,
        params.devLanguage,
        params.devExperience,
        params.designerTool,
        params.designerProjects,
        params.qaTestType,
        params.qaBugs};
    auto employee = EmployeeDialogHelper::createEmployeeFromType(createParams);
    if (employee == nullptr) {
        QMessageBox::warning(
            params.dialog, "Validation Error",
            "Failed to create employee!\n\n"
            "Please check that all required fields are filled correctly.");
        return false;
    }

    params.company->addEmployee(employee);
    auto employees = params.company->getAllEmployees();
    params.nextEmployeeId =
        IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));
    return true;
}

bool EmployeeDialogHandler::processAddEmployee(
    const AddEmployeeParams& params) {
    return processAddEmployeeImpl(params);
}

static bool validateEmployeeFields(QDialog* dialog, const QLineEdit* nameEdit,
                                   const QLineEdit* salaryEdit,
                                   const QLineEdit* deptEdit, QString& name,
                                   double& salary, QString& department) {
    if (!dialog || !nameEdit || !salaryEdit || !deptEdit) return false;

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

static bool checkBasicFieldChanges(const std::shared_ptr<Employee>& oldEmployee,
                                   const QString& name, double salary,
                                   const QString& department,
                                   double newEmploymentRate) {
    return oldEmployee->getName() != name ||
           oldEmployee->getSalary() != salary ||
           oldEmployee->getDepartment() != department ||
           oldEmployee->getEmploymentRate() != newEmploymentRate;
}

static bool checkManagerChanges(const std::shared_ptr<Employee>& oldEmployee,
                                const QComboBox* managerProject) {
    const auto* manager = dynamic_cast<const Manager*>(oldEmployee.get());
    if (!manager) {
        return false;
    }
    int newProjectId = managerProject->currentData().toInt();
    return manager->getManagedProjectId() != newProjectId;
}

static bool checkDeveloperChanges(const std::shared_ptr<Employee>& oldEmployee,
                                  const QLineEdit* devLanguage,
                                  const QLineEdit* devExperience) {
    const auto* developer = dynamic_cast<const Developer*>(oldEmployee.get());
    if (!developer) {
        return false;
    }
    auto newLanguage = devLanguage->text().trimmed();
    double newExperience = devExperience->text().toDouble();
    return developer->getProgrammingLanguage() != newLanguage ||
           qAbs(developer->getYearsOfExperience() - newExperience) > 0.01;
}

static bool checkDesignerChanges(const std::shared_ptr<Employee>& oldEmployee,
                                 const QLineEdit* designerTool,
                                 const QLineEdit* designerProjects) {
    const auto* designer = dynamic_cast<const Designer*>(oldEmployee.get());
    if (!designer) {
        return false;
    }
    auto newTool = designerTool->text().trimmed();
    int newProjects = designerProjects->text().toInt();
    return designer->getDesignTool() != newTool ||
           designer->getNumberOfProjects() != newProjects;
}

static bool checkQAChanges(const std::shared_ptr<Employee>& oldEmployee,
                           const QLineEdit* qaTestType,
                           const QLineEdit* qaBugs) {
    const auto* qa = dynamic_cast<const QA*>(oldEmployee.get());
    if (!qa) {
        return false;
    }
    auto newTestType = qaTestType->text().trimmed();
    int newBugs = qaBugs->text().toInt();
    return qa->getTestingType() != newTestType || qa->getBugsFound() != newBugs;
}

struct CheckTypeSpecificChangesParams {
    const std::shared_ptr<Employee>& oldEmployee;
    const QString& currentType;
    QComboBox* managerProject;
    QLineEdit* devLanguage;
    QLineEdit* devExperience;
    QLineEdit* designerTool;
    QLineEdit* designerProjects;
    QLineEdit* qaTestType;
    QLineEdit* qaBugs;
};

static bool checkTypeSpecificChanges(
    const CheckTypeSpecificChangesParams& params) {
    if (params.currentType == "Manager") {
        return checkManagerChanges(params.oldEmployee, params.managerProject);
    } else if (params.currentType == "Developer") {
        return checkDeveloperChanges(params.oldEmployee, params.devLanguage,
                                     params.devExperience);
    } else if (params.currentType == "Designer") {
        return checkDesignerChanges(params.oldEmployee, params.designerTool,
                                    params.designerProjects);
    } else if (params.currentType == "QA") {
        return checkQAChanges(params.oldEmployee, params.qaTestType,
                              params.qaBugs);
    }
    return false;
}

static void collectTaskAssignmentsForEmployee(
    const Company* company, int employeeId,
    const std::vector<int>& assignedProjects,
    std::vector<std::tuple<int, int, int, int>>& savedTaskAssignments) {
    for (int projectId : assignedProjects) {
        auto tasks = company->getProjectTasks(projectId);
        for (const auto& task : tasks) {
            int hours =
                company->getEmployeeHours(employeeId, projectId, task.getId());
            if (hours > 0) {
                savedTaskAssignments.push_back(std::make_tuple(
                    employeeId, projectId, task.getId(), hours));
            }
        }
    }
}

static void restoreEmployeeState(
    const std::shared_ptr<Employee>& updatedEmployee,
    const std::vector<int>& savedProjectHistory,
    const std::vector<int>& savedAssignedProjects, bool savedIsActive) {
    for (int projectId : savedProjectHistory) {
        updatedEmployee->addToProjectHistory(projectId);
    }

    for (int projectId : savedAssignedProjects) {
        updatedEmployee->addAssignedProject(projectId);
    }

    try {
        updatedEmployee->setIsActive(savedIsActive);
    } catch (const EmployeeException&) {
        if (!savedIsActive && updatedEmployee->getCurrentWeeklyHours() > 0) {
            int currentHours = updatedEmployee->getCurrentWeeklyHours();
            updatedEmployee->removeWeeklyHours(currentHours);
            updatedEmployee->setIsActive(false);
        }
    }
}

static bool processEditEmployeeImpl(
    const EmployeeDialogHandler::EditEmployeeParams& params) {
    if (!params.dialog || !params.company || !params.nameEdit ||
        !params.salaryEdit || !params.deptEdit || !params.employmentRateCombo)
        return false;

    QString name;
    double salary = 0.0;
    QString department;

    if (!validateEmployeeFields(params.dialog, params.nameEdit,
                                params.salaryEdit, params.deptEdit, name,
                                salary, department))
        return false;

    if (!EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(
            name, params.employeeId, params.company)) {
        QMessageBox::warning(params.dialog, "Duplicate Error",
                             "Employee with this name already exists!\n\n"
                             "Employee name: \"" +
                                 name +
                                 "\"\n"
                                 "Employee ID: " +
                                 QString::number(params.employeeId) +
                                 "\n"
                                 "Please choose a different name or edit the "
                                 "existing employee.");
        return false;
    }

    if (EmployeeValidator::ValidateEmployeeTypeFieldsParams validateParams{
            params.currentType, params.dialog, params.devLanguage,
            params.devExperience, params.designerTool, params.designerProjects,
            params.qaTestType, params.qaBugs};
        !EmployeeValidator::validateEmployeeTypeFields(validateParams)) {
        return false;
    }

    auto oldEmployee = params.company->getEmployee(params.employeeId);
    if (!oldEmployee) {
        QMessageBox::warning(params.dialog, "Error", "Employee not found!");
        return false;
    }

    double newEmploymentRate =
        params.employmentRateCombo->currentData().toDouble();
    bool hasChanges = checkBasicFieldChanges(oldEmployee, name, salary,
                                             department, newEmploymentRate);

    if (!hasChanges) {
        CheckTypeSpecificChangesParams checkParams{oldEmployee,
                                                   params.currentType,
                                                   params.managerProject,
                                                   params.devLanguage,
                                                   params.devExperience,
                                                   params.designerTool,
                                                   params.designerProjects,
                                                   params.qaTestType,
                                                   params.qaBugs};
        hasChanges = checkTypeSpecificChanges(checkParams);
    }

    if (!hasChanges) {
        QWidget* parent = params.dialog->parentWidget();
        if (!parent) {
            parent = params.dialog;
        }
        QMessageBox::information(
            parent, "No Changes",
            "No changes were made to the employee.\n\n"
            "Please modify at least one field before saving.");
        params.dialog->raise();
        params.dialog->activateWindow();
        return false;
    }

    std::vector<int> savedAssignedProjects = oldEmployee->getAssignedProjects();
    std::vector<int> savedProjectHistory = oldEmployee->getProjectHistory();
    bool savedIsActive = oldEmployee->getIsActive();
    double oldEmploymentRate = oldEmployee->getEmploymentRate();

    double scaleFactor = 1.0;
    bool employmentRateChanged =
        (qAbs(oldEmploymentRate - newEmploymentRate) > 0.001);
    if (employmentRateChanged && oldEmploymentRate > 0) {
        scaleFactor = newEmploymentRate / oldEmploymentRate;
    }

    std::vector<std::tuple<int, int, int, int>> savedTaskAssignments;
    collectTaskAssignmentsForEmployee(params.company, params.employeeId,
                                      savedAssignedProjects,
                                      savedTaskAssignments);

    EmployeeDialogHelper::CreateEmployeeFromTypeParams createParams{
        params.currentType,
        params.employeeId,
        name,
        salary,
        department,
        params.employmentRateCombo,
        params.managerProject,
        params.devLanguage,
        params.devExperience,
        params.designerTool,
        params.designerProjects,
        params.qaTestType,
        params.qaBugs};
    auto updatedEmployee =
        EmployeeDialogHelper::createEmployeeFromType(createParams);
    if (updatedEmployee == nullptr) {
        QMessageBox::warning(
            params.dialog, "Validation Error",
            "Failed to update employee!\n\n"
            "Please check that all required fields are filled correctly.");
        return false;
    }

    params.company->removeEmployee(params.employeeId);
    params.company->addEmployee(updatedEmployee);
    restoreEmployeeState(updatedEmployee, savedProjectHistory,
                         savedAssignedProjects, savedIsActive);

    if (employmentRateChanged && scaleFactor > 0) {
        params.company->scaleEmployeeTaskAssignments(params.employeeId,
                                                     scaleFactor);
    } else {
        for (const auto& assignment : savedTaskAssignments) {
            const auto [empId, projId, taskId, hours] = assignment;
            params.company->restoreTaskAssignment(params.employeeId, projId,
                                                  taskId, hours);
        }
    }

    auto employees = params.company->getAllEmployees();
    params.nextEmployeeId =
        IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));
    return true;
}

bool EmployeeDialogHandler::processEditEmployee(
    const EditEmployeeParams& params) {
    return processEditEmployeeImpl(params);
}
