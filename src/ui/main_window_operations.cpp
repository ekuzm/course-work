#include "ui/main_window_operations.h"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextEdit>
#include <QVBoxLayout>
#include <span>
#include <set>
#include <vector>

#include "entities/company.h"
#include "entities/derived_employees.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"
#include "exceptions/exception_handler.h"
#include "exceptions/exceptions.h"
#include "helpers/dialog_helper.h"
#include "helpers/display_helper.h"
#include "helpers/employee_dialog_helper.h"
#include "helpers/employee_dialog_handler.h"
#include "helpers/file_helper.h"
#include "helpers/html_generator.h"
#include "helpers/id_helper.h"
#include "helpers/project_dialog_helper.h"
#include "helpers/project_helper.h"
#include "helpers/task_assignment_helper.h"
#include "helpers/task_dialog_helper.h"
#include "helpers/validation_helper.h"
#include "managers/company_manager.h"
#include "services/cost_calculation_service.h"
#include "services/task_assignment_service.h"
#include "ui/main_window.h"
#include "ui/main_window_helpers.h"
#include "utils/consts.h"

void handleOperationExceptions(QWidget* parent, const std::exception& e,
                               const QString& operation) {
    QMessageBox::critical(
        parent, "Error",
        QString("Failed to %1: %2").arg(operation).arg(e.what()));
}

static void handleAddEmployeeSuccess(MainWindow* window, QDialog& dialog,
                                     const QLineEdit* nameEdit, const QComboBox* typeCombo,
                                     const QLineEdit* salaryEdit) {
    MainWindowDataOperations::refreshAllData(window);
    MainWindowDataOperations::autoSave(window);
    QMessageBox::information(&dialog, "Success",
                             "Employee added successfully!\n\n"
                             "Name: " +
                                 nameEdit->text().trimmed() +
                                 "\nType: " + typeCombo->currentText() +
                                 "\nSalary: $" +
                                 salaryEdit->text().trimmed());
    dialog.accept();
}

