#include "ui/main_window_helpers.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <ranges>

#include "entities/company.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"
#include "exceptions/exception_handler.h"
#include "exceptions/exceptions.h"
#include "helpers/action_button_helper.h"
#include "helpers/file_helper.h"
#include "helpers/html_generator.h"
#include "helpers/id_helper.h"
#include "helpers/project_dialog_helper.h"
#include "helpers/project_helper.h"
#include "helpers/task_assignment_helper.h"
#include "helpers/task_dialog_helper.h"
#include "helpers/validation_helper.h"
#include "managers/auto_save_loader.h"
#include "managers/file_manager.h"
#include "ui/main_window.h"
#include "ui/main_window_operations.h"
#include "ui/main_window_ui_builder.h"
#include "ui/statistics_chart_widget.h"
#include "utils/consts.h"

int MainWindowSelectionHelper::getSelectedEmployeeId(const MainWindow* window) {
    if (!window || !window->employeeUI.table) return -1;

    if (int rowIndex = window->employeeUI.table->currentRow(); rowIndex >= 0) {
        if (const QTableWidgetItem* tableItem =
                window->employeeUI.table->item(rowIndex, 0);
            tableItem != nullptr) {
            bool conversionSuccess = false;
            int employeeId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? employeeId : -1;
        }
    }
    return -1;
}

int MainWindowSelectionHelper::getSelectedProjectId(const MainWindow* window) {
    if (!window || !window->projectUI.table) return -1;

    int rowIndex = window->projectUI.table->currentRow();
    if (rowIndex < 0) return -1;

    const QTableWidgetItem* tableItem =
        window->projectUI.table->item(rowIndex, 0);
    if (tableItem == nullptr) return -1;

    bool conversionSuccess = false;
    int projectId = tableItem->text().toInt(&conversionSuccess);
    return conversionSuccess ? projectId : -1;
}

void MainWindowProjectDetailHelper::showProjectDetails(MainWindow* window,
                                                       int projectId) {
    if (!MainWindowValidationHelper::checkCompanyAndHandleError(
            window, "viewing project details"))
        return;

    const Project* project = window->currentCompany->getProject(projectId);
    if (project == nullptr) {
        QMessageBox::warning(window, "Error", "Project not found!");
        return;
    }

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(
        window->currentCompany, projectId);
    project = window->currentCompany->getProject(projectId);

    window->detailedProjectId = projectId;
    window->pendingTaskSelectionId = -1;
    if (window->companyUI.widget != nullptr) {
        window->companyUI.widget->setVisible(false);
        window->companyUI.widget->setMaximumHeight(0);
    }
    if (window->mainTabWidget != nullptr &&
        window->mainTabWidget->tabBar() != nullptr) {
        window->mainTabWidget->tabBar()->setVisible(false);
    }
    if (window->projectUI.detailTitle != nullptr) {
        window->projectUI.detailTitle->setText(
            QString("Current Project — %1").arg(project->getName()));
    }
    if (window->projectUI.detailInfoText != nullptr) {
        window->projectUI.detailInfoText->setVisible(true);
        window->projectUI.detailInfoText->setHtml(
            HtmlGenerator::generateProjectDetailHtml(*project,
                                                     window->currentCompany));
    }
    if (window->projectUI.listContainer != nullptr) {
        window->projectUI.listContainer->setVisible(false);
    }
    if (window->projectUI.detailContainer != nullptr) {
        window->projectUI.detailContainer->setVisible(true);
    }

    populateProjectTasksTable(window, *project);
    MainWindowUIHelper::selectProjectRowById(window, projectId);

    if (window->projectUI.detailContainer != nullptr) {
        window->projectUI.detailContainer->update();
        window->projectUI.detailContainer->repaint();
    }
    if (window->mainTabWidget != nullptr) {
        window->mainTabWidget->update();
        window->mainTabWidget->repaint();
    }
    window->update();
    window->repaint();
}

