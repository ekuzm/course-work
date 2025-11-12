#include "main_window.h"

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
#include <QTimer>
#include <QVBoxLayout>
#include <algorithm>
#include <fstream>
#include <map>
#include <ranges>
#include <set>

#include "action_button_helper.h"
#include "app_styles.h"
#include "auto_save_loader.h"
#include "derived_employees.h"
#include "dialog_helper.h"
#include "display_helper.h"
#include "employee_dialog_handler.h"
#include "employee_validator.h"
#include "exception_handler.h"
#include "file_helper.h"
#include "html_generator.h"
#include "id_helper.h"
#include "main_window_ui_builder.h"
#include "project_dialog_helper.h"
#include "project_helper.h"
#include "statistics_chart_widget.h"
#include "task_assignment_helper.h"
#include "task_dialog_helper.h"
#include "validation_helper.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    autoLoad();
    initializeCompanySetup();
}

void MainWindow::initializeCompanySetup() {
    DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
    hideProjectDetails();
    refreshProjectTable();
    showStatistics();

    if (currentCompany != nullptr) {
        auto employees = currentCompany->getAllEmployees();
        nextEmployeeId =
            IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));

        auto projects = currentCompany->getAllProjects();
        nextProjectId =
            IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
    }
}

MainWindow::~MainWindow() {
    autoSave();
    for (auto* company : companies) {
        delete company;
    }
}

void MainWindow::setupUI() { MainWindowUIBuilder::setupMainUI(this); }

void MainWindow::setupEmployeeTab() {
    if (mainTabWidget) {
        MainWindowUIBuilder::setupEmployeeTab(this, mainTabWidget);
    }
}

void MainWindow::setupProjectTab() {
    if (mainTabWidget) {
        MainWindowUIBuilder::setupProjectTab(this, mainTabWidget);
    }
}

void MainWindow::setupStatisticsTab() {
    if (mainTabWidget) {
        MainWindowUIBuilder::setupStatisticsTab(this, mainTabWidget);
    }
}

int MainWindow::getSelectedEmployeeId() const {
    if (int rowIndex = employeeTable->currentRow(); rowIndex >= 0) {
        if (const QTableWidgetItem* tableItem =
                employeeTable->item(rowIndex, 0);
            tableItem != nullptr) {
            bool conversionSuccess = false;
            int employeeId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? employeeId : -1;
        }
    }
    return -1;
}

int MainWindow::getSelectedProjectId() const {
    if (projectTable != nullptr) {
        int rowIndex = projectTable->currentRow();
        if (rowIndex >= 0) {
            const QTableWidgetItem* tableItem = projectTable->item(rowIndex, 0);
            if (tableItem != nullptr) {
                bool conversionSuccess = false;
                int projectId = tableItem->text().toInt(&conversionSuccess);
                if (conversionSuccess) {
                    return projectId;
                }
            }
        }
    }

    if (projectDetailContainer != nullptr &&
        projectDetailContainer->isVisible() && detailedProjectId >= 0) {
        return detailedProjectId;
    }
    return -1;
}

bool MainWindow::checkDuplicateProjectOnEdit(const QString& projectName,
                                             int excludeId,
                                             const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingProjects = currentCompany->getAllProjects();
    auto duplicateFound = std::ranges::any_of(
        existingProjects, [&projectName, excludeId](const auto& project) {
            return project.getId() != excludeId &&
                   project.getName().toLower() == projectName.toLower();
        });
    return !duplicateFound;
}

void MainWindow::addEmployee() {
    if (!checkCompanyAndHandleError("adding employees")) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* mainLayout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    mainLayout->addLayout(form);
    QComboBox* typeCombo = nullptr;
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

    EmployeeDialogHelper::createEmployeeDialog(
        dialog, form, typeCombo, nameEdit, salaryEdit, deptEdit,
        employmentRateCombo, managerProject, devLanguage, devExperience,
        designerTool, designerProjects, qaTestType, qaBugs, managerProjectLabel,
        devLanguageLabel, devExperienceLabel, designerToolLabel,
        designerProjectsLabel, qaTestTypeLabel, qaBugsLabel);

    auto projects = currentCompany->getAllProjects();
    managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        managerProject->addItem(proj.getName(), proj.getId());
    }

    mainLayout->addStretch();
    auto* okButton = new QPushButton("OK");
    mainLayout->addWidget(okButton);

    if (managerProjectLabel) managerProjectLabel->setVisible(true);
    if (managerProject) managerProject->setVisible(true);
    if (devLanguageLabel) devLanguageLabel->setVisible(true);
    if (devLanguage) devLanguage->setVisible(true);
    if (devExperienceLabel) devExperienceLabel->setVisible(true);
    if (devExperience) devExperience->setVisible(true);
    if (designerToolLabel) designerToolLabel->setVisible(true);
    if (designerTool) designerTool->setVisible(true);
    if (designerProjectsLabel) designerProjectsLabel->setVisible(true);
    if (designerProjects) designerProjects->setVisible(true);
    if (qaTestTypeLabel) qaTestTypeLabel->setVisible(true);
    if (qaTestType) qaTestType->setVisible(true);
    if (qaBugsLabel) qaBugsLabel->setVisible(true);
    if (qaBugs) qaBugs->setVisible(true);

    dialog.adjustSize();
    QSize maxSize = dialog.size();
    maxSize.setHeight(maxSize.height() - 227);

    if (typeCombo) {
        int index = typeCombo->currentIndex();
        EmployeeDialogHelper::showDeveloperFields(devLanguageLabel, devLanguage,
                                                  devExperienceLabel,
                                                  devExperience, index == 1);
        EmployeeDialogHelper::showDesignerFields(
            designerToolLabel, designerTool, designerProjectsLabel,
            designerProjects, index == 2);
        EmployeeDialogHelper::showQaFields(qaTestTypeLabel, qaTestType,
                                           qaBugsLabel, qaBugs, index == 3);
        EmployeeDialogHelper::showManagerFields(managerProjectLabel,
                                                managerProject, index == 0);
    }

    dialog.setFixedSize(maxSize);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            if (!EmployeeDialogHandler::processAddEmployee(
                    &dialog, currentCompany, nextEmployeeId, nameEdit,
                    salaryEdit, deptEdit, typeCombo, employmentRateCombo,
                    managerProject, devLanguage, devExperience, designerTool,
                    designerProjects, qaTestType, qaBugs)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(&dialog, "Success",
                                     "Employee added successfully!\n\n"
                                     "Name: " +
                                         nameEdit->text().trimmed() +
                                         "\nType: " + typeCombo->currentText() +
                                         "\nSalary: $" +
                                         salaryEdit->text().trimmed());
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog,
                                                     "add employee");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog,
                                                         "employee");
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
}