static void handleAddEmployeeError(MainWindow* [[maybe_unused]] window, QDialog& dialog,
                                    const std::exception& e) {
    if (const auto* companyEx = dynamic_cast<const CompanyException*>(&e)) {
        QMessageBox::warning(&dialog, "Error",
                             QString("Failed to add employee!\n\nError details: %1\n\nPlease "
                                     "check the input data and try again.")
                                 .arg(companyEx->what()));
    } else if (const auto* fileEx = dynamic_cast<const FileManagerException*>(&e)) {
        QMessageBox::warning(
            &dialog, "Auto-save Error",
            QString("Failed to auto-save changes!\n\nError details: %1\n\n"
                    "The employee was completed but the data could not be saved "
                    "automatically.\n"
                    "Please check file permissions and try again.")
                .arg(fileEx->what()));
    } else {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

static void handleEditEmployeeSuccess(QDialog& dialog, const QLineEdit* nameEdit,
                                      const QString& currentType,
                                      const QLineEdit* salaryEdit) {
    MainWindowDataOperations::refreshAllData(
        qobject_cast<MainWindow*>(dialog.parent()));
    MainWindowDataOperations::autoSave(
        qobject_cast<MainWindow*>(dialog.parent()));
    QMessageBox::information(
        &dialog, "Success",
        "Employee updated successfully!\n\n"
        "Name: " +
            nameEdit->text().trimmed() + "\nType: " + currentType +
            "\nSalary: $" + salaryEdit->text().trimmed());
    dialog.accept();
}

struct HandleAddEmployeeButtonClickParams {
    MainWindow* window;
    QDialog& dialog;
    QLineEdit* nameEdit;
    QLineEdit* salaryEdit;
    QLineEdit* deptEdit;
    QComboBox* typeCombo;
    QComboBox* employmentRateCombo;
    QComboBox* managerProject;
    QLineEdit* devLanguage;
    QLineEdit* devExperience;
    QLineEdit* designerTool;
    QLineEdit* designerProjects;
    QLineEdit* qaTestType;
    QLineEdit* qaBugs;
};

static void handleAddEmployeeButtonClick(const HandleAddEmployeeButtonClickParams& params) {
    MainWindow* window = params.window;
    QDialog& dialog = params.dialog;
    const QLineEdit* nameEdit = params.nameEdit;
    const QLineEdit* salaryEdit = params.salaryEdit;
    const QLineEdit* deptEdit = params.deptEdit;
    QComboBox* typeCombo = params.typeCombo;
    QComboBox* employmentRateCombo = params.employmentRateCombo;
    QComboBox* managerProject = params.managerProject;
    QLineEdit* devLanguage = params.devLanguage;
    QLineEdit* devExperience = params.devExperience;
    QLineEdit* designerTool = params.designerTool;
    QLineEdit* designerProjects = params.designerProjects;
    QLineEdit* qaTestType = params.qaTestType;
    QLineEdit* qaBugs = params.qaBugs;
    try {
        if (EmployeeDialogHandler::AddEmployeeParams addParams{
                &dialog, window->currentCompany, window->nextEmployeeId,
                nameEdit, salaryEdit, deptEdit, typeCombo, employmentRateCombo,
                managerProject, devLanguage, devExperience, designerTool,
                designerProjects, qaTestType, qaBugs};
            !EmployeeDialogHandler::processAddEmployee(addParams)) {
            return;
        }
        handleAddEmployeeSuccess(window, dialog, nameEdit, typeCombo, salaryEdit);
    } catch (const CompanyException& e) {
        handleAddEmployeeError(window, dialog, e);
    } catch (const FileManagerException& e) {
        handleAddEmployeeError(window, dialog, e);
    } catch (const EmployeeException& e) {
        handleAddEmployeeError(window, dialog, e);
    } catch (const ProjectException& e) {
        handleAddEmployeeError(window, dialog, e);
    } catch (const TaskException& e) {
        handleAddEmployeeError(window, dialog, e);
    }
}

static void handleEditEmployeeError(QDialog& dialog, const std::exception& e);

struct HandleEditEmployeeButtonClickParams {
    MainWindow* window;
    QDialog& dialog;
    int employeeId;
    QLineEdit* nameEdit;
    QLineEdit* salaryEdit;
    QLineEdit* deptEdit;
    QComboBox* employmentRateCombo;
    QComboBox* managerProject;
    QLineEdit* devLanguage;
    QLineEdit* devExperience;
    QLineEdit* designerTool;
    QLineEdit* designerProjects;
    QLineEdit* qaTestType;
    QLineEdit* qaBugs;
    const QString& currentType;
};

static void handleEditEmployeeButtonClick(const HandleEditEmployeeButtonClickParams& params) {
    MainWindow* window = params.window;
    QDialog& dialog = params.dialog;
    int employeeId = params.employeeId;
    const QLineEdit* nameEdit = params.nameEdit;
    const QLineEdit* salaryEdit = params.salaryEdit;
    const QLineEdit* deptEdit = params.deptEdit;
    QComboBox* employmentRateCombo = params.employmentRateCombo;
    QComboBox* managerProject = params.managerProject;
    QLineEdit* devLanguage = params.devLanguage;
    QLineEdit* devExperience = params.devExperience;
    QLineEdit* designerTool = params.designerTool;
    QLineEdit* designerProjects = params.designerProjects;
    QLineEdit* qaTestType = params.qaTestType;
    QLineEdit* qaBugs = params.qaBugs;
    const QString& currentType = params.currentType;
    try {
        if (EmployeeDialogHandler::EditEmployeeParams editParams{
                &dialog, window->currentCompany, employeeId, window->nextEmployeeId,
                nameEdit, salaryEdit, deptEdit, employmentRateCombo,
                managerProject, devLanguage, devExperience, designerTool,
                designerProjects, qaTestType, qaBugs, currentType};
            !EmployeeDialogHandler::processEditEmployee(editParams)) {
            return;
        }
        if (auto* mainWindow = qobject_cast<MainWindow*>(window)) {
            MainWindowDataOperations::refreshAllData(mainWindow);
            MainWindowDataOperations::autoSave(mainWindow);
        }
        QMessageBox::information(
            &dialog, "Success",
            "Employee updated successfully!\n\n"
            "Name: " +
                nameEdit->text().trimmed() + "\nType: " + currentType +
                "\nSalary: $" + salaryEdit->text().trimmed());
        dialog.accept();
    } catch (const CompanyException& e) {
        handleEditEmployeeError(dialog, e);
    } catch (const FileManagerException& e) {
        handleEditEmployeeError(dialog, e);
    } catch (const EmployeeException& e) {
        handleEditEmployeeError(dialog, e);
    } catch (const ProjectException& e) {
        handleEditEmployeeError(dialog, e);
    } catch (const TaskException& e) {
        handleEditEmployeeError(dialog, e);
    }
}

static void handleEditEmployeeError(QDialog& dialog, const std::exception& e) {
    if (const auto* companyEx = dynamic_cast<const CompanyException*>(&e)) {
        ExceptionHandler::handleCompanyException(*companyEx, &dialog,
                                                 "edit employee");
    } else if (const auto* fileEx = dynamic_cast<const FileManagerException*>(&e)) {
        ExceptionHandler::handleFileManagerException(*fileEx, &dialog,
                                                     "employee update");
    } else if (const auto* empEx = dynamic_cast<const EmployeeException*>(&e)) {
        ExceptionHandler::handleGenericException(*empEx, &dialog);
    } else if (const auto* projEx = dynamic_cast<const ProjectException*>(&e)) {
        ExceptionHandler::handleGenericException(*projEx, &dialog);
    } else if (const auto* taskEx = dynamic_cast<const TaskException*>(&e)) {
        ExceptionHandler::handleGenericException(*taskEx, &dialog);
    } else {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

static void handleAutoAssignCompanyException(MainWindow* window, int projectId,
                                             const CompanyException& e) {
    QString errorMsg = e.what();
    if (errorMsg.contains("ASSIGNED_HOURS:")) {
        QStringList parts = errorMsg.split(":");
        int trackedHours = parts.size() > 1 ? parts[1].toInt() : 0;
        int actualHours = parts.size() > 2 ? parts[2].toInt() : 0;

        MainWindowDataOperations::refreshAllData(window);
        const auto* projectAfter = window->currentCompany->getProject(projectId);
        int allocatedAfter = projectAfter ? projectAfter->getAllocatedHours() : 0;
        
        QString message = QString("Employees auto-assigned successfully!\n\n"
                                  "Hours assigned (tracked): %1h\n"
                                  "Hours assigned (actual): %2h\n"
                                  "Total allocated: %3h / %4h estimated")
                              .arg(trackedHours)
                              .arg(actualHours)
                              .arg(allocatedAfter)
                              .arg(projectAfter ? projectAfter->getEstimatedHours() : 0);
        
        QMessageBox::information(window, "Success", message);
    } else {
        QString detailedMessage = errorMsg;
        if (!errorMsg.contains("Error details:")) {
            detailedMessage = QString("Failed to assign employee to task!\n\n"
                                      "Error details:\n%1\n\n"
                                      "Please check the input data and try again.")
                                  .arg(errorMsg);
        }
        QMessageBox::warning(window, "Failed to assign employee to task!",
                             detailedMessage);
    }
}

template <typename Func>
void executeWithExceptionHandling(MainWindow* window, Func&& operation,
                                  const QString& operationName) {
    try {
        operation();
    } catch (const CompanyException& e) {
        handleOperationExceptions(window, e, operationName);
    } catch (const FileManagerException& e) {
        handleOperationExceptions(window, e, operationName);
    } catch (const EmployeeException& e) {
        handleOperationExceptions(window, e, operationName);
    } catch (const ProjectException& e) {
        handleOperationExceptions(window, e, operationName);
    }
}

struct EmployeeDialogFieldPointers {
    QLineEdit* nameEdit = nullptr;
    QLineEdit* salaryEdit = nullptr;
    QLineEdit* deptEdit = nullptr;
    QComboBox* employmentRateCombo = nullptr;
    QComboBox* managerProject = nullptr;
    QLineEdit* devLanguage = nullptr;
    QLineEdit* devExperience = nullptr;
    QLineEdit* designerTool = nullptr;
    QLineEdit* designerProjects = nullptr;
    QLineEdit* qaTestType = nullptr;
    QLineEdit* qaBugs = nullptr;
    QLabel* managerProjectLabel = nullptr;
    QLabel* devLanguageLabel = nullptr;
    QLabel* devExperienceLabel = nullptr;
    QLabel* designerToolLabel = nullptr;
    QLabel* designerProjectsLabel = nullptr;
    QLabel* qaTestTypeLabel = nullptr;
    QLabel* qaBugsLabel = nullptr;
};

struct SetupEmployeeDialogCommonFieldsParams {
    QDialog& dialog;
    QVBoxLayout* mainLayout;
    QLabel* managerProjectLabel;
    QComboBox* managerProject;
    QLabel* devLanguageLabel;
    QLineEdit* devLanguage;
    QLabel* devExperienceLabel;
    QLineEdit* devExperience;
    QLabel* designerToolLabel;
    QLineEdit* designerTool;
    QLabel* designerProjectsLabel;
    QLineEdit* designerProjects;
    QLabel* qaTestTypeLabel;
    QLineEdit* qaTestType;
    QLabel* qaBugsLabel;
    QLineEdit* qaBugs;
};

static QPushButton* setupEmployeeDialogCommonFields(const SetupEmployeeDialogCommonFieldsParams& params) {
    params.mainLayout->addStretch();
    auto* okButton = new QPushButton("OK");
    params.mainLayout->addWidget(okButton);
    
    if (params.managerProjectLabel) params.managerProjectLabel->setVisible(true);
    if (params.managerProject) params.managerProject->setVisible(true);
    if (params.devLanguageLabel) params.devLanguageLabel->setVisible(true);
    if (params.devLanguage) params.devLanguage->setVisible(true);
    if (params.devExperienceLabel) params.devExperienceLabel->setVisible(true);
    if (params.devExperience) params.devExperience->setVisible(true);
    if (params.designerToolLabel) params.designerToolLabel->setVisible(true);
    if (params.designerTool) params.designerTool->setVisible(true);
    if (params.designerProjectsLabel) params.designerProjectsLabel->setVisible(true);
    if (params.designerProjects) params.designerProjects->setVisible(true);
    if (params.qaTestTypeLabel) params.qaTestTypeLabel->setVisible(true);
    if (params.qaTestType) params.qaTestType->setVisible(true);
    if (params.qaBugsLabel) params.qaBugsLabel->setVisible(true);
    if (params.qaBugs) params.qaBugs->setVisible(true);
    
    params.dialog.adjustSize();
    QSize maxSize = params.dialog.size();
    maxSize.setHeight(maxSize.height() - kEmployeeDialogHeightOffset);
    params.dialog.setFixedSize(maxSize);
    
    return okButton;
}

static QPushButton* setupAddEmployeeDialogUI(QDialog& dialog, QVBoxLayout* mainLayout,
                                     EmployeeDialogHelper::CreateEmployeeDialogFields& fields,
                                     QComboBox*& typeCombo, const MainWindow* window) {
    auto* form = new QFormLayout();
    mainLayout->addLayout(form);
    
    EmployeeDialogHelper::createEmployeeDialog(dialog, form, fields);
    
    auto projects = window->currentCompany->getAllProjects();
    fields.managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        fields.managerProject->addItem(proj.getName(), proj.getId());
    }
    
    SetupEmployeeDialogCommonFieldsParams commonParams{
        dialog, mainLayout,
        fields.managerProjectLabel, fields.managerProject,
        fields.devLanguageLabel, fields.devLanguage,
        fields.devExperienceLabel, fields.devExperience,
        fields.designerToolLabel, fields.designerTool,
        fields.designerProjectsLabel, fields.designerProjects,
        fields.qaTestTypeLabel, fields.qaTestType,
        fields.qaBugsLabel, fields.qaBugs
    };
    auto* okButton = setupEmployeeDialogCommonFields(commonParams);
    
    if (typeCombo) {
        int index = typeCombo->currentIndex();
        EmployeeDialogHelper::showDeveloperFields(fields.devLanguageLabel, fields.devLanguage,
                                                  fields.devExperienceLabel,
                                                  fields.devExperience, index == 1);
        EmployeeDialogHelper::showDesignerFields(
            fields.designerToolLabel, fields.designerTool, fields.designerProjectsLabel,
            fields.designerProjects, index == 2);
        EmployeeDialogHelper::showQaFields(fields.qaTestTypeLabel, fields.qaTestType,
                                           fields.qaBugsLabel, fields.qaBugs, index == 3);
        EmployeeDialogHelper::showManagerFields(fields.managerProjectLabel,
                                                fields.managerProject, index == 0);
    }
    
    return okButton;
}

static QPushButton* setupEditEmployeeDialogUI(QDialog& dialog, QVBoxLayout* mainLayout,
                                              EmployeeDialogHelper::CreateEditEmployeeDialogFields& fields,
                                              const std::shared_ptr<Employee>& employee,
                                              const QString& currentType, const MainWindow* window) {
    auto* form = new QFormLayout();
    mainLayout->addLayout(form);
    
    EmployeeDialogHelper::createEditEmployeeDialog(dialog, form, employee, fields);
    
    auto projects = window->currentCompany->getAllProjects();
    fields.managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        fields.managerProject->addItem(proj.getName(), proj.getId());
    }
    
    if (const auto* manager = dynamic_cast<const Manager*>(employee.get()); manager) {
        int currentProjectId = manager->getManagedProjectId();
        if (currentProjectId > 0) {
            auto index = fields.managerProject->findData(currentProjectId);
            if (index >= 0) {
                fields.managerProject->setCurrentIndex(index);
            }
        }
    }
    
    SetupEmployeeDialogCommonFieldsParams commonParams{
        dialog, mainLayout,
        fields.managerProjectLabel, fields.managerProject,
        fields.devLanguageLabel, fields.devLanguage,
        fields.devExperienceLabel, fields.devExperience,
        fields.designerToolLabel, fields.designerTool,
        fields.designerProjectsLabel, fields.designerProjects,
        fields.qaTestTypeLabel, fields.qaTestType,
        fields.qaBugsLabel, fields.qaBugs
    };
    auto* okButton = setupEmployeeDialogCommonFields(commonParams);
    
    EmployeeDialogHelper::showManagerFields(fields.managerProjectLabel, fields.managerProject,
                                            currentType == "Manager");
    EmployeeDialogHelper::showDeveloperFields(fields.devLanguageLabel, fields.devLanguage,
                                              fields.devExperienceLabel, fields.devExperience,
                                              currentType == "Developer");
    EmployeeDialogHelper::showDesignerFields(
        fields.designerToolLabel, fields.designerTool, fields.designerProjectsLabel,
        fields.designerProjects, currentType == "Designer");
    EmployeeDialogHelper::showQaFields(fields.qaTestTypeLabel, fields.qaTestType, fields.qaBugsLabel,
                                       fields.qaBugs, currentType == "QA");
    
    return okButton;
}

void EmployeeOperations::addEmployee(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "adding employees")) return;

    QDialog dialog(window);
    dialog.setWindowTitle("Add Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* mainLayout = new QVBoxLayout(&dialog);
    QComboBox* typeCombo = nullptr;
    EmployeeDialogFieldPointers fieldPointers;

    EmployeeDialogHelper::CreateEmployeeDialogFields fields{
        typeCombo, fieldPointers.nameEdit, fieldPointers.salaryEdit, fieldPointers.deptEdit, fieldPointers.employmentRateCombo,
        fieldPointers.managerProject, fieldPointers.devLanguage, fieldPointers.devExperience, fieldPointers.designerTool,
        fieldPointers.designerProjects, fieldPointers.qaTestType, fieldPointers.qaBugs, fieldPointers.managerProjectLabel,
        fieldPointers.devLanguageLabel, fieldPointers.devExperienceLabel, fieldPointers.designerToolLabel,
        fieldPointers.designerProjectsLabel, fieldPointers.qaTestTypeLabel, fieldPointers.qaBugsLabel};

    const auto* okButton = setupAddEmployeeDialogUI(dialog, mainLayout, fields, typeCombo, window);
    if (!okButton) return;

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, &fieldPointers, typeCombo]() {
        handleAddEmployeeButtonClick({window, dialog, fieldPointers.nameEdit, fieldPointers.salaryEdit, fieldPointers.deptEdit,
                                     typeCombo, fieldPointers.employmentRateCombo, fieldPointers.managerProject,
                                     fieldPointers.devLanguage, fieldPointers.devExperience, fieldPointers.designerTool,
                                     fieldPointers.designerProjects, fieldPointers.qaTestType, fieldPointers.qaBugs});
    });

    dialog.exec();
}