void MainWindowProjectDetailHelper::hideProjectDetails(MainWindow* window) {
    if (!window) return;

    window->detailedProjectId = -1;
    window->pendingTaskSelectionId = -1;

    if (window->companyUI.widget != nullptr) {
        window->companyUI.widget->setVisible(true);
        window->companyUI.widget->setMaximumHeight(QWIDGETSIZE_MAX);
    }
    if (window->mainTabWidget != nullptr &&
        window->mainTabWidget->tabBar() != nullptr) {
        window->mainTabWidget->tabBar()->setVisible(true);
    }
    if (window->projectUI.listContainer != nullptr) {
        window->projectUI.listContainer->setVisible(true);
    }
    if (window->projectUI.detailContainer != nullptr) {
        window->projectUI.detailContainer->setVisible(false);
    }
    if (window->projectUI.detailTitle != nullptr) {
        window->projectUI.detailTitle->setText("Current Project");
    }
    if (window->projectUI.detailInfoText != nullptr) {
        window->projectUI.detailInfoText->clear();
        window->projectUI.detailInfoText->setVisible(false);
    }
    if (window->projectUI.tasksTable != nullptr) {
        window->projectUI.tasksTable->clearContents();
        window->projectUI.tasksTable->setRowCount(0);
    }
}

void MainWindowProjectDetailHelper::refreshProjectDetailView(
    MainWindow* window) {
    if (!window) return;

    if (window->projectUI.detailContainer == nullptr ||
        !window->projectUI.detailContainer->isVisible()) {
        return;
    }
    if (window->currentCompany == nullptr || window->detailedProjectId < 0) {
        hideProjectDetails(window);
        return;
    }

    const Project* project =
        window->currentCompany->getProject(window->detailedProjectId);
    if (project == nullptr) {
        hideProjectDetails(window);
        return;
    }

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(
        window->currentCompany, window->detailedProjectId);
    project = window->currentCompany->getProject(window->detailedProjectId);

    if (window->projectUI.detailTitle != nullptr) {
        window->projectUI.detailTitle->setText(
            QString("Current Project — %1").arg(project->getName()));
    }
    if (window->projectUI.detailInfoText != nullptr) {
        window->projectUI.detailInfoText->setVisible(true);
        window->projectUI.detailInfoText->setHtml(
            HtmlGenerator::generateProjectDetailHtml(*project,
                                                     window->currentCompany));
    }

    populateProjectTasksTable(window, *project);
    MainWindowUIHelper::selectProjectRowById(window, window->detailedProjectId);
}

void MainWindowProjectDetailHelper::populateProjectTasksTable(
    MainWindow* window, const Project& project) {
    if (!window || !window->projectUI.tasksTable || !window->currentCompany)
        return;

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(
        window->currentCompany, project.getId());
    const Project* updatedProject =
        window->currentCompany->getProject(project.getId());
    if (updatedProject) {
        ProjectHelper::populateProjectTasksTable(window->projectUI.tasksTable,
                                                 *updatedProject, window);
    }
}

bool MainWindowValidationHelper::checkCompanyAndHandleError(
    MainWindow* window, const QString& actionName) {
    if (!window) return false;

    if (window->currentCompany != nullptr) {
        return true;
    }

    QMessageBox msgBox(window);
    msgBox.setWindowTitle("No Company Selected");
    msgBox.setText(QString("Please create a company first before %1.\n\n"
                           "Use the company selector at the top to create or "
                           "select a company.")
                       .arg(actionName));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Yes);

    QAbstractButton* okBtn = msgBox.button(QMessageBox::Ok);
    QAbstractButton* clearBtn = msgBox.button(QMessageBox::Yes);
    if (okBtn) okBtn->setText("OK");
    if (clearBtn) clearBtn->setText("Clear All Data Files");

    if (int reply = msgBox.exec(); reply == QMessageBox::Yes) {
        FileHelper::clearAllDataFiles(window);
    }

    return false;
}

bool MainWindowValidationHelper::checkDuplicateProjectOnEdit(
    const QString& projectName, int excludeId, const Company* currentCompany) {
    if (!currentCompany) return true;

    auto projects = currentCompany->getAllProjects();
    return std::ranges::none_of(projects, [excludeId,
                                           &projectName](const auto& project) {
        return project.getId() != excludeId && project.getName() == projectName;
    });
}