void MainWindow::editEmployee() {
    if (!checkCompanyAndHandleError("editing employees")) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to edit.");
        return;
    }

    auto employee = currentCompany->getEmployee(employeeId);
    if (employee == nullptr) {
        QMessageBox::warning(this, "Error", "Employee not found!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* mainLayout = new QVBoxLayout(&dialog);
    auto* form = new QFormLayout();
    mainLayout->addLayout(form);
    QString currentType = employee->getEmployeeType();
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

    EmployeeDialogHelper::createEditEmployeeDialog(
        dialog, form, employee, nameEdit, salaryEdit, deptEdit,
        employmentRateCombo, managerProject, devLanguage, devExperience,
        designerTool, designerProjects, qaTestType, qaBugs, managerProjectLabel,
        devLanguageLabel, devExperienceLabel, designerToolLabel,
        designerProjectsLabel, qaTestTypeLabel, qaBugsLabel);

    auto projects = currentCompany->getAllProjects();
    managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        managerProject->addItem(proj.getName(), proj.getId());
    }

    if (const auto* manager = dynamic_cast<const Manager*>(employee.get())) {
        int currentProjectId = manager->getManagedProjectId();
        int index = managerProject->findData(currentProjectId);
        if (index >= 0) {
            managerProject->setCurrentIndex(index);
        }
    }

    mainLayout->addStretch();
    auto* okButton = new QPushButton("OK");
    mainLayout->addWidget(okButton);

    if (managerProjectLabel) managerProjectLabel->setVisible(true);
    if (managerProject) managerProject->setVisible(true);
    if (devLanguageLabel) devLanguageLabel->setVisible(true);
    if (devLanguage) devLanguage->setVisible(true);
    if (devExperienceLabel) devExperienceLabel->setVisible(true);
    if (devExperience) devExperience->setVisible(true);
    if (designerToolLabel) designerToolLabel->setVisible(true);
    if (designerTool) designerTool->setVisible(true);
    if (designerProjectsLabel) designerProjectsLabel->setVisible(true);
    if (designerProjects) designerProjects->setVisible(true);
    if (qaTestTypeLabel) qaTestTypeLabel->setVisible(true);
    if (qaTestType) qaTestType->setVisible(true);
    if (qaBugsLabel) qaBugsLabel->setVisible(true);
    if (qaBugs) qaBugs->setVisible(true);

    dialog.adjustSize();
    QSize maxSize = dialog.size();
    maxSize.setHeight(maxSize.height() - 227);

    EmployeeDialogHelper::showManagerFields(managerProjectLabel, managerProject,
                                            currentType == "Manager");
    EmployeeDialogHelper::showDeveloperFields(devLanguageLabel, devLanguage,
                                              devExperienceLabel, devExperience,
                                              currentType == "Developer");
    EmployeeDialogHelper::showDesignerFields(
        designerToolLabel, designerTool, designerProjectsLabel,
        designerProjects, currentType == "Designer");
    EmployeeDialogHelper::showQaFields(qaTestTypeLabel, qaTestType, qaBugsLabel,
                                       qaBugs, currentType == "QA");

    dialog.setFixedSize(maxSize);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            if (!EmployeeDialogHandler::processEditEmployee(
                    &dialog, currentCompany, employeeId, nextEmployeeId,
                    nameEdit, salaryEdit, deptEdit, employmentRateCombo,
                    managerProject, devLanguage, devExperience, designerTool,
                    designerProjects, qaTestType, qaBugs, currentType)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
                "Employee updated successfully!\n\n"
                "Name: " +
                    nameEdit->text().trimmed() + "\nType: " + currentType +
                    "\nSalary: $" + salaryEdit->text().trimmed());
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog,
                                                     "edit employee");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog,
                                                         "employee update");
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
}