void EmployeeOperations::editEmployee(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "editing employees")) return;
    
    auto employeeId = MainWindowSelectionHelper::getSelectedEmployeeId(window);
    if (employeeId < 0) {
        QMessageBox::warning(window, "Error",
                             "Please select an employee to edit.");
        return;
    }
    
    auto employee = window->currentCompany->getEmployee(employeeId);
    if (employee == nullptr) {
        QMessageBox::warning(window, "Error", "Employee not found!");
        return;
    }
    
    QDialog dialog(window);
    dialog.setWindowTitle("Edit Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* mainLayout = new QVBoxLayout(&dialog);
    auto currentType = employee->getEmployeeType();
    EmployeeDialogFieldPointers fieldPointers;

    EmployeeDialogHelper::CreateEditEmployeeDialogFields fields{
        fieldPointers.nameEdit, fieldPointers.salaryEdit, fieldPointers.deptEdit, fieldPointers.employmentRateCombo, fieldPointers.managerProject,
        fieldPointers.devLanguage, fieldPointers.devExperience, fieldPointers.designerTool, fieldPointers.designerProjects,
        fieldPointers.qaTestType, fieldPointers.qaBugs, fieldPointers.managerProjectLabel, fieldPointers.devLanguageLabel,
        fieldPointers.devExperienceLabel, fieldPointers.designerToolLabel, fieldPointers.designerProjectsLabel,
        fieldPointers.qaTestTypeLabel, fieldPointers.qaBugsLabel};

    const auto* okButton = setupEditEmployeeDialogUI(dialog, mainLayout, fields, employee, currentType, window);
    if (!okButton) return;

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, employeeId, &fieldPointers, currentType]() {
        handleEditEmployeeButtonClick({window, dialog, employeeId, fieldPointers.nameEdit, fieldPointers.salaryEdit,
                                      fieldPointers.deptEdit, fieldPointers.employmentRateCombo, fieldPointers.managerProject,
                                      fieldPointers.devLanguage, fieldPointers.devExperience, fieldPointers.designerTool,
                                      fieldPointers.designerProjects, fieldPointers.qaTestType, fieldPointers.qaBugs, currentType});
    });

    dialog.exec();
}