void MainWindowValidationHelper::validateAndFixProjectAssignments(
    MainWindow* window, Company* company) {
    if (!window || !company) return;

    bool hasWarnings = false;
    QStringList warningMessages;
    auto projects = company->getAllProjects();

    for (const auto& project : projects) {
        if (project.getAllocatedHours() > 0 &&
            !ProjectHelper::hasAssignedEmployees(company, project.getId())) {
            auto oldAllocatedHours = project.getAllocatedHours();
            ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(
                company, project.getId());
            hasWarnings = true;
            warningMessages.append(
                QString("Project '%1': Found allocated hours (%2h) but no "
                        "assigned employees. Hours have been cleared.")
                    .arg(project.getName(),
                         QString::number(oldAllocatedHours)));
        }
    }

    if (hasWarnings) {
        QMessageBox::warning(
            window, "Data Validation Warning",
            "Data inconsistency detected:\n\n" + warningMessages.join("\n"));
    }
}

bool MainWindowProjectDialogHandler::validateProjectEditFields(
    QDialog& dialog, const ProjectDialogHelper::ProjectDialogFields& fields,
    ProjectEditData& data) {
    data.projectName = fields.nameEdit->text().trimmed();
    if (!ValidationHelper::validateNonEmpty(data.projectName, "Project name",
                                            &dialog))
        return false;

    if (!ValidationHelper::validateDouble(fields.budgetEdit->text().trimmed(),
                                          data.projectBudget, 0.0, kMaxBudget,
                                          "Budget", &dialog))
        return false;

    data.selectedPhase = fields.phaseCombo->currentText();
    data.clientName = fields.clientNameEdit->text().trimmed();

    if (!ValidationHelper::validateNonEmpty(data.clientName, "Client name",
                                            &dialog))
        return false;

    if (!ValidationHelper::validateDateRange(fields.startDate->date(),
                                             fields.endDate->date(), &dialog))
        return false;

    if (!ValidationHelper::validateInt(
            fields.estimatedHoursEdit->text().trimmed(), data.estimatedHours, 0,
            kMaxEstimatedHours, "Estimated hours", &dialog))
        return false;

    data.newName = fields.nameEdit->text().trimmed();
    data.newDescription = fields.descEdit->toPlainText().trimmed();
    data.newPhase = fields.phaseCombo->currentText();
    data.newStartDate = fields.startDate->date();
    data.newEndDate = fields.endDate->date();
    data.newClientName = fields.clientNameEdit->text().trimmed();

    return true;
}

bool MainWindowProjectDialogHandler::checkProjectChanges(
    const Project* oldProject, const ProjectEditData& data) {
    return oldProject->getName() != data.newName ||
           oldProject->getDescription() != data.newDescription ||
           oldProject->getPhase() != data.newPhase ||
           oldProject->getStartDate() != data.newStartDate ||
           oldProject->getEndDate() != data.newEndDate ||
           oldProject->getBudget() != data.projectBudget ||
           oldProject->getClientName() != data.newClientName ||
           oldProject->getEstimatedHours() != data.estimatedHours;
}

bool MainWindowProjectDialogHandler::validatePhaseTransition(
    QDialog& dialog, const QString& currentPhase, const QString& newPhase) {
    if (currentPhase == newPhase) {
        return true;
    }

    auto currentPhaseOrder = Project::getPhaseOrder(currentPhase);

    if (int newPhaseOrder = Project::getPhaseOrder(newPhase);
        currentPhaseOrder >= 0 && newPhaseOrder >= 0 &&
        newPhaseOrder < currentPhaseOrder) {
        QMessageBox::warning(
            &dialog, "Phase Validation Error",
            QString("Cannot set phase to '%1' because current "
                    "phase '%2' is already later in the project "
                    "lifecycle.\n\n"
                    "Phase order: Analysis → Planning → Design → "
                    "Development → Testing → Deployment → "
                    "Maintenance → Completed\n\n"
                    "You can only move forward in the project "
                    "lifecycle, not backward.")
                .arg(newPhase, currentPhase));
        return false;
    }
    return true;
}