void MainWindow::deleteEmployee() {
    if (!checkCompanyAndHandleError("deleting employees")) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to delete.");
        return;
    }

    int userChoice =
        QMessageBox::question(this, "Confirm Delete",
                              "Are you sure you want to delete this employee?",
                              QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            currentCompany->removeEmployee(employeeId);
            auto employees = currentCompany->getAllEmployees();
            nextEmployeeId = IdHelper::calculateNextId(
                IdHelper::findMaxEmployeeId(employees));
            refreshAllData();
            autoSave();
            QMessageBox::information(this, "Success",
                                     "Employee deleted successfully!");

        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

void MainWindow::fireEmployee() {
    if (!checkCompanyAndHandleError("firing employees")) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to fire.");
        return;
    }

    auto employee = currentCompany->getEmployee(employeeId);
    if (!employee) {
        QMessageBox::warning(this, "Error", "Employee not found.");
        return;
    }

    if (!employee->getIsActive()) {
        QMessageBox::information(this, "Information",
                                 "This employee is already fired.");
        return;
    }

    if (employee->getCurrentWeeklyHours() > 0) {
        int userChoice = QMessageBox::question(
            this, "Confirm Fire",
            QString("This employee has active assignments (%1 hours/week).\n\n"
                    "Are you sure you want to fire this employee?\n\n"
                    "This will remove all active assignments.")
                .arg(employee->getCurrentWeeklyHours()),
            QMessageBox::Yes | QMessageBox::No);

        if (userChoice != QMessageBox::Yes) {
            return;
        }

        const std::vector<int>& assignedProjects =
            employee->getAssignedProjects();

        for (int projectId : assignedProjects) {
            const Project* project = currentCompany->getProject(projectId);
            if (project) {
                auto tasks = currentCompany->getProjectTasks(projectId);

                Project* mutableProject =
                    const_cast<Project*>(currentCompany->getProject(projectId));
                if (mutableProject) {
                    double employeeHourlyRate = employee->getSalary() / 160.0;
                    double totalCostToRemove = 0.0;

                    for (auto& task : mutableProject->getTasks()) {
                        int taskId = task.getId();
                        int employeeTaskHours =
                            currentCompany->getEmployeeTaskHours(
                                employeeId, projectId, taskId);

                        if (employeeTaskHours > 0) {
                            int currentAllocated = task.getAllocatedHours();
                            int newAllocated =
                                currentAllocated - employeeTaskHours;
                            if (newAllocated < 0) {
                                newAllocated = 0;
                            }
                            task.setAllocatedHours(newAllocated);

                            double taskCost =
                                employeeHourlyRate * employeeTaskHours;
                            totalCostToRemove += taskCost;
                        }
                    }

                    if (totalCostToRemove > 0) {
                        double currentCosts =
                            mutableProject->getEmployeeCosts();
                        double costToRemove =
                            std::min(totalCostToRemove, currentCosts);
                        if (costToRemove > 0) {
                            mutableProject->removeEmployeeCost(costToRemove);
                        }
                    }

                    mutableProject->recomputeTotalsFromTasks();
                }
            }
        }

        int currentHours = employee->getCurrentWeeklyHours();
        employee->removeWeeklyHours(currentHours);

        for (int projectId : assignedProjects) {
            employee->addToProjectHistory(projectId);
            employee->removeAssignedProject(projectId);
        }
    }

    int userChoice = QMessageBox::question(
        this, "Confirm Fire",
        QString("Are you sure you want to fire %1?").arg(employee->getName()),
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            employee->setIsActive(false);
            refreshAllData();
            autoSave();
            QMessageBox::information(
                this, "Success",
                QString("Employee %1 has been fired successfully!")
                    .arg(employee->getName()));

        } catch (const EmployeeException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to fire employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

void MainWindow::searchEmployee() {
    if (!checkCompanyAndHandleError("searching employees")) return;

    QString searchTerm = employeeSearchEdit->text().trimmed().toLower();

    if (searchTerm.isEmpty()) {
        DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
        return;
    }

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(0);
    int rowIndex = 0;

    for (const auto& employee : employees) {
        if (!employee) continue;

        QString name = employee->getName().toLower();
        QString department = employee->getDepartment().toLower();
        QString position = employee->getPosition().toLower();

        if (name.contains(searchTerm) || department.contains(searchTerm) ||
            position.contains(searchTerm)) {
            employeeTable->insertRow(rowIndex);
            employeeTable->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(employee->getId())));
            employeeTable->setItem(rowIndex, 1,
                                   new QTableWidgetItem(employee->getName()));
            employeeTable->setItem(
                rowIndex, 2, new QTableWidgetItem(employee->getDepartment()));
            employeeTable->setItem(rowIndex, 3,
                                   new QTableWidgetItem(QString::number(
                                       employee->getSalary(), 'f', 2)));
            employeeTable->setItem(
                rowIndex, 4, new QTableWidgetItem(employee->getEmployeeType()));

            QString projectInfo =
                DisplayHelper::formatProjectInfo(employee, currentCompany);
            employeeTable->setItem(rowIndex, 5,
                                   new QTableWidgetItem(projectInfo));

            employeeTable->setCellWidget(rowIndex, 6,
                                         createEmployeeActionButtons(rowIndex));
            rowIndex++;
        }
    }
}