void EmployeeOperations::deleteEmployee(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "deleting employees")) return;

    auto employeeId = MainWindowSelectionHelper::getSelectedEmployeeId(window);
    if (employeeId < 0) {
        QMessageBox::warning(window, "Error",
                             "Please select an employee to delete.");
        return;
    }
    
    int userChoice =
        QMessageBox::question(window, "Confirm Delete",
                              "Are you sure you want to delete this employee?",
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            window->currentCompany->removeEmployee(employeeId);
            auto employees = window->currentCompany->getAllEmployees();
            window->nextEmployeeId = IdHelper::calculateNextId(
                IdHelper::findMaxEmployeeId(employees));
            MainWindowDataOperations::refreshAllData(window);
            MainWindowDataOperations::autoSave(window);
            QMessageBox::information(window, "Success",
                                     "Employee deleted successfully!");

        } catch (const CompanyException& e) {
            QMessageBox::warning(
                window, "Error",
                QString("Failed to delete employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                window, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

void EmployeeOperations::fireEmployee(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "firing employees")) return;
    
    auto employeeId = MainWindowSelectionHelper::getSelectedEmployeeId(window);
    if (employeeId < 0) {
        QMessageBox::warning(window, "Error",
                             "Please select an employee to fire.");
        return;
    }
    
    auto employee = window->currentCompany->getEmployee(employeeId);
    if (!employee) {
        QMessageBox::warning(window, "Error", "Employee not found.");
        return;
    }
    
    if (!employee->getIsActive()) {
        QMessageBox::information(window, "Information",
                                 "This employee is already fired.");
        return;
    }
    
    if (employee->getCurrentWeeklyHours() > 0) {
        if (int userChoice = QMessageBox::question(
        window, "Confirm Fire",
            QString("This employee has active assignments (%1 hours/week).\n\n"
                    "Are you sure you want to fire this employee?\n\n"
                    "This will remove all active assignments.")
                .arg(employee->getCurrentWeeklyHours()),
        QMessageBox::Yes | QMessageBox::No);
            userChoice != QMessageBox::Yes) {
            return;
        }

        MainWindowTaskAssignmentHelper::handleEmployeeActiveAssignments(window, employeeId, employee);
    }

    int userChoice = QMessageBox::question(
        window, "Confirm Fire",
        QString("Are you sure you want to fire %1?").arg(employee->getName()),
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            employee->setIsActive(false);
            MainWindowDataOperations::refreshAllData(window);
            MainWindowDataOperations::autoSave(window);
            QMessageBox::information(
                window, "Success",
                QString("Employee %1 has been fired successfully!")
                    .arg(employee->getName()));

        } catch (const EmployeeException& e) {
            QMessageBox::warning(
                window, "Error", QString("Failed to fire employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                window, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

void EmployeeOperations::searchEmployee(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "searching employees")) return;
    
    auto searchTerm = window->employeeUI.searchEdit->text().trimmed().toLower();

    if (searchTerm.isEmpty()) {
        DisplayHelper::displayEmployees(window->employeeUI.table, window->currentCompany, window);
        return;
    }
    
    auto employees = window->currentCompany->getAllEmployees();
    window->employeeUI.table->setRowCount(0);
    int rowIndex = 0;

    for (const auto& employee : employees) {
        if (!employee) continue;
        
        auto name = employee->getName().toLower();
        auto department = employee->getDepartment().toLower();
        auto position = employee->getPosition().toLower();

        if (name.contains(searchTerm) || department.contains(searchTerm) ||
            position.contains(searchTerm)) {
        window->employeeUI.table->insertRow(rowIndex);
        window->employeeUI.table->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(employee->getId())));
        window->employeeUI.table->setItem(rowIndex, 1,
                                         new QTableWidgetItem(employee->getName()));
        window->employeeUI.table->setItem(
            rowIndex, 2, new QTableWidgetItem(employee->getDepartment()));
        window->employeeUI.table->setItem(rowIndex, 3,
                                   new QTableWidgetItem(QString::number(
                                       employee->getSalary(), 'f', 2)));
        window->employeeUI.table->setItem(
            rowIndex, 4, new QTableWidgetItem(employee->getEmployeeType()));

            QString projectInfo =
                DisplayHelper::formatProjectInfo(employee, window->currentCompany);
        window->employeeUI.table->setItem(rowIndex, 5,
                                   new QTableWidgetItem(projectInfo));

        window->employeeUI.table->setCellWidget(rowIndex, 6,
                                               MainWindowUIHelper::createEmployeeActionButtons(window, rowIndex));
        rowIndex++;
        }
    }
}

void EmployeeOperations::refreshEmployeeTable(MainWindow* window) {
    if (!window) return;
    DisplayHelper::displayEmployees(window->employeeUI.table,
                                    window->currentCompany, window);
}

void ProjectOperations::addProject(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "adding projects")) return;

    QDialog dialog(window);
    dialog.setWindowTitle("Add Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);
    ProjectDialogHelper::ProjectDialogFields fields;
    ProjectDialogHelper::createProjectDialogFields(dialog, form, fields);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, &fields]() {
        handleAddProjectButtonClick(window, dialog, fields);
    });

    dialog.exec();
}