void MainWindowProjectDialogHandler::updateProjectWithChanges(
    MainWindow* window, int projectId, const ProjectEditData& data,
    const ProjectDialogHelper::ProjectDialogFields& fields,
    const Project* oldProject) {
    if (!window || !window->currentCompany) return;

    std::vector<Task> savedTasks = oldProject->getTasks();
    double savedEmployeeCosts = oldProject->getEmployeeCosts();

    std::vector<std::tuple<int, int, int, int>> savedTaskAssignments;
    MainWindowTaskAssignmentHelper::collectTaskAssignments(
        window, projectId, savedTasks, savedTaskAssignments);

    ProjectParams updatedProjectParams{projectId,
                                       fields.nameEdit->text().trimmed(),
                                       fields.descEdit->toPlainText().trimmed(),
                                       fields.phaseCombo->currentText(),
                                       fields.startDate->date(),
                                       fields.endDate->date(),
                                       data.projectBudget,
                                       data.clientName,
                                       data.estimatedHours};
    Project updatedProject(updatedProjectParams);

    for (const auto& task : savedTasks) {
        updatedProject.addTask(task);
    }

    if (savedEmployeeCosts > 0.0) {
        updatedProject.addEmployeeCost(savedEmployeeCosts);
    }

    window->currentCompany->removeProject(projectId);
    window->currentCompany->addProject(updatedProject);

    for (const auto& assignment : savedTaskAssignments) {
        const auto& [empId, projId, taskId, hours] = assignment;
        try {
            window->currentCompany->restoreTaskAssignment(empId, projId, taskId,
                                                          hours);
        } catch (const CompanyException&) {
            continue;
        }
    }

    auto projects = window->currentCompany->getAllProjects();
    window->nextProjectId =
        IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
}

void MainWindowProjectDialogHandler::showProjectUpdateSuccess(
    QDialog& dialog, const ProjectEditData& data) {
    dialog.hide();
    QMessageBox::information(dialog.parentWidget(), "Success",
                             "Project updated successfully!\n\n"
                             "Name: " +
                                 data.projectName +
                                 "\n"
                                 "Phase: " +
                                 data.newPhase +
                                 "\n"
                                 "Budget: $" +
                                 QString::number(data.projectBudget, 'f', 2));
    dialog.accept();
}