void MainWindow::addProject() {
    if (!checkCompanyAndHandleError("adding projects")) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);
    ProjectDialogHelper::ProjectDialogFields fields;
    ProjectDialogHelper::createProjectDialogFields(dialog, form, fields);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString projectName = fields.nameEdit->text().trimmed();
            if (!ValidationHelper::validateNonEmpty(projectName, "Project name",
                                                    &dialog))
                return;

            double projectBudget = 0.0;
            if (!ValidationHelper::validateDouble(
                    fields.budgetEdit->text().trimmed(), projectBudget, 0.0,
                    kMaxBudget, "Budget", &dialog))
                return;

            QString selectedPhase = fields.phaseCombo->currentText();
            QString clientName = fields.clientNameEdit->text().trimmed();

            if (!ValidationHelper::validateNonEmpty(clientName, "Client name",
                                                    &dialog))
                return;

            if (!ValidationHelper::validateDateRange(
                    fields.startDate->date(), fields.endDate->date(), &dialog))
                return;

            int estimatedHours = 0;
            if (!ValidationHelper::validateInt(
                    fields.estimatedHoursEdit->text().trimmed(), estimatedHours,
                    0, kMaxEstimatedHours, "Estimated hours", &dialog))
                return;

            auto existingProjects = currentCompany->getAllProjects();
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

            int projectId = nextProjectId;
            nextProjectId++;
            Project project(projectId, projectName,
                            fields.descEdit->toPlainText().trimmed(),
                            selectedPhase, fields.startDate->date(),
                            fields.endDate->date(), projectBudget, clientName,
                            estimatedHours);

            currentCompany->addProject(project);
            auto projects = currentCompany->getAllProjects();
            nextProjectId =
                IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
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
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
}

void MainWindow::editProject() {
    if (!checkCompanyAndHandleError("editing projects")) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project to edit.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (project == nullptr) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");
    dialog.setMinimumWidth(400);

    auto* form = new QFormLayout(&dialog);
    ProjectDialogHelper::ProjectDialogFields fields;
    ProjectDialogHelper::createProjectDialogFields(dialog, form, fields);
    ProjectDialogHelper::populateProjectDialogFields(project, fields);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString projectName = fields.nameEdit->text().trimmed();
            if (!ValidationHelper::validateNonEmpty(projectName, "Project name",
                                                    &dialog))
                return;

            double projectBudget = 0.0;
            if (!ValidationHelper::validateDouble(
                    fields.budgetEdit->text().trimmed(), projectBudget, 0.0,
                    kMaxBudget, "Budget", &dialog))
                return;

            QString selectedPhase = fields.phaseCombo->currentText();
            QString clientName = fields.clientNameEdit->text().trimmed();

            if (!ValidationHelper::validateNonEmpty(clientName, "Client name",
                                                    &dialog))
                return;

            if (!ValidationHelper::validateDateRange(
                    fields.startDate->date(), fields.endDate->date(), &dialog))
                return;

            int estimatedHours = 0;
            if (!ValidationHelper::validateInt(
                    fields.estimatedHoursEdit->text().trimmed(), estimatedHours,
                    0, kMaxEstimatedHours, "Estimated hours", &dialog))
                return;

            if (!checkDuplicateProjectOnEdit(projectName, projectId,
                                             currentCompany)) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "A project with this name already exists!\n\n"
                    "Project name: \"" +
                        projectName +
                        "\"\n"
                        "Project ID: " +
                        QString::number(projectId) +
                        "\n"
                        "Please choose a different name.");
                return;
            }

            const Project* oldProject = currentCompany->getProject(projectId);
            if (!oldProject) {
                QMessageBox::warning(&dialog, "Error", "Project not found!");
                return;
            }

            bool hasChanges = false;

            QString newName = fields.nameEdit->text().trimmed();
            QString newDescription = fields.descEdit->toPlainText().trimmed();
            QString newPhase = fields.phaseCombo->currentText();
            QDate newStartDate = fields.startDate->date();
            QDate newEndDate = fields.endDate->date();
            QString newClientName = fields.clientNameEdit->text().trimmed();

            if (oldProject->getName() != newName ||
                oldProject->getDescription() != newDescription ||
                oldProject->getPhase() != newPhase ||
                oldProject->getStartDate() != newStartDate ||
                oldProject->getEndDate() != newEndDate ||
                oldProject->getBudget() != projectBudget ||
                oldProject->getClientName() != newClientName ||
                oldProject->getEstimatedHours() != estimatedHours) {
                hasChanges = true;
            }

            if (!hasChanges) {
                QMessageBox::information(
                    &dialog, "No Changes",
                    "No changes were made to the project.\n\n"
                    "Please modify at least one field before saving.");
                return;
            }

            QString currentPhase = oldProject->getPhase();
            if (currentPhase != newPhase) {
                int currentPhaseOrder = Project::getPhaseOrder(currentPhase);
                int newPhaseOrder = Project::getPhaseOrder(newPhase);

                if (currentPhaseOrder >= 0 && newPhaseOrder >= 0 &&
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
                    return;
                }
            }

            std::vector<Task> savedTasks = oldProject->getTasks();
            double savedEmployeeCosts = oldProject->getEmployeeCosts();

            std::vector<std::tuple<int, int, int, int>> savedTaskAssignments;
            auto allEmployees = currentCompany->getAllEmployees();
            for (const auto& emp : allEmployees) {
                if (!emp) continue;
                for (const auto& task : savedTasks) {
                    int taskId = task.getId();
                    int hours = currentCompany->getEmployeeTaskHours(
                        emp->getId(), projectId, taskId);
                    if (hours > 0) {
                        savedTaskAssignments.push_back(std::make_tuple(
                            emp->getId(), projectId, taskId, hours));
                    }
                }
            }

            Project updatedProject(projectId, fields.nameEdit->text().trimmed(),
                                   fields.descEdit->toPlainText().trimmed(),
                                   fields.phaseCombo->currentText(),
                                   fields.startDate->date(),
                                   fields.endDate->date(), projectBudget,
                                   clientName, estimatedHours);

            for (const auto& task : savedTasks) {
                updatedProject.addTask(task);
            }

            if (savedEmployeeCosts > 0.0) {
                updatedProject.addEmployeeCost(savedEmployeeCosts);
            }

            currentCompany->removeProject(projectId);
            currentCompany->addProject(updatedProject);

            for (const auto& assignment : savedTaskAssignments) {
                int empId = std::get<0>(assignment);
                int projId = std::get<1>(assignment);
                int taskId = std::get<2>(assignment);
                int hours = std::get<3>(assignment);
                try {
                    currentCompany->restoreTaskAssignment(empId, projId, taskId,
                                                          hours);
                } catch (const std::exception&) {
                    continue;
                }
            }

            auto projects = currentCompany->getAllProjects();
            nextProjectId =
                IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
                "Project updated successfully!\n\n"
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
            ExceptionHandler::handleCompanyException(e, &dialog,
                                                     "edit project");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog,
                                                         "project update");
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
}