void ProjectOperations::editProject(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "editing projects")) return;
    
    auto projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    if (projectId < 0) {
        QMessageBox::warning(window, "Error", "Please select a project to edit.");
        return;
    }
    
    const auto* project = window->currentCompany->getProject(projectId);
    if (project == nullptr) {
        QMessageBox::warning(window, "Error", "Project not found!");
        return;
    }
    
    QDialog dialog(window);
    dialog.setWindowTitle("Edit Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);

    auto* form = new QFormLayout(&dialog);
    ProjectDialogHelper::ProjectDialogFields fields;
    ProjectDialogHelper::createProjectDialogFields(dialog, form, fields);
    ProjectDialogHelper::populateProjectDialogFields(project, fields);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, projectId, &fields]() {
        MainWindowProjectDialogHandler::handleEditProjectDialog(window, projectId, dialog, fields);
    });

    dialog.exec();
}

void ProjectOperations::deleteProject(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "deleting projects")) return;
    
    auto projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    if (projectId < 0) {
        QMessageBox::warning(window, "Error",
                             "Please select a project to delete.");
        return;
    }
    
    int userChoice = QMessageBox::question(
        window, "Confirm Delete", "Are you sure you want to delete this project?",
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            window->currentCompany->removeProject(projectId);
            auto projects = window->currentCompany->getAllProjects();
            window->nextProjectId =
                IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
            MainWindowDataOperations::refreshAllData(window);
            MainWindowDataOperations::autoSave(window);
            if (window->detailedProjectId == projectId) {
                ProjectDetailOperations::hideProjectDetails(window);
            }
            QMessageBox::information(window, "Success",
                                     "Project deleted successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                window, "Error",
                QString("Failed to delete project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                window, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

void ProjectOperations::refreshProjectTable(MainWindow* window) {
    if (!window) return;
    DisplayHelper::displayProjects(window->projectUI.table,
                                   window->currentCompany, window);
}

void ProjectOperations::openProjectDetails(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "viewing project details")) return;
    auto projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    if (projectId < 0) {
        QMessageBox::information(window, "Project Details",
                                 "Please select a project to view details.");
        return;
    }
    ProjectDetailOperations::showProjectDetails(window, projectId);
}

void ProjectOperations::closeProjectDetails(MainWindow* window) {
    if (!window) return;
    ProjectDetailOperations::hideProjectDetails(window);
    ProjectOperations::refreshProjectTable(window);
}

void ProjectOperations::autoAssignDetailedProject(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "auto-assigning employees")) return;
    if (window->detailedProjectId < 0) {
        QMessageBox::information(window, "Auto Assign",
                                 "Open a project to manage its tasks first.");
        return;
    }
    
    ProjectOperations::autoAssignToProject(window, window->detailedProjectId);

    if (window->projectUI.detailContainer != nullptr &&
        window->projectUI.detailContainer->isVisible()) {
        ProjectDetailOperations::refreshProjectDetailView(window);
    }
}

void ProjectOperations::assignTaskFromDetails(MainWindow* window, int projectId,
                                              int taskId) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "assigning tasks")) return;

    if (projectId <= 0 || taskId <= 0) {
        QObject* senderObject = window->sender();
        const auto* assignButton = qobject_cast<QPushButton*>(senderObject);
        if (assignButton != nullptr) {
            projectId = assignButton->property("projectId").toInt();
            taskId = assignButton->property("taskId").toInt();
        }
    }

    if (projectId <= 0 || taskId <= 0) return;

    window->detailedProjectId = projectId;
    MainWindowDataOperations::selectProjectRowById(window, projectId);
    window->pendingTaskSelectionId = taskId;
    ProjectOperations::assignEmployeeToTask(window);
    window->pendingTaskSelectionId = -1;
}

void ProjectOperations::addProjectTask(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "adding tasks")) return;

    auto projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    if (projectId < 0) {
        QMessageBox::warning(window, "Error", "Please select a project first.");
        return;
    }
    
    QDialog dialog(window);
    auto* form = new QFormLayout(&dialog);

    QLineEdit* taskNameEdit = nullptr;
    QComboBox* taskTypeCombo = nullptr;
    QLineEdit* taskEstHoursEdit = nullptr;
    QLineEdit* priorityEdit = nullptr;

    TaskDialogHelper::createAddTaskDialog(&dialog, form, taskNameEdit,
                                          taskTypeCombo, taskEstHoursEdit,
                                          priorityEdit);

    auto* okButton = new QPushButton("Add Task");
    form->addRow(okButton);

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, projectId, taskNameEdit, taskTypeCombo, taskEstHoursEdit, priorityEdit]() {
        MainWindowProjectDialogHandler::handleAddTaskDialog(window, projectId, dialog, taskNameEdit, taskTypeCombo, taskEstHoursEdit, priorityEdit);
    });

    dialog.exec();
}

static bool validateAssignEmployeeToTask(MainWindow* window, int& projectId,
                                         std::vector<Task>& tasks, QString& projectPhase) {
    if (projectId < 0) {
        projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    }
    if (projectId < 0) {
        QMessageBox::warning(window, "Error", "Please select a project first.");
        return false;
    }
    
    tasks = window->currentCompany->getProjectTasks(projectId);
    if (window->pendingTaskSelectionId > 0) {
        std::vector<Task> filtered;
        for (const auto& task : tasks) {
            if (task.getId() == window->pendingTaskSelectionId) {
                filtered.push_back(task);
                break;
            }
        }
        if (!filtered.empty()) {
            tasks = filtered;
        }
    }
    if (tasks.empty()) {
        QMessageBox::warning(
            window, "Error", "No tasks in this project. Please add tasks first.");
        return false;
    }

    const Project* project = window->currentCompany->getProject(projectId);
    projectPhase = (project != nullptr) ? project->getPhase() : "";

    if (projectPhase == "Completed") {
        QMessageBox::warning(window, "Error",
                             "Cannot assign employees to completed project.");
        return false;
    }
    
    QComboBox tempEmployeeCombo;
    int matchingCount = 0;
    TaskAssignmentHelper::populateEmployeeCombo(
        &tempEmployeeCombo, window->currentCompany, projectId, projectPhase, matchingCount);
    
    if (tempEmployeeCombo.count() == 0) {
        QMessageBox::warning(window, "Error", "No available employees found!");
        return false;
    }
    return true;
}

struct SetupAssignTaskDialogUIParams {
    QFormLayout* form;
    QComboBox*& taskCombo;
    QComboBox*& employeeCombo;
    QLineEdit*& hoursEdit;
    QLabel*& taskInfoLabel;
    MainWindow* window;
    int projectId;
    const QString& projectPhase;
    const std::vector<Task>& tasks;
    int matchingCount;
};

