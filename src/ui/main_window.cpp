#include "ui/main_window.h"

#include <QAbstractItemView>
#include <QBrush>
#include <QCloseEvent>
#include <QColor>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QHeaderView>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QSizePolicy>
#include <QStringList>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <fstream>
#include <map>
#include <ranges>
#include <set>

#include "entities/derived_employees.h"
#include "exceptions/exception_handler.h"
#include "helpers/action_button_helper.h"
#include "helpers/dialog_helper.h"
#include "helpers/display_helper.h"
#include "helpers/employee_dialog_handler.h"
#include "helpers/file_helper.h"
#include "helpers/html_generator.h"
#include "helpers/id_helper.h"
#include "helpers/project_dialog_helper.h"
#include "helpers/project_helper.h"
#include "helpers/task_assignment_helper.h"
#include "helpers/task_dialog_helper.h"
#include "helpers/validation_helper.h"
#include "managers/auto_save_loader.h"
#include "ui/main_window_helpers.h"
#include "ui/main_window_operations.h"
#include "ui/main_window_ui_builder.h"
#include "ui/statistics_chart_widget.h"
#include "utils/app_styles.h"
#include "utils/consts.h"

struct AddEmployeeButtonParams {
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

void handleAddEmployeeButtonClick(const AddEmployeeButtonParams& params) {
    try {
        if (EmployeeDialogHandler::AddEmployeeParams handlerParams{
                &params.dialog, params.window->currentCompany,
                params.window->nextEmployeeId, params.nameEdit,
                params.salaryEdit, params.deptEdit, params.typeCombo,
                params.employmentRateCombo, params.managerProject,
                params.devLanguage, params.devExperience, params.designerTool,
                params.designerProjects, params.qaTestType, params.qaBugs};
            !EmployeeDialogHandler::processAddEmployee(handlerParams)) {
            return;
        }

        MainWindowDataOperations::refreshAllData(params.window);
        MainWindowDataOperations::autoSave(params.window);
        QMessageBox::information(
            &params.dialog, "Success",
                                 "Employee added successfully!\n\n"
                                 "Name: " +
                                     params.nameEdit->text().trimmed() +
                "\nType: " + params.typeCombo->currentText() + "\nSalary: $" +
                                     params.salaryEdit->text().trimmed());
        params.dialog.accept();
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &params.dialog,
                                                 "add employee");
    } catch (const FileManagerException& e) {
        ExceptionHandler::handleFileManagerException(e, &params.dialog,
                                                     "employee");
    } catch (const EmployeeException& e) {
        ExceptionHandler::handleGenericException(e, &params.dialog);
    }
}

struct EditEmployeeButtonParams {
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

void handleEditEmployeeButtonClick(const EditEmployeeButtonParams& params) {
    try {
        if (EmployeeDialogHandler::EditEmployeeParams handlerParams{
                &params.dialog, params.window->currentCompany,
                params.employeeId, params.window->nextEmployeeId,
                params.nameEdit, params.salaryEdit, params.deptEdit,
                params.employmentRateCombo, params.managerProject,
                params.devLanguage, params.devExperience, params.designerTool,
                params.designerProjects, params.qaTestType, params.qaBugs,
                params.currentType};
            !EmployeeDialogHandler::processEditEmployee(handlerParams)) {
            return;
        }

        MainWindowDataOperations::refreshAllData(params.window);
        MainWindowDataOperations::autoSave(params.window);
        params.dialog.hide();
        QMessageBox::information(params.dialog.parentWidget(), "Success",
            "Employee updated successfully!\n\n"
            "Name: " +
                                     params.nameEdit->text().trimmed() +
                                     "\nType: " + params.currentType +
                                     "\nSalary: $" +
                                     params.salaryEdit->text().trimmed());
        params.dialog.accept();
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &params.dialog,
                                                 "edit employee");
    } catch (const FileManagerException& e) {
        ExceptionHandler::handleFileManagerException(e, &params.dialog,
                                                     "employee update");
    } catch (const EmployeeException& e) {
        ExceptionHandler::handleGenericException(e, &params.dialog);
    }
}