void MainWindowProjectDialogHandler::handleEditProjectDialog(
    MainWindow* window, int projectId, QDialog& dialog,
    const ProjectDialogHelper::ProjectDialogFields& fields) {
    if (!window) return;

    try {
        ProjectEditData data;
        if (!validateProjectEditFields(dialog, fields, data)) {
            return;
        }

        if (!MainWindowValidationHelper::checkDuplicateProjectOnEdit(
                data.projectName, projectId, window->currentCompany)) {
            QWidget* parent = dialog.parentWidget();
            if (!parent) {
                parent = &dialog;
            }
            QMessageBox::warning(parent, "Duplicate Error",
                                 "A project with this name already exists!\n\n"
                                 "Project name: \"" +
                                     data.projectName +
                                     "\"\n"
                                     "Project ID: " +
                                     QString::number(projectId) +
                                     "\n"
                                     "Please choose a different name.");
            return;
        }

        const Project* oldProject =
            window->currentCompany->getProject(projectId);
        if (!oldProject) {
            QWidget* parent = dialog.parentWidget();
            if (!parent) {
                parent = &dialog;
            }
            QMessageBox::warning(parent, "Error", "Project not found!");
            return;
        }

        if (!checkProjectChanges(oldProject, data)) {
            QWidget* parent = window;
            QMessageBox::information(
                parent, "No Changes",
                "No changes were made to the project.\n\n"
                "Please modify at least one field before saving.");
            dialog.raise();
            dialog.activateWindow();
            return;
        }

        if (!validatePhaseTransition(dialog, oldProject->getPhase(),
                                     data.newPhase)) {
            return;
        }

        updateProjectWithChanges(window, projectId, data, fields, oldProject);
        MainWindowDataOperations::refreshAllData(window);
        MainWindowDataOperations::autoSave(window);
        showProjectUpdateSuccess(dialog, data);
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &dialog, "edit project");
    } catch (const FileManagerException& e) {
        ExceptionHandler::handleFileManagerException(e, &dialog,
                                                     "project update");
    } catch (const ProjectException& e) {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

void MainWindowProjectDialogHandler::handleAddTaskDialog(
    MainWindow* window, int projectId, QDialog& dialog,
    const QLineEdit* taskNameEdit, const QComboBox* taskTypeCombo,
    const QLineEdit* taskEstHoursEdit, const QLineEdit* priorityEdit) {
    if (!window) return;

    try {
        auto taskName = taskNameEdit->text().trimmed();
        if (!ValidationHelper::validateNonEmpty(taskName, "Task name", &dialog))
            return;

        int taskEst = 0;
        if (!ValidationHelper::validateInt(taskEstHoursEdit->text().trimmed(),
                                           taskEst, 1, kMaxEstimatedHours,
                                           "Estimated hours", &dialog))
            return;

        int priority = 0;
        if (!ValidationHelper::validateInt(priorityEdit->text().trimmed(),
                                           priority, 0, kMaxPriority,
                                           "Priority", &dialog))
            return;

        if (!TaskDialogHelper::validateAndAddTask(
                taskName, taskTypeCombo->currentText(), taskEst, priority,
                projectId, window->currentCompany, &dialog)) {
            return;
        }

        if (auto* project = window->currentCompany->getProject(projectId); project) {
            project->recomputeTotalsFromTasks();
        }

        MainWindowDataOperations::refreshAllData(window);
        MainWindowDataOperations::autoSave(window);
        dialog.hide();
        QMessageBox::information(
            dialog.parentWidget(), "Success",
            QString("Task added successfully!\n\n"
                    "Task name: %1\n"
                    "Type: %2\n"
                    "Estimated hours: %3\n"
                    "Priority: %4")
                .arg(taskName, taskTypeCombo->currentText(),
                     QString::number(taskEst), QString::number(priority)));
        dialog.accept();
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &dialog, "add task");
    } catch (const TaskException& e) {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

void MainWindowProjectDialogHandler::handleAssignEmployeeToTaskDialog(
    MainWindow* window, int projectId, QDialog& dialog,
    const QComboBox* taskCombo, const QComboBox* employeeCombo,
    const QLineEdit* hoursEdit, const std::vector<Task>& tasks) {
    if (!window) return;

    try {
        int taskId = taskCombo->currentData().toInt();
        int employeeId = employeeCombo->currentData().toInt();

        auto employee = window->currentCompany->getEmployee(employeeId);
        if (!employee) {
            QWidget* parent = dialog.parentWidget();
            if (!parent) {
                parent = &dialog;
            }
            QMessageBox::warning(parent, "Error", "Employee not found!");
            return;
        }

        auto availableHours = employee->getAvailableHours();
        int maxHours = std::min(kMaxHoursPerWeek, availableHours);

        if (maxHours <= 0) {
            QWidget* parent = dialog.parentWidget();
            if (!parent) {
                parent = &dialog;
            }
            QMessageBox::warning(
                parent, "Error",
                QString("Employee '%1' has no available hours.\n\n"
                        "Weekly capacity: %2h\n"
                        "Currently used: %3h\n"
                        "Available: %4h\n\n"
                        "The employee cannot be assigned to more tasks.")
                    .arg(employee->getName())
                    .arg(employee->getWeeklyHoursCapacity())
                    .arg(employee->getCurrentWeeklyHours())
                    .arg(availableHours));
            return;
        }

        int hours = 0;
        if (!ValidationHelper::validateInt(hoursEdit->text().trimmed(), hours,
                                           1, maxHours, "Hours per week",
                                           &dialog))
            return;

        window->currentCompany->assignEmployeeToTask(employeeId, projectId,
                                                     taskId, hours);
        window->currentCompany->recalculateTaskAllocatedHours();
        
        if (auto* project = window->currentCompany->getProject(projectId); project) {
            project->recomputeTotalsFromTasks();
        }
        
        MainWindowDataOperations::refreshAllData(window);
        MainWindowDataOperations::autoSave(window);

        QString taskName;
        QString employeeName;
        for (const auto& task : tasks) {
            if (task.getId() == taskId) {
                taskName = task.getName();
                break;
            }
        }
        if (auto emp = window->currentCompany->getEmployee(employeeId)) {
            employeeName = emp->getName();
        }

        dialog.hide();
        QMessageBox::information(
            dialog.parentWidget(), "Success",
            QString("Employee assigned to task successfully!\n\n"
                    "Task: %1\n"
                    "Employee: %2\n"
                    "Hours per week: %3")
                .arg(taskName, employeeName, QString::number(hours)));
        dialog.accept();
    } catch (const CompanyException& e) {
        ExceptionHandler::handleCompanyException(e, &dialog,
                                                 "assign employee to task");
    } catch (const EmployeeException& e) {
        ExceptionHandler::handleGenericException(e, &dialog);
    } catch (const TaskException& e) {
        ExceptionHandler::handleGenericException(e, &dialog);
    }
}

void MainWindowTaskAssignmentHelper::removeEmployeeFromProjectTasks(
    const MainWindow* window, int employeeId, int projectId,
    Project* mutableProject, double employeeHourlyRate) {
    if (!window || !window->currentCompany || !mutableProject) return;

    const Company* company = window->currentCompany;
    double totalCostToRemove = 0.0;

    for (auto& task : mutableProject->getTasks()) {
        auto taskId = task.getId();
        auto employeeTaskHours =
            company->getEmployeeHours(employeeId, projectId, taskId);

        if (employeeTaskHours > 0) {
            auto currentAllocated = task.getAllocatedHours();
            int newAllocated = currentAllocated - employeeTaskHours;
            if (newAllocated < 0) {
                newAllocated = 0;
            }
            task.setAllocatedHours(newAllocated);

            double taskCost = employeeHourlyRate * employeeTaskHours;
            totalCostToRemove += taskCost;
        }
    }

    if (totalCostToRemove > 0) {
        double currentCosts = mutableProject->getEmployeeCosts();
        double costToRemove = std::min(totalCostToRemove, currentCosts);
        if (costToRemove > 0) {
            mutableProject->removeEmployeeCost(costToRemove);
        }
    }
}

void MainWindowTaskAssignmentHelper::collectTaskAssignments(
    const MainWindow* window, int projectId,
    const std::vector<Task>& savedTasks,
    std::vector<std::tuple<int, int, int, int>>& savedTaskAssignments) {
    if (!window || !window->currentCompany) return;

    auto allEmployees = window->currentCompany->getAllEmployees();
    for (const auto& emp : allEmployees) {
        if (!emp) continue;
        for (const auto& task : savedTasks) {
            auto taskId = task.getId();
            auto hours = window->currentCompany->getEmployeeHours(
                emp->getId(), projectId, taskId);
            if (hours > 0) {
                savedTaskAssignments.push_back(
                    std::make_tuple(emp->getId(), projectId, taskId, hours));
            }
        }
    }
}

void MainWindowTaskAssignmentHelper::handleEmployeeActiveAssignments(
    const MainWindow* window, int employeeId,
    const std::shared_ptr<Employee>& employee) {
    if (!window || !window->currentCompany || !employee) return;

    const std::vector<int>& assignedProjects = employee->getAssignedProjects();

    for (int projectId : assignedProjects) {
        Project* mutableProject = window->currentCompany->getProject(projectId);
        if (mutableProject) {
            auto employeeHourlyRate = employee->getSalary() / 160.0;
            removeEmployeeFromProjectTasks(window, employeeId, projectId,
                                           mutableProject, employeeHourlyRate);
            mutableProject->recomputeTotalsFromTasks();
        }
    }

    auto currentHours = employee->getCurrentWeeklyHours();
    employee->removeWeeklyHours(currentHours);

    for (int projectId : assignedProjects) {
        employee->addToProjectHistory(projectId);
        employee->removeAssignedProject(projectId);
    }
}

void MainWindowUIHelper::setupUI(MainWindow* window) {
    MainWindowUIBuilder::setupMainUI(window);
}

void MainWindowUIHelper::setupEmployeeTab(MainWindow* window) {
    if (window->mainTabWidget) {
        MainWindowUIBuilder::setupEmployeeTab(window, window->mainTabWidget);
    }
}

void MainWindowUIHelper::setupProjectTab(MainWindow* window) {
    if (window->mainTabWidget) {
        MainWindowUIBuilder::setupProjectTab(window, window->mainTabWidget);
    }
}

void MainWindowUIHelper::setupStatisticsTab(MainWindow* window) {
    if (window->mainTabWidget) {
        MainWindowUIBuilder::setupStatisticsTab(window, window->mainTabWidget);
    }
}

void MainWindowUIHelper::drawStatisticsChart(MainWindow* window,
                                             QWidget* widget) {
    if (widget == nullptr || window->currentCompany == nullptr) return;

    if (window->statisticsUI.chartInnerWidget == nullptr) {
        window->statisticsUI.chartInnerWidget =
            new StatisticsChartWidget(widget);
        auto* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(window->statisticsUI.chartInnerWidget);
    }

    auto* chartWidget = static_cast<StatisticsChartWidget*>(
        window->statisticsUI.chartInnerWidget);
    chartWidget->setData(window->currentCompany);
}

void MainWindowUIHelper::clearAllDataFiles(MainWindow* window) {
    FileHelper::clearAllDataFiles(window);
}

void MainWindowUIHelper::setupTableWidget(QTableWidget* table,
                                          const QStringList& headers,
                                          const QList<int>& columnWidths,
                                          bool stretchLast) {
    if (!table) return;

    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setVisible(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->horizontalHeader()->setStretchLastSection(stretchLast);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget::item:selected { background-color: #c8c8c8; color: "
        "black; }"
        "QTableWidget::item:selected:active { background-color: #c8c8c8; }"
        "QTableWidget::item:selected:!active { background-color: #c8c8c8; }");
    table->setShowGrid(true);
    table->verticalHeader()->setDefaultSectionSize(58);
    table->verticalHeader()->setVisible(false);

    for (int i = 0; i < columnWidths.size() && i < headers.size(); ++i) {
        table->setColumnWidth(i, columnWidths[i]);
    }
}

QWidget* MainWindowUIHelper::createEmployeeActionButtons(MainWindow* window,
                                                         int rowIndex) {
    return ActionButtonHelper::createEmployeeActionButtons(
        window->employeeUI.table, rowIndex, window);
}

QWidget* MainWindowUIHelper::createProjectActionButtons(MainWindow* window,
                                                        int rowIndex) {
    return ActionButtonHelper::createProjectActionButtons(
        window->projectUI.table, rowIndex, window);
}

void MainWindowDataOperations::refreshAllData(MainWindow* window) {
    if (!window) return;

    if (window->currentCompany != nullptr) {
        MainWindowValidationHelper::validateAndFixProjectAssignments(
            window, window->currentCompany);
        window->currentCompany->recalculateTaskAllocatedHours();
    }

    EmployeeOperations::refreshEmployeeTable(window);
    ProjectOperations::refreshProjectTable(window);
    ProjectOperations::showStatistics(window);

    if (window->projectUI.detailContainer != nullptr &&
        window->projectUI.detailContainer->isVisible() &&
        window->detailedProjectId >= 0) {
        MainWindowProjectDetailHelper::refreshProjectDetailView(window);
    }
}

void MainWindowDataOperations::autoSave(MainWindow* window) {
    if (!window) return;
    AutoSaveLoader::autoSave(window->companies, window);
}

void MainWindowDataOperations::autoLoad(MainWindow* window) {
    if (!window) return;

    AutoSaveLoader::autoLoad(window->companies, window->currentCompany,
                             window->currentCompanyIndex, window);

    if (window->companyUI.selector) {
        CompanyOperations::refreshCompanyList(window);
        if (window->currentCompanyIndex >= 0 &&
            window->currentCompanyIndex < window->companyUI.selector->count()) {
            window->companyUI.selector->setCurrentIndex(
                window->currentCompanyIndex);
        }
    }

    EmployeeOperations::refreshEmployeeTable(window);
    ProjectOperations::refreshProjectTable(window);
    ProjectOperations::showStatistics(window);
}

void MainWindowDataOperations::selectProjectRowById(MainWindow* window,
                                                    int projectId) {
    MainWindowUIHelper::selectProjectRowById(window, projectId);
}

void MainWindowUIHelper::selectProjectRowById(MainWindow* window,
                                              int projectId) {
    if (window->projectUI.table == nullptr) return;

    int rowCount = window->projectUI.table->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const QTableWidgetItem* item = window->projectUI.table->item(row, 0);
        if (item == nullptr) {
            continue;
        }
        bool ok = false;
        int idValue = item->text().toInt(&ok);
        if (ok && idValue == projectId) {
            window->projectUI.table->setCurrentCell(row, 0);
            break;
        }
    }
}