static void setupAssignTaskDialogUI(const SetupAssignTaskDialogUIParams& params) {
    QFormLayout* form = params.form;
    QComboBox*& taskCombo = params.taskCombo;
    QComboBox*& employeeCombo = params.employeeCombo;
    QLineEdit*& hoursEdit = params.hoursEdit;
    QLabel*& taskInfoLabel = params.taskInfoLabel;
    MainWindow* window = params.window;
    int projectId = params.projectId;
    const QString& projectPhase = params.projectPhase;
    const std::vector<Task>& tasks = params.tasks;
    int matchingCount = params.matchingCount;
    taskCombo = new QComboBox();
    taskCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    TaskAssignmentHelper::setupTaskCombo(taskCombo, tasks,
                                         window->pendingTaskSelectionId);

    if (window->pendingTaskSelectionId > 0 && taskCombo->count() == 1) {
        taskCombo->setEnabled(false);
        taskCombo->setStyleSheet(
            taskCombo->styleSheet() +
            " QComboBox { color: #3c3c3c; font-weight: 400; }"
            " QComboBox::drop-down { width: 0px; }");
    } else if (window->pendingTaskSelectionId > 0) {
        for (int index = 0; index < taskCombo->count(); ++index) {
            if (taskCombo->itemData(index).toInt() == window->pendingTaskSelectionId) {
                taskCombo->setCurrentIndex(index);
                break;
            }
        }
    }
    form->addRow("Task:", taskCombo);

    employeeCombo = new QComboBox();
    employeeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");

    TaskAssignmentHelper::populateEmployeeCombo(
        employeeCombo, window->currentCompany, projectId, projectPhase, matchingCount);

    if (matchingCount == 0 && !projectPhase.isEmpty()) {
        QString expectedRole =
            TaskAssignmentHelper::getExpectedRoleForProjectPhase(projectPhase);
        QMessageBox::information(
            window, "Note",
            QString("No %1 employees available for %2 phase.")
                .arg(expectedRole, projectPhase));
    }

    form->addRow("Employee:", employeeCombo);

    QString expectedRoleText =
        TaskAssignmentHelper::getExpectedRoleForProjectPhase(projectPhase);
    if (expectedRoleText.isEmpty()) {
        expectedRoleText = "Unknown phase";
    } else if (expectedRoleText == "any role") {
        expectedRoleText = "Any role allowed";
    } else {
        expectedRoleText += " role required";
    }

    taskInfoLabel =
        new QLabel(QString("Project Phase: %1 | %2")
                       .arg(projectPhase.isEmpty() ? "Unknown" : projectPhase)
                       .arg(expectedRoleText));
    form->addRow(taskInfoLabel);

    TaskAssignmentHelper::setupEmployeeComboUpdate(
        employeeCombo, taskCombo, taskInfoLabel, window->currentCompany, projectId,
        projectPhase);

    hoursEdit = new QLineEdit();
    hoursEdit->setPlaceholderText(QString("e.g., %1 (hours per week)").arg(kExampleHoursPerWeek));
    TaskAssignmentHelper::setupHoursEdit(hoursEdit, taskCombo, employeeCombo,
                                         tasks, window->currentCompany);
    form->addRow("Hours per week:", hoursEdit);
}

void ProjectOperations::assignEmployeeToTask(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "assigning employees to tasks")) return;

    int projectId = -1;
    std::vector<Task> tasks;
    QString projectPhase;
    if (!validateAssignEmployeeToTask(window, projectId, tasks, projectPhase)) {
        return;
    }
    
    int matchingCount = 0;
    QComboBox tempEmployeeCombo;
    TaskAssignmentHelper::populateEmployeeCombo(
        &tempEmployeeCombo, window->currentCompany, projectId, projectPhase, matchingCount);
    
    QDialog dialog(window);
    dialog.setWindowTitle("Assign Employee to Task");
    dialog.setMinimumWidth(kAssignTaskDialogMinWidth);
    dialog.setStyleSheet(
        "QDialog { background-color: white; } "
        "QComboBox { background-color: white; color: black; } "
        "QLineEdit { background-color: white; color: black; } "
        "QLabel { color: black; }");

    auto* form = new QFormLayout(&dialog);
    QComboBox* taskCombo = nullptr;
    QComboBox* employeeCombo = nullptr;
    QLineEdit* hoursEdit = nullptr;
    QLabel* taskInfoLabel = nullptr;
    
    setupAssignTaskDialogUI({form, taskCombo, employeeCombo, hoursEdit,
                            taskInfoLabel, window, projectId, projectPhase, tasks, matchingCount});

    auto* okButton = new QPushButton("Assign");
    form->addRow(okButton);

    QObject::connect(okButton, &QPushButton::clicked, [window, &dialog, projectId, taskCombo, employeeCombo, hoursEdit, tasks]() {
        MainWindowProjectDialogHandler::handleAssignEmployeeToTaskDialog(window, projectId, dialog, taskCombo, employeeCombo, hoursEdit, tasks);
    });

    dialog.exec();
    window->pendingTaskSelectionId = -1;
    ProjectDetailOperations::refreshProjectDetailView(window);
}