void handleAddProjectButtonClick(
    MainWindow* window, QDialog& dialog,
    const ProjectDialogHelper::ProjectDialogFields& fields) {
    try {
        auto projectName = fields.nameEdit->text().trimmed();
        if (!ValidationHelper::validateNonEmpty(projectName, "Project name",
                                                &dialog))
            return;

        double projectBudget = 0.0;
        if (!ValidationHelper::validateDouble(
                fields.budgetEdit->text().trimmed(), projectBudget, 0.0,
                kMaxBudget, "Budget", &dialog))
            return;

        auto selectedPhase = fields.phaseCombo->currentText();
        auto clientName = fields.clientNameEdit->text().trimmed();

        if (!ValidationHelper::validateNonEmpty(clientName, "Client name",
                                                &dialog))
            return;

        if (!ValidationHelper::validateDateRange(
                fields.startDate->date(), fields.endDate->date(), &dialog))
            return;

        int estimatedHours = 0;
        if (!ValidationHelper::validateInt(
                fields.estimatedHoursEdit->text().trimmed(), estimatedHours, 0,
                kMaxEstimatedHours, "Estimated hours", &dialog))
            return;

        auto existingProjects = window->currentCompany->getAllProjects();
        for (const auto& project : existingProjects) {
            if (project.getName().toLower() == projectName.toLower()) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "A project with this name already exists!\n\n"
                    "Project name: \"" +
                        projectName +
                        "\"\n"
                        "Please choose a different name.");
                return;
            }
        }

        int projectId = window->nextProjectId;
        window->nextProjectId = window->nextProjectId + 1;
        ProjectParams projectParams{projectId,
                                    projectName,
            fields.descEdit->toPlainText().trimmed(),
                                    selectedPhase,
                                    fields.startDate->date(),
                                    fields.endDate->date(),
                                    projectBudget,
                                    clientName,
            estimatedHours};
        Project project(projectParams);

        window->currentCompany->addProject(project);
        auto projects = window->currentCompany->getAllProjects();
        window->nextProjectId =
            IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
        MainWindowDataOperations::refreshAllData(window);
        MainWindowDataOperations::autoSave(window);
        dialog.hide();
        QMessageBox::information(window, "Success",
            "Project added successfully!\n\n"
            "Name: " +
                projectName +
                "\n"
                "Phase: " +
                selectedPhase +
                "\n"
                "Budget: $" +
                QString::number(projectBudget, 'f', 2));
        dialog.accept();
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &dialog, "add project");
    } catch (const FileManagerException& e) {
        ExceptionHandler::handleFileManagerException(e, &dialog, "project");
    } catch (const ProjectException& e) {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    MainWindowUIHelper::setupUI(this);
    MainWindowDataOperations::autoLoad(this);
    CompanyOperations::initializeCompanySetup(this);
}

MainWindow::~MainWindow() {
    autoSave();
    for (auto* company : companies) {
        delete company;
    }
}

QString MainWindow::getDataDirectory() {
    return AutoSaveLoader::getDataDirectory();
}

void MainWindow::autoSave() { MainWindowDataOperations::autoSave(this); }

void MainWindow::autoLoad() { MainWindowDataOperations::autoLoad(this); }

void MainWindow::closeEvent(QCloseEvent* event) {
    MainWindowDataOperations::autoSave(this);
    event->accept();
}

void MainWindow::refreshAllData() {
    MainWindowDataOperations::refreshAllData(this);
            }

static void filterTasksByPendingId(int pendingTaskSelectionId,
                                   std::vector<Task>& tasks) {
    if (pendingTaskSelectionId <= 0) return;
    
    std::vector<Task> filtered;
    for (const auto& task : tasks) {
        if (task.getId() == pendingTaskSelectionId) {
            filtered.push_back(task);
            break;
        }
    }
    if (!filtered.empty()) {
        tasks = filtered;
    }
}