void MainWindow::deleteProject() {
    if (!checkCompanyAndHandleError("deleting projects")) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select a project to delete.");
        return;
    }

    int userChoice = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete this project?",
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            currentCompany->removeProject(projectId);
            auto projects = currentCompany->getAllProjects();
            nextProjectId =
                IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
            refreshAllData();
            autoSave();
            if (detailedProjectId == projectId) {
                hideProjectDetails();
            }
            QMessageBox::information(this, "Success",
                                     "Project deleted successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to auto-save changes: ") + e.what());
        }
    }
}

QString MainWindow::getDataDirectory() {
    return AutoSaveLoader::getDataDirectory();
}

void MainWindow::autoSave() const {
    AutoSaveLoader::autoSave(companies, const_cast<MainWindow*>(this));
}

void MainWindow::autoLoad() {
    AutoSaveLoader::autoLoad(companies, currentCompany, currentCompanyIndex,
                             this);

    if (companySelector) {
        refreshCompanyList();
        if (currentCompanyIndex >= 0 &&
            currentCompanyIndex < companySelector->count()) {
            companySelector->setCurrentIndex(currentCompanyIndex);
        }
    }

    if (currentCompany) {
        refreshEmployeeTable();
        refreshProjectTable();
        showStatistics();
    } else {
        refreshEmployeeTable();
        refreshProjectTable();
        showStatistics();
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    autoSave();
    event->accept();
}

bool MainWindow::checkCompanyAndHandleError(const QString& actionName) {
    if (currentCompany != nullptr) {
        return true;
    }

    QMessageBox msgBox(this);
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

    int reply = msgBox.exec();
    if (reply == QMessageBox::Yes) {
        clearAllDataFiles();
    }

    return false;
}

void MainWindow::setupTableWidget(QTableWidget* table,
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

QWidget* MainWindow::createEmployeeActionButtons(int rowIndex) {
    return ActionButtonHelper::createEmployeeActionButtons(employeeTable,
                                                           rowIndex, this);
}

QWidget* MainWindow::createProjectActionButtons(int rowIndex) {
    return ActionButtonHelper::createProjectActionButtons(projectTable,
                                                          rowIndex, this);
}

void MainWindow::clearAllDataFiles() { FileHelper::clearAllDataFiles(this); }

void MainWindow::refreshEmployeeTable() const {
    MainWindow* self = const_cast<MainWindow*>(this);
    DisplayHelper::displayEmployees(employeeTable, currentCompany, self);
}

void MainWindow::refreshProjectTable() const {
    MainWindow* self = const_cast<MainWindow*>(this);
    DisplayHelper::displayProjects(projectTable, currentCompany, self);
    if (projectDetailContainer != nullptr &&
        projectDetailContainer->isVisible()) {
        MainWindow* self = const_cast<MainWindow*>(this);
        self->refreshProjectDetailView();
    }
}

void MainWindow::showStatistics() const {
    DisplayHelper::showStatistics(statisticsText, currentCompany);
    if (statisticsChartWidget != nullptr) {
        MainWindow* self = const_cast<MainWindow*>(this);
        self->drawStatisticsChart(statisticsChartWidget);
        statisticsChartWidget->update();
    }
}

void MainWindow::drawStatisticsChart(QWidget* widget) {
    if (widget == nullptr || currentCompany == nullptr) return;

    if (statisticsChartInnerWidget == nullptr) {
        statisticsChartInnerWidget = new StatisticsChartWidget(widget);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(statisticsChartInnerWidget);
    }

    StatisticsChartWidget* chartWidget =
        static_cast<StatisticsChartWidget*>(statisticsChartInnerWidget);
    chartWidget->setData(currentCompany);
}

void MainWindow::refreshAllData() const {
    if (currentCompany != nullptr) {
        MainWindow* self = const_cast<MainWindow*>(this);
        self->validateAndFixProjectAssignments(currentCompany);
    }

    refreshEmployeeTable();
    refreshProjectTable();
    showStatistics();

    if (projectDetailContainer != nullptr &&
        projectDetailContainer->isVisible() && detailedProjectId >= 0) {
        MainWindow* self = const_cast<MainWindow*>(this);
        self->refreshProjectDetailView();
    }
}

void MainWindow::validateAndFixProjectAssignments(Company* company) {
    if (!company) return;

    bool hasWarnings = false;
    QStringList warningMessages;
    auto projects = company->getAllProjects();

    for (const auto& project : projects) {
        if (project.getAllocatedHours() > 0 &&
            !ProjectHelper::hasAssignedEmployees(company, project.getId())) {
            int oldAllocatedHours = project.getAllocatedHours();
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
            this, "Data Validation Warning",
            "Data inconsistency detected:\n\n" + warningMessages.join("\n"));
    }
}

void MainWindow::refreshCompanyList() {
    CompanyManager::refreshCompanyList(companies, companySelector);
}

void MainWindow::addCompany() {
    CompanyManager::addCompany(companies, currentCompany, currentCompanyIndex,
                               companySelector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
    hideProjectDetails();
    refreshProjectTable();
    showStatistics();
    autoSave();
}

void MainWindow::switchCompany() {
    if (companySelector != nullptr) {
        int newIndex = companySelector->currentIndex();
        CompanyManager::switchCompany(companies, currentCompany,
                                      currentCompanyIndex, companySelector,
                                      newIndex);
        DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
        hideProjectDetails();
        refreshProjectTable();
        showStatistics();

        auto employees = currentCompany->getAllEmployees();
        nextEmployeeId =
            IdHelper::calculateNextId(IdHelper::findMaxEmployeeId(employees));

        auto projects = currentCompany->getAllProjects();
        nextProjectId =
            IdHelper::calculateNextId(IdHelper::findMaxProjectId(projects));
    }
}

void MainWindow::deleteCompany() {
    CompanyManager::deleteCompany(companies, currentCompany,
                                  currentCompanyIndex, companySelector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
    hideProjectDetails();
    refreshProjectTable();
    showStatistics();
    autoSave();
}

void MainWindow::addProjectTask() {
    if (!checkCompanyAndHandleError("adding tasks")) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    QDialog dialog(this);
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

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString taskName = taskNameEdit->text().trimmed();
            if (!ValidationHelper::validateNonEmpty(taskName, "Task name",
                                                    &dialog))
                return;

            int taskEst = 0;
            if (!ValidationHelper::validateInt(
                    taskEstHoursEdit->text().trimmed(), taskEst, 1,
                    kMaxEstimatedHours, "Estimated hours", &dialog))
                return;

            int priority = 0;
            if (!ValidationHelper::validateInt(priorityEdit->text().trimmed(),
                                               priority, 0, kMaxPriority,
                                               "Priority", &dialog))
                return;

            if (!TaskDialogHelper::validateAndAddTask(
                    taskName, taskTypeCombo->currentText(), taskEst, priority,
                    projectId, currentCompany, &dialog)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
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
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
}

void MainWindow::assignEmployeeToTask() {
    if (!checkCompanyAndHandleError("assigning employees to tasks")) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    auto tasks = currentCompany->getProjectTasks(projectId);
    if (pendingTaskSelectionId > 0) {
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
    if (tasks.empty()) {
        QMessageBox::warning(
            this, "Error", "No tasks in this project. Please add tasks first.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Assign Employee to Task");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(
        "QDialog { background-color: white; } "
        "QComboBox { background-color: white; color: black; } "
        "QLineEdit { background-color: white; color: black; } "
        "QLabel { color: black; }");

    auto* form = new QFormLayout(&dialog);

    const Project* project = currentCompany->getProject(projectId);
    QString projectPhase = (project != nullptr) ? project->getPhase() : "";

    if (projectPhase == "Completed") {
        QMessageBox::warning(this, "Error",
                             "Cannot assign employees to completed project.");
        return;
    }

    auto* taskCombo = new QComboBox();
    taskCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    TaskAssignmentHelper::setupTaskCombo(taskCombo, tasks,
                                         pendingTaskSelectionId);

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
    form->addRow("Task:", taskCombo);

    auto* employeeCombo = new QComboBox();
    employeeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");

    int matchingCount = 0;
    TaskAssignmentHelper::populateEmployeeCombo(
        employeeCombo, currentCompany, projectId, projectPhase, matchingCount);

    if (employeeCombo->count() == 0) {
        QMessageBox::warning(this, "Error", "No available employees found!");
        return;
    }

    if (matchingCount == 0 && !projectPhase.isEmpty()) {
        QString expectedRole =
            TaskAssignmentHelper::getExpectedRoleForProjectPhase(projectPhase);
        QMessageBox::information(
            this, "Note",
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

    auto* taskInfoLabel =
        new QLabel(QString("Project Phase: %1 | %2")
                       .arg(projectPhase.isEmpty() ? "Unknown" : projectPhase)
                       .arg(expectedRoleText));
    form->addRow(taskInfoLabel);

    TaskAssignmentHelper::setupEmployeeComboUpdate(
        employeeCombo, taskCombo, taskInfoLabel, currentCompany, projectId,
        projectPhase);

    auto* hoursEdit = new QLineEdit();
    hoursEdit->setPlaceholderText("e.g., 20 (hours per week)");
    TaskAssignmentHelper::setupHoursEdit(hoursEdit, taskCombo, employeeCombo,
                                         tasks, currentCompany);
    form->addRow("Hours per week:", hoursEdit);

    auto* okButton = new QPushButton("Assign");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            int taskId = taskCombo->currentData().toInt();
            int employeeId = employeeCombo->currentData().toInt();

            auto employee = currentCompany->getEmployee(employeeId);
            if (!employee) {
                QMessageBox::warning(&dialog, "Error", "Employee not found!");
                return;
            }

            int availableHours = employee->getAvailableHours();
            int maxHours = std::min(kMaxHoursPerWeek, availableHours);

            if (maxHours <= 0) {
                QMessageBox::warning(
                    &dialog, "Error",
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
            if (!ValidationHelper::validateInt(hoursEdit->text().trimmed(),
                                               hours, 1, maxHours,
                                               "Hours per week", &dialog))
                return;

            currentCompany->assignEmployeeToTask(employeeId, projectId, taskId,
                                                 hours);
            refreshAllData();
            autoSave();

            QString taskName;
            QString employeeName;
            for (const auto& task : tasks) {
                if (task.getId() == taskId) {
                    taskName = task.getName();
                    break;
                }
            }
            if (auto emp = currentCompany->getEmployee(employeeId)) {
                employeeName = emp->getName();
            }

            QMessageBox::information(
                &dialog, "Success",
                QString("Employee assigned to task successfully!\n\n"
                        "Task: %1\n"
                        "Employee: %2\n"
                        "Hours per week: %3")
                    .arg(taskName, employeeName, QString::number(hours)));
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog,
                                                     "assign employee to task");
        } catch (const std::exception& e) {
            ExceptionHandler::handleGenericException(e, &dialog);
        }
    });

    dialog.exec();
    pendingTaskSelectionId = -1;
    refreshProjectDetailView();
}

void MainWindow::autoAssignToProject(int projectId) {
    if (!checkCompanyAndHandleError("auto-assigning employees")) return;

    if (projectId < 0) {
        projectId = getSelectedProjectId();
    }
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    auto tasks = currentCompany->getProjectTasks(projectId);
    if (tasks.empty()) {
        QMessageBox::warning(this, "Error",
                             "Project has no tasks. Please add tasks first "
                             "before using auto-assign!");
        return;
    }

    int response = QMessageBox::question(
        this, "Auto Assign",
        QString("Automatically assign available employees to project "
                "'%1'?\n\nEstimated: %2 hours\nAllocated: %3 hours")
            .arg(project->getName())
            .arg(project->getEstimatedHours())
            .arg(project->getAllocatedHours()),
        QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes) {
        try {
            int allocatedBefore = project->getAllocatedHours();

            currentCompany->autoAssignEmployeesToProject(projectId);
            refreshAllData();

            selectProjectRowById(projectId);

            const auto* projectAfter = currentCompany->getProject(projectId);
            int allocatedAfter = projectAfter
                                     ? projectAfter->getAllocatedHours()
                                     : allocatedBefore;
            int hoursAssigned = allocatedAfter - allocatedBefore;

            autoSave();
            QMessageBox::information(
                this, "Success",
                QString("Employees auto-assigned successfully!\n\n"
                        "Hours assigned: %1h\n"
                        "Total allocated: %2h / %3h estimated")
                    .arg(hoursAssigned)
                    .arg(allocatedAfter)
                    .arg(projectAfter ? projectAfter->getEstimatedHours() : 0));
        } catch (const CompanyException& e) {
            QString errorMsg = e.what();
            if (errorMsg.contains("ASSIGNED_HOURS:")) {
                QStringList parts = errorMsg.split(":");
                int trackedHours = parts.size() > 1 ? parts[1].toInt() : 0;
                int actualHours = parts.size() > 2 ? parts[2].toInt() : 0;

                refreshAllData();
                const auto* projectAfter =
                    currentCompany->getProject(projectId);
                int allocatedAfter =
                    projectAfter ? projectAfter->getAllocatedHours() : 0;

                QMessageBox::information(
                    this, "Success",
                    QString("Employees auto-assigned successfully!\n\n"
                            "Hours assigned (tracked): %1h\n"
                            "Hours assigned (actual): %2h\n"
                            "Total allocated: %3h / %4h estimated")
                        .arg(trackedHours)
                        .arg(actualHours)
                        .arg(allocatedAfter)
                        .arg(projectAfter ? projectAfter->getEstimatedHours()
                                          : 0));
            } else {
                QMessageBox::warning(
                    this, "Error",
                    QString("Failed to auto-assign: ") + e.what());
            }
        }
    }
}

void MainWindow::openProjectDetails() {
    if (!checkCompanyAndHandleError("viewing project details")) return;
    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::information(this, "Project Details",
                                 "Please select a project to view details.");
        return;
    }
    showProjectDetails(projectId);
}

void MainWindow::closeProjectDetails() {
    hideProjectDetails();
    refreshProjectTable();
}

void MainWindow::autoAssignDetailedProject() {
    if (!checkCompanyAndHandleError("auto-assigning employees")) return;
    if (detailedProjectId < 0) {
        QMessageBox::information(this, "Auto Assign",
                                 "Open a project to manage its tasks first.");
        return;
    }

    autoAssignToProject(detailedProjectId);

    if (projectDetailContainer != nullptr &&
        projectDetailContainer->isVisible()) {
        refreshProjectDetailView();
    }
}

void MainWindow::assignTaskFromDetails(int projectId, int taskId) {
    if (!checkCompanyAndHandleError("assigning tasks")) return;

    if (projectId <= 0 || taskId <= 0) {
        QObject* senderObject = sender();
        auto* assignButton = qobject_cast<QPushButton*>(senderObject);
        if (assignButton != nullptr) {
            projectId = assignButton->property("projectId").toInt();
            taskId = assignButton->property("taskId").toInt();
        }
    }

    if (projectId <= 0 || taskId <= 0) return;

    detailedProjectId = projectId;
    selectProjectRowById(projectId);
    pendingTaskSelectionId = taskId;
    assignEmployeeToTask();
    pendingTaskSelectionId = -1;
}

void MainWindow::showProjectDetails(int projectId) {
    if (!checkCompanyAndHandleError("viewing project details")) return;
    const Project* project = currentCompany->getProject(projectId);
    if (project == nullptr) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany,
                                                           projectId);
    project = currentCompany->getProject(projectId);

    detailedProjectId = projectId;
    pendingTaskSelectionId = -1;
    if (companyWidget != nullptr) {
        companyWidget->setVisible(false);
        companyWidget->setMaximumHeight(0);
    }
    if (mainTabWidget != nullptr && mainTabWidget->tabBar() != nullptr) {
        mainTabWidget->tabBar()->setVisible(false);
    }
    if (projectDetailTitle != nullptr) {
        projectDetailTitle->setText(
            QString("Current Project — %1").arg(project->getName()));
    }
    if (projectDetailInfoText != nullptr) {
        projectDetailInfoText->setVisible(true);
        projectDetailInfoText->setHtml(
            HtmlGenerator::generateProjectDetailHtml(*project, currentCompany));
    }
    if (projectListContainer != nullptr) {
        projectListContainer->setVisible(false);
    }
    if (projectDetailContainer != nullptr) {
        projectDetailContainer->setVisible(true);
    }

    populateProjectTasksTable(*project);
    selectProjectRowById(projectId);

    if (projectDetailContainer != nullptr) {
        projectDetailContainer->update();
        projectDetailContainer->repaint();
    }
    if (mainTabWidget != nullptr) {
        mainTabWidget->update();
        mainTabWidget->repaint();
    }
    update();
    repaint();
}

void MainWindow::hideProjectDetails() {
    detailedProjectId = -1;
    pendingTaskSelectionId = -1;

    if (companyWidget != nullptr) {
        companyWidget->setVisible(true);
        companyWidget->setMaximumHeight(QWIDGETSIZE_MAX);
    }
    if (mainTabWidget != nullptr && mainTabWidget->tabBar() != nullptr) {
        mainTabWidget->tabBar()->setVisible(true);
    }
    if (projectListContainer != nullptr) {
        projectListContainer->setVisible(true);
    }
    if (projectDetailContainer != nullptr) {
        projectDetailContainer->setVisible(false);
    }
    if (projectDetailTitle != nullptr) {
        projectDetailTitle->setText("Current Project");
    }
    if (projectDetailInfoText != nullptr) {
        projectDetailInfoText->clear();
        projectDetailInfoText->setVisible(false);
    }
    if (projectTasksTable != nullptr) {
        projectTasksTable->clearContents();
        projectTasksTable->setRowCount(0);
    }
}

void MainWindow::refreshProjectDetailView() {
    if (projectDetailContainer == nullptr ||
        !projectDetailContainer->isVisible()) {
        return;
    }
    if (currentCompany == nullptr || detailedProjectId < 0) {
        hideProjectDetails();
        return;
    }

    const Project* project = currentCompany->getProject(detailedProjectId);
    if (project == nullptr) {
        hideProjectDetails();
        return;
    }

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany,
                                                           detailedProjectId);
    project = currentCompany->getProject(detailedProjectId);

    if (projectDetailTitle != nullptr) {
        projectDetailTitle->setText(
            QString("Current Project — %1").arg(project->getName()));
    }
    if (projectDetailInfoText != nullptr) {
        projectDetailInfoText->setVisible(true);
        projectDetailInfoText->setHtml(
            HtmlGenerator::generateProjectDetailHtml(*project, currentCompany));
    }

    populateProjectTasksTable(*project);
    selectProjectRowById(detailedProjectId);
}

void MainWindow::populateProjectTasksTable(const Project& project) {
    if (projectTasksTable == nullptr || currentCompany == nullptr) return;
    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany,
                                                           project.getId());
    const Project* updatedProject = currentCompany->getProject(project.getId());
    if (updatedProject) {
        ProjectHelper::populateProjectTasksTable(projectTasksTable,
                                                 *updatedProject, this);
    }
}

void MainWindow::selectProjectRowById(int projectId) {
    if (projectTable == nullptr) return;

    int rowCount = projectTable->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        QTableWidgetItem* item = projectTable->item(row, 0);
        if (item == nullptr) {
            continue;
        }
        bool ok = false;
        int idValue = item->text().toInt(&ok);
        if (ok && idValue == projectId) {
            projectTable->setCurrentCell(row, 0);
            break;
        }
    }
}

void MainWindow::viewProjectAssignments() {
    if (!checkCompanyAndHandleError("viewing project assignments")) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    QDialog dialog(this);
    DialogHelper::createHtmlDialog(
        &dialog, "Project Team: " + project->getName(),
        HtmlGenerator::generateProjectAssignmentsHtml(*project,
                                                      currentCompany));
    dialog.exec();
}

void MainWindow::viewEmployeeHistory() {
    if (!checkCompanyAndHandleError("viewing employee history")) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error", "Please select an employee first.");
        return;
    }

    auto employee = currentCompany->getEmployee(employeeId);
    if (!employee) {
        QMessageBox::warning(this, "Error", "Employee not found!");
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

    auto allProjects = currentCompany->getAllProjects();

    std::vector<const Project*> employeeProjects;
    for (const auto& proj : allProjects) {
        if (allProjectIds.find(proj.getId()) != allProjectIds.end()) {
            employeeProjects.push_back(&proj);
        }
    }

    QDialog dialog(this);
    DialogHelper::createHtmlDialog(
        &dialog, "Employee History: " + employee->getName(),
        HtmlGenerator::generateEmployeeHistoryHtml(*employee, currentCompany,
                                                   employeeProjects));
    dialog.exec();
}