void ProjectOperations::autoAssignToProject(MainWindow* window, int projectId) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "auto-assigning employees")) return;
    
    if (projectId < 0) {
        projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    }
    if (projectId < 0) {
        QMessageBox::warning(window, "Error", "Please select a project first.");
        return;
    }
    
    const auto* project = window->currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(window, "Error", "Project not found!");
        return;
    }

    if (auto tasks = window->currentCompany->getProjectTasks(projectId); tasks.empty()) {
        QMessageBox::warning(window, "Error",
                             "Project has no tasks. Please add tasks first "
                             "before using auto-assign!");
        return;
    }
    
    int response = QMessageBox::question(
        window, "Auto Assign",
        QString("Automatically assign available employees to project "
                "'%1'?\n\nEstimated: %2 hours\nAllocated: %3 hours")
            .arg(project->getName())
            .arg(project->getEstimatedHours())
            .arg(project->getAllocatedHours()),
        QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes) {
        try {
            auto allocatedBefore = project->getAllocatedHours();

        window->currentCompany->autoAssignEmployeesToProject(projectId);
            MainWindowDataOperations::refreshAllData(window);

            MainWindowDataOperations::selectProjectRowById(window, projectId);

            const auto* projectAfter = window->currentCompany->getProject(projectId);
            int allocatedAfter = projectAfter
                                     ? projectAfter->getAllocatedHours()
                                     : allocatedBefore;
            int hoursAssigned = allocatedAfter - allocatedBefore;

        MainWindowDataOperations::autoSave(window);
        
        QString message = QString("Employees auto-assigned successfully!\n\n"
                                  "Hours assigned: %1h\n"
                                  "Total allocated: %2h / %3h estimated")
                              .arg(hoursAssigned)
                              .arg(allocatedAfter)
                              .arg(projectAfter ? projectAfter->getEstimatedHours() : 0);
        
        QStringList warnings;
        if (projectAfter) {
            int estimated = projectAfter->getEstimatedHours();
            if (allocatedAfter < estimated && hoursAssigned == 0) {
                QString projectPhase = projectAfter->getPhase();
                QString expectedRole = TaskAssignmentHelper::getExpectedRoleForProjectPhase(projectPhase);
                warnings.append(
                    QString("No employees were assigned.\n"
                            "Possible reasons:\n"
                            "- No employees match project phase: %1\n"
                            "- Expected role: %2\n"
                            "- No employees have available hours\n"
                            "- Employee salaries exceed project budget")
                        .arg(projectPhase)
                        .arg(expectedRole));
            } else if (allocatedAfter < estimated) {
                int remaining = estimated - allocatedAfter;
                warnings.append(
                    QString("Not all hours were assigned.\n"
                            "Remaining: %1h / %2h estimated\n"
                            "Possible reasons:\n"
                            "- Not enough employees match project phase\n"
                            "- Employees have insufficient available hours\n"
                            "- Project budget constraints")
                        .arg(remaining)
                        .arg(estimated));
            }
        }
        
        if (!warnings.isEmpty()) {
            message += "\n\n--- Warnings ---\n";
            for (const auto& warning : warnings) {
                message += warning + "\n";
            }
        }
        
        QMessageBox::information(window, "Success", message);
    } catch (const CompanyException& e) {
            handleAutoAssignCompanyException(window, projectId, e);
    } catch (const EmployeeException& e) {
            QString detailedMessage = QString("Failed to assign employee to task!\n\n"
                                              "Error details:\n%1\n\n"
                                              "Please check the input data and try again.")
                                          .arg(e.what());
            QMessageBox::warning(window, "Failed to assign employee to task!",
                                 detailedMessage);
    } catch (const ProjectException& e) {
            QString detailedMessage = QString("Failed to assign employee to task!\n\n"
                                              "Error details:\n%1\n\n"
                                              "Please check the input data and try again.")
                                          .arg(e.what());
            QMessageBox::warning(window, "Failed to assign employee to task!",
                                 detailedMessage);
        } catch (const TaskException& e) {
            QString detailedMessage = QString("Failed to assign employee to task!\n\n"
                                              "Error details:\n%1\n\n"
                                              "Please check the input data and try again.")
                                          .arg(e.what());
            QMessageBox::warning(window, "Failed to assign employee to task!",
                                 detailedMessage);
        } catch (const FileManagerException& e) {
            QString detailedMessage = QString("Failed to assign employee to task!\n\n"
                                              "Error details:\n%1\n\n"
                                              "Please check the input data and try again.")
                                          .arg(e.what());
            QMessageBox::warning(window, "Failed to assign employee to task!",
                                 detailedMessage);
        }
    }
}

void ProjectOperations::viewProjectAssignments(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "viewing project assignments")) return;
    
    auto projectId = MainWindowSelectionHelper::getSelectedProjectId(window);
    if (projectId < 0) {
        QMessageBox::warning(window, "Error", "Please select a project first.");
        return;
    }
    
    const auto* project = window->currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(window, "Error", "Project not found!");
        return;
    }

    auto allProjects = window->currentCompany->getAllProjects();

    auto* table = new QTableWidget();
    auto headers = QStringList{"ID", "Project Name", "Client", "Phase", 
                               "Budget ($)", "Estimated Hours", "Allocated Hours", 
                               "Employee Costs ($)", "Status"};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    
    size_t projectCount = allProjects.size();
    if (projectCount > static_cast<size_t>(kMaxProjects)) {
        QMessageBox::warning(window, "Error", 
                             "Too many projects to display. Data may be corrupted.");
        delete table;
        return;
    }
    table->setRowCount(static_cast<int>(projectCount));

    auto row = 0;
    for (const auto& proj : allProjects) {
        auto* idItem = new QTableWidgetItem(QString::number(proj.getId()));
        idItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 0, idItem);
        
        auto* nameItem = new QTableWidgetItem(proj.getName());
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setItem(row, 1, nameItem);
        
        auto* clientItem = new QTableWidgetItem(proj.getClientName());
        clientItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        table->setItem(row, 2, clientItem);
        
        auto* phaseItem = new QTableWidgetItem(proj.getPhase());
        phaseItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 3, phaseItem);
        
        auto* budgetItem = new QTableWidgetItem(QString::number(proj.getBudget(), 'f', 2));
        budgetItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 4, budgetItem);
        
        auto* estimatedItem = new QTableWidgetItem(QString::number(proj.getEstimatedHours()));
        estimatedItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 5, estimatedItem);
        
        auto* allocatedItem = new QTableWidgetItem(QString::number(proj.getAllocatedHours()));
        allocatedItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 6, allocatedItem);
        
        auto* costsItem = new QTableWidgetItem(QString::number(proj.getEmployeeCosts(), 'f', 2));
        costsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        table->setItem(row, 7, costsItem);
        
        auto status = proj.isActive() ? "Active" : "Inactive";
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 8, statusItem);
        row++;
    }

    QHeaderView* header = table->horizontalHeader();
    
    header->setSectionResizeMode(0, QHeaderView::Fixed);  
    header->setSectionResizeMode(2, QHeaderView::Fixed);  
    header->setSectionResizeMode(3, QHeaderView::Fixed);  
    header->setSectionResizeMode(4, QHeaderView::Fixed);  
    header->setSectionResizeMode(5, QHeaderView::Fixed);  
    header->setSectionResizeMode(6, QHeaderView::Fixed);  
    header->setSectionResizeMode(7, QHeaderView::Fixed);  
    header->setSectionResizeMode(8, QHeaderView::Fixed);  
    
    table->setColumnWidth(0, kTableColumnWidthId);   
    table->setColumnWidth(2, kTableColumnWidth130);  
    table->setColumnWidth(3, kTableColumnWidth110);  
    table->setColumnWidth(4, kTableColumnWidth130);  
    table->setColumnWidth(5, kTableColumnWidth140);  
    table->setColumnWidth(6, kTableColumnWidth135);  
    table->setColumnWidth(7, kTableColumnWidth160);  
    table->setColumnWidth(8, kTableColumnWidth90);   
    
    header->setSectionResizeMode(1, QHeaderView::Stretch);  

    QDialog dialog(window);
    DialogHelper::createTableDialog(
        &dialog, "Projects and Clients: " + project->getName(), table, kProjectsTableDialogWidth, kProjectsTableDialogHeight);
    dialog.exec();
}