static QString formatExpectedRoleText(const QString& projectPhase) {
    QString expectedRoleText =
        TaskAssignmentHelper::getExpectedRoleForProjectPhase(projectPhase);
    if (expectedRoleText.isEmpty()) {
        expectedRoleText = "Unknown phase";
    } else if (expectedRoleText == "any role") {
        expectedRoleText = "Any role allowed";
    } else {
        expectedRoleText += " role required";
    }
    return expectedRoleText;
}

static void setupTaskComboForPendingSelection(QComboBox* taskCombo,
                                              int pendingTaskSelectionId) {
    if (pendingTaskSelectionId > 0 && taskCombo->count() == 1) {
        taskCombo->setEnabled(false);
        taskCombo->setStyleSheet(
            taskCombo->styleSheet() +
            " QComboBox { color: #3c3c3c; font-weight: 400; }"
            " QComboBox::drop-down { width: 0px; }");
    } else if (pendingTaskSelectionId > 0) {
        for (int index = 0; index < taskCombo->count(); ++index) {
            if (taskCombo->itemData(index).toInt() == pendingTaskSelectionId) {
                taskCombo->setCurrentIndex(index);
                break;
            }
        }
    }
}

static void showAutoAssignSuccessMessage(MainWindow* mainWindow,
                                         const Company* company, int projectId,
                                         int allocatedBefore) {
    MainWindowDataOperations::refreshAllData(mainWindow);
    MainWindowDataOperations::selectProjectRowById(mainWindow, projectId);
    
    const auto* projectAfter = company->getProject(projectId);
    int allocatedAfter =
        projectAfter ? projectAfter->getAllocatedHours() : allocatedBefore;
    int hoursAssigned = allocatedAfter - allocatedBefore;

    MainWindowDataOperations::autoSave(mainWindow);
    QMessageBox::information(
        mainWindow, "Success",
        QString("Employees auto-assigned successfully!\n\n"
                "Hours assigned: %1h\n"
                "Total allocated: %2h / %3h estimated")
            .arg(hoursAssigned)
            .arg(allocatedAfter)
            .arg(projectAfter ? projectAfter->getEstimatedHours() : 0));
}

static void handleAutoAssignException(MainWindow* mainWindow, const Company* company,
                                      int projectId,
                                      const CompanyException& e) {
    QString errorMsg = e.what();
    if (errorMsg.contains("ASSIGNED_HOURS:")) {
        QStringList parts = errorMsg.split(":");
        int trackedHours = parts.size() > 1 ? parts[1].toInt() : 0;
        int actualHours = parts.size() > 2 ? parts[2].toInt() : 0;

        MainWindowDataOperations::refreshAllData(mainWindow);
        const auto* projectAfter = company->getProject(projectId);
        int allocatedAfter =
            projectAfter ? projectAfter->getAllocatedHours() : 0;

        QMessageBox::information(
            mainWindow, "Success",
            QString("Employees auto-assigned successfully!\n\n"
                    "Hours assigned (tracked): %1h\n"
                    "Hours assigned (actual): %2h\n"
                    "Total allocated: %3h / %4h estimated")
                .arg(trackedHours)
                .arg(actualHours)
                .arg(allocatedAfter)
                .arg(projectAfter ? projectAfter->getEstimatedHours() : 0));
    } else {
        QMessageBox::warning(mainWindow, "Error",
            QString("Failed to auto-assign: ") + e.what());
    }
}

void MainWindow::selectProjectRowById(int projectId) {
    MainWindowDataOperations::selectProjectRowById(this, projectId);
}

void MainWindow::validateAndFixProjectAssignments(Company* company) {
    MainWindowValidationHelper::validateAndFixProjectAssignments(this, company);
}