void ProjectOperations::viewEmployeeHistory(MainWindow* window) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(window, "viewing employee history")) return;
    
    auto employeeId = MainWindowSelectionHelper::getSelectedEmployeeId(window);
    if (employeeId < 0) {
        QMessageBox::warning(window, "Error", "Please select an employee first.");
        return;
    }
    
    auto employee = window->currentCompany->getEmployee(employeeId);
    if (!employee) {
        QMessageBox::warning(window, "Error", "Employee not found!");
        return;
    }
    
    auto assignedProjectIds = employee->getAssignedProjects();
    auto historyProjectIds = employee->getProjectHistory();

    std::set<int> allProjectIds;
    for (int id : assignedProjectIds) {
        allProjectIds.insert(id);
    }
    for (int id : historyProjectIds) {
        allProjectIds.insert(id);
    }

    auto allProjects = window->currentCompany->getAllProjects();

    std::vector<const Project*> employeeProjects;
    for (const auto& proj : allProjects) {
        if (allProjectIds.contains(proj.getId())) {
            employeeProjects.push_back(&proj);
    }
}

    if (employeeProjects.empty()) {
        QMessageBox::information(window, "Employee History",
                                 "No project history available for this employee.");
        return;
    }
    
    auto* table = new QTableWidget();
    auto headers = QStringList{"ID", "Project Name", "Client", "Phase", 
                               "Budget ($)", "Estimated Hours", "Allocated Hours", 
                               "Employee Costs ($)", "Status"};
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    
    size_t employeeProjectCount = employeeProjects.size();
    if (employeeProjectCount > kMaxProjects) {
        QMessageBox::warning(window, "Error", 
                             "Too many projects to display. Data may be corrupted.");
        delete table;
        return;
}
    table->setRowCount(static_cast<int>(employeeProjectCount));

    auto row = 0;
    for (const auto* proj : employeeProjects) {
        bool isCurrentlyAssigned = employee->isAssignedToProject(proj->getId());
        auto status = isCurrentlyAssigned ? "Active" : "Fired";

        table->setItem(row, 0, new QTableWidgetItem(QString::number(proj->getId())));
        table->setItem(row, 1, new QTableWidgetItem(proj->getName()));
        table->setItem(row, 2, new QTableWidgetItem(proj->getClientName()));
        table->setItem(row, 3, new QTableWidgetItem(proj->getPhase()));
        table->setItem(row, 4, new QTableWidgetItem(QString::number(proj->getBudget(), 'f', 2)));
        table->setItem(row, 5, new QTableWidgetItem(QString::number(proj->getEstimatedHours())));
        table->setItem(row, 6, new QTableWidgetItem(QString::number(proj->getAllocatedHours())));
        table->setItem(row, 7, new QTableWidgetItem(QString::number(proj->getEmployeeCosts(), 'f', 2)));
        table->setItem(row, 8, new QTableWidgetItem(status));
        row++;
}

    table->resizeColumnsToContents();
    table->setColumnWidth(0, kTableColumnWidthId); 
    table->setColumnWidth(1, kTableColumnWidth200); 
    table->setColumnWidth(2, kTableColumnWidthName); 
    table->setColumnWidth(3, kTableColumnWidth140); 
    table->setColumnWidth(4, kTableColumnWidthName); 
    table->setColumnWidth(5, kTableColumnWidth170); 
    table->setColumnWidth(6, kTableColumnWidth180); 
    table->setColumnWidth(7, kTableColumnWidth180); 
    table->setColumnWidth(8, kTableColumnWidth110); 

    QDialog dialog(window);
    DialogHelper::createTableDialog(
        &dialog, "Employee History: " + employee->getName(), table, kEmployeeHistoryDialogWidth, kEmployeeHistoryDialogHeight);
    dialog.exec();
}

void ProjectOperations::showStatistics(MainWindow* window) {
    if (!window || !window->currentCompany) return;
    
    DisplayHelper::showStatistics(window->statisticsUI.text,
                                  window->currentCompany);
    if (window->statisticsUI.chartWidget != nullptr) {
        MainWindowUIHelper::drawStatisticsChart(
            window, window->statisticsUI.chartWidget);
        window->statisticsUI.chartWidget->update();
    }
}

void ProjectDetailOperations::showProjectDetails(MainWindow* window,
                                                 int projectId) {
    MainWindowProjectDetailHelper::showProjectDetails(window, projectId);
}

void ProjectDetailOperations::hideProjectDetails(MainWindow* window) {
    MainWindowProjectDetailHelper::hideProjectDetails(window);
}

void ProjectDetailOperations::refreshProjectDetailView(MainWindow* window) {
    MainWindowProjectDetailHelper::refreshProjectDetailView(window);
}

void ProjectDetailOperations::populateProjectTasksTable(
    MainWindow* window, const Project& project) {
    MainWindowProjectDetailHelper::populateProjectTasksTable(window, project);
}

void CompanyOperations::addCompany(MainWindow* window) {
    if (!window) return;
    CompanyManager::addCompany(window->companies, window->currentCompany,
                               window->currentCompanyIndex,
                               window->companyUI.selector, window);
    CompanyOperations::refreshCompanyList(window);
    EmployeeOperations::refreshEmployeeTable(window);
    ProjectDetailOperations::hideProjectDetails(window);
    ProjectOperations::refreshProjectTable(window);
    ProjectOperations::showStatistics(window);
    MainWindowDataOperations::autoSave(window);
}

void CompanyOperations::switchCompany(MainWindow* window) {
    if (!window) return;
    if (window->companyUI.selector != nullptr) {
    int newIndex = window->companyUI.selector->currentIndex();
        CompanyManager::switchCompany(window->companies, window->currentCompany,
                                      window->currentCompanyIndex,
                                      window->companyUI.selector, newIndex);
        EmployeeOperations::refreshEmployeeTable(window);
        ProjectDetailOperations::hideProjectDetails(window);
        ProjectOperations::refreshProjectTable(window);
        ProjectOperations::showStatistics(window);

        if (window->currentCompany != nullptr) {
            auto employees = window->currentCompany->getAllEmployees();
            window->nextEmployeeId =
                IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));

            auto projects = window->currentCompany->getAllProjects();
            window->nextProjectId =
                IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
        }
    }
}

void CompanyOperations::deleteCompany(MainWindow* window) {
    if (!window) return;
    CompanyManager::deleteCompany(window->companies, window->currentCompany,
                                  window->currentCompanyIndex,
                                  window->companyUI.selector, window);
    CompanyOperations::refreshCompanyList(window);
    EmployeeOperations::refreshEmployeeTable(window);
    ProjectDetailOperations::hideProjectDetails(window);
    ProjectOperations::refreshProjectTable(window);
    ProjectOperations::showStatistics(window);
    MainWindowDataOperations::autoSave(window);
}

void CompanyOperations::refreshCompanyList(MainWindow* window) {
    if (!window) return;
    CompanyManager::refreshCompanyList(
        std::span<Company* const>(window->companies),
        window->companyUI.selector);
}

void CompanyOperations::initializeCompanySetup(MainWindow* window) {
    if (!window) return;
    
    EmployeeOperations::refreshEmployeeTable(window);
    ProjectDetailOperations::hideProjectDetails(window);
    ProjectOperations::refreshProjectTable(window);
    ProjectOperations::showStatistics(window);

    if (window->currentCompany != nullptr) {
        auto employees = window->currentCompany->getAllEmployees();
        window->nextEmployeeId =
            IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));

        auto projects = window->currentCompany->getAllProjects();
        window->nextProjectId =
            IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
    }
}
