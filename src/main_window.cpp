#include "main_window.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFrame>
#include <QFile>
#include <QFormLayout>
#include <QHeaderView>
#include <QSizePolicy>
#include <QStringList>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QFont>
#include <QRect>
#include <QTimer>
#include <algorithm>
#include <fstream>
#include <map>
#include <ranges>

#include "derived_employees.h"
#include "display_helper.h"
#include "employee_validator.h"
#include "exception_handler.h"
#include "auto_save_loader.h"
#include "action_button_helper.h"
#include "dialog_helper.h"
#include "employee_dialog_handler.h"
#include "file_helper.h"
#include "html_generator.h"
#include "project_dialog_helper.h"
#include "project_helper.h"
#include "statistics_chart_widget.h"
#include "task_assignment_helper.h"
#include "task_dialog_helper.h"
#include "main_window_ui_builder.h"
#include "app_styles.h"
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
        if (auto employees = currentCompany->getAllEmployees();
            !employees.empty()) {
            nextEmployeeId = employees.back()->getId() + 1;
        }
        if (auto projects = currentCompany->getAllProjects();
            !projects.empty()) {
            nextProjectId = projects.back().getId() + 1;
        }
    }
}

MainWindow::~MainWindow() {
    autoSave();
    for (auto* company : companies) {
        delete company;
    }
}

void MainWindow::setupUI() {
    MainWindowUIBuilder::setupMainUI(this);
}

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

    if (projectDetailContainer != nullptr && projectDetailContainer->isVisible() &&
        detailedProjectId >= 0) {
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

    auto* form = new QFormLayout(&dialog);
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

    EmployeeDialogHelper::createEmployeeDialog(dialog, form, typeCombo, nameEdit,
                                               salaryEdit, deptEdit, employmentRateCombo,
                                               managerProject, devLanguage, devExperience,
                                               designerTool, designerProjects, qaTestType,
                                               qaBugs, managerProjectLabel, devLanguageLabel,
                                               devExperienceLabel, designerToolLabel,
                                               designerProjectsLabel, qaTestTypeLabel, qaBugsLabel);

    auto projects = currentCompany->getAllProjects();
    managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        managerProject->addItem(proj.getName(), proj.getId());
    }

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            if (!EmployeeDialogHandler::processAddEmployee(&dialog, currentCompany, nextEmployeeId,
                                                           nameEdit, salaryEdit, deptEdit, typeCombo,
                                                           employmentRateCombo, managerProject, devLanguage,
                                                           devExperience, designerTool, designerProjects,
                                                           qaTestType, qaBugs)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(&dialog, "Success",
                                     "Employee added successfully!\n\n"
                                     "Name: " + nameEdit->text().trimmed() +
                                     "\nType: " + typeCombo->currentText() +
                                     "\nSalary: $" + salaryEdit->text().trimmed());
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog, "add employee");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog, "employee");
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

    auto* form = new QFormLayout(&dialog);
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

    EmployeeDialogHelper::createEditEmployeeDialog(dialog, form, employee,
                                                   nameEdit, salaryEdit, deptEdit,
                                                   employmentRateCombo, managerProject,
                                                   devLanguage, devExperience,
                                                   designerTool, designerProjects,
                                                   qaTestType, qaBugs,
                                                   managerProjectLabel, devLanguageLabel,
                                                   devExperienceLabel, designerToolLabel,
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

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            if (!EmployeeDialogHandler::processEditEmployee(&dialog, currentCompany, employeeId, nextEmployeeId,
                                                            nameEdit, salaryEdit, deptEdit,
                                                            employmentRateCombo, managerProject, devLanguage,
                                                            devExperience, designerTool, designerProjects,
                                                            qaTestType, qaBugs, currentType)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(&dialog, "Success",
                                     "Employee updated successfully!\n\n"
                                     "Name: " + nameEdit->text().trimmed() +
                                     "\nType: " + currentType +
                                     "\nSalary: $" + salaryEdit->text().trimmed());
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog, "edit employee");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog, "employee update");
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
            if (auto employees = currentCompany->getAllEmployees();
                !employees.empty()) {
                nextEmployeeId = employees.back()->getId() + 1;
            }
            refreshAllData();
            autoSave();
            QMessageBox::information(this, "Success",
                                     "Employee deleted successfully!");

        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to auto-save changes: ") +
                                     e.what());
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

    // Проверяем, есть ли у работника активные назначения
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
        
        // Сбрасываем часы работника перед увольнением
        // Нужно убрать работника из всех проектов и сбросить его часы
        const std::vector<int>& assignedProjects = employee->getAssignedProjects();
        
        // Вычисляем пропорцию часов для каждой задачи и убираем их
        for (int projectId : assignedProjects) {
            const Project* project = currentCompany->getProject(projectId);
            if (project) {
                auto tasks = currentCompany->getProjectTasks(projectId);
                int totalAllocatedHours = 0;
                for (const auto& task : tasks) {
                    totalAllocatedHours += task.getAllocatedHours();
                }
                
                if (totalAllocatedHours > 0) {
                    int employeeHours = employee->getCurrentWeeklyHours();
                    // Получаем не-const проект для изменения задач
                    Project* mutableProject = const_cast<Project*>(currentCompany->getProject(projectId));
                    if (mutableProject) {
                        double employeeHourlyRate = employee->getSalary() / 160.0; // Часов в месяц
                        double totalCostToRemove = 0.0;
                        
                        for (auto& task : mutableProject->getTasks()) {
                            if (task.getAllocatedHours() > 0) {
                                // Вычисляем пропорцию часов работника для этой задачи
                                int taskHours = (task.getAllocatedHours() * employeeHours) / totalAllocatedHours;
                                if (taskHours > 0 && task.getAllocatedHours() >= taskHours) {
                                    task.setAllocatedHours(task.getAllocatedHours() - taskHours);
                                    // Вычисляем стоимость для удаления
                                    double taskCost = employeeHourlyRate * taskHours;
                                    totalCostToRemove += taskCost;
                                }
                            }
                        }
                        
                        // Убираем стоимость работника из проекта
                        if (totalCostToRemove > 0) {
                            mutableProject->removeEmployeeCost(totalCostToRemove);
                        }
                        
                        mutableProject->recomputeTotalsFromTasks();
                    }
                }
            }
        }
        
        // Сбрасываем все часы работника
        int currentHours = employee->getCurrentWeeklyHours();
        employee->removeWeeklyHours(currentHours);
        
        // Удаляем работника из всех проектов
        for (int projectId : assignedProjects) {
            employee->removeAssignedProject(projectId);
        }
    }

    int userChoice =
        QMessageBox::question(this, "Confirm Fire",
                              QString("Are you sure you want to fire %1?")
                                  .arg(employee->getName()),
                              QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            // Увольняем работника (устанавливаем isActive = false)
            employee->setIsActive(false);
            refreshAllData();
            autoSave();
            QMessageBox::information(this, "Success",
                                     QString("Employee %1 has been fired successfully!")
                                         .arg(employee->getName()));

        } catch (const EmployeeException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to fire employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to auto-save changes: ") +
                                     e.what());
        }
    }
}

void MainWindow::searchEmployee() {
    if (!checkCompanyAndHandleError("searching employees")) return;

    QString searchTerm = employeeSearchEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        DisplayHelper::displayEmployees(employeeTable, currentCompany, this);
        return;
    }

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(0);
    int rowIndex = 0;

    for (const auto& employee : employees) {
        if (employee->getName().toLower().contains(searchTerm) ||
            employee->getDepartment().toLower().contains(searchTerm) ||
            employee->getPosition().toLower().contains(searchTerm)) {
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
            employeeTable->setItem(rowIndex, 4, new QTableWidgetItem(
                                       DisplayHelper::formatEmploymentRate(employee->getEmploymentRate())));
            employeeTable->setItem(
                rowIndex, 5, new QTableWidgetItem(employee->getEmployeeType()));

            QString projectInfo =
                DisplayHelper::formatProjectInfo(employee, currentCompany);
            employeeTable->setItem(rowIndex, 6,
                                   new QTableWidgetItem(projectInfo));
            
            employeeTable->setCellWidget(rowIndex, 7, createEmployeeActionButtons(rowIndex));
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
            if (!ValidationHelper::validateNonEmpty(projectName, "Project name", &dialog)) return;

            double projectBudget = 0.0;
            if (!ValidationHelper::validateDouble(fields.budgetEdit->text().trimmed(), projectBudget, 0.0, kMaxBudget, "Budget", &dialog)) return;

            QString selectedStatus = fields.statusCombo->currentText();
            QString clientName = fields.clientNameEdit->text().trimmed();

            if (!ValidationHelper::validateNonEmpty(clientName, "Client name", &dialog)) return;

            if (!ValidationHelper::validateDateRange(fields.startDate->date(), fields.endDate->date(), &dialog)) return;

            int estimatedHours = 0;
            if (!ValidationHelper::validateInt(fields.estimatedHoursEdit->text().trimmed(), estimatedHours, 0, kMaxEstimatedHours,
                            "Estimated hours", &dialog)) return;

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
                            fields.descEdit->toPlainText().trimmed(), selectedStatus,
                            fields.startDate->date(), fields.endDate->date(), projectBudget,
                            clientName, estimatedHours);

            currentCompany->addProject(project);
            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
                "Project added successfully!\n\n"
                "Name: " +
                    projectName +
                    "\n"
                    "Status: " +
                    selectedStatus +
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
            if (!ValidationHelper::validateNonEmpty(projectName, "Project name", &dialog)) return;

            double projectBudget = 0.0;
            if (!ValidationHelper::validateDouble(fields.budgetEdit->text().trimmed(), projectBudget, 0.0, kMaxBudget, "Budget", &dialog)) return;

            QString selectedStatus = fields.statusCombo->currentText();
            QString clientName = fields.clientNameEdit->text().trimmed();

            if (!ValidationHelper::validateNonEmpty(clientName, "Client name", &dialog)) return;

            if (!ValidationHelper::validateDateRange(fields.startDate->date(), fields.endDate->date(), &dialog)) return;

            int estimatedHours = 0;
            if (!ValidationHelper::validateInt(fields.estimatedHoursEdit->text().trimmed(), estimatedHours, 0, kMaxEstimatedHours,
                            "Estimated hours", &dialog)) return;

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
            std::vector<Task> savedTasks;
            double savedEmployeeCosts = 0.0;
            if (oldProject) {
                savedTasks = oldProject->getTasks();
                savedEmployeeCosts = oldProject->getEmployeeCosts();
            }

            Project updatedProject(projectId, fields.nameEdit->text().trimmed(),
                                   fields.descEdit->toPlainText().trimmed(),
                                   fields.statusCombo->currentText(),
                                   fields.startDate->date(), fields.endDate->date(),
                                   projectBudget, clientName, estimatedHours);

            for (const auto& task : savedTasks) {
                updatedProject.addTask(task);
            }

            if (savedEmployeeCosts > 0.0) {
                updatedProject.addEmployeeCost(savedEmployeeCosts);
            }

            currentCompany->removeProject(projectId);
            currentCompany->addProject(updatedProject);

            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
            refreshAllData();
            autoSave();
            QMessageBox::information(
                &dialog, "Success",
                "Project updated successfully!\n\n"
                "Name: " +
                    projectName +
                    "\n"
                    "Status: " +
                    selectedStatus +
                    "\n"
                    "Budget: $" +
                    QString::number(projectBudget, 'f', 2));
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog, "edit project");
        } catch (const FileManagerException& e) {
            ExceptionHandler::handleFileManagerException(e, &dialog, "project update");
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
            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
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
            QMessageBox::warning(this, "Error",
                                 QString("Failed to auto-save changes: ") +
                                     e.what());
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
    AutoSaveLoader::autoLoad(companies, currentCompany, currentCompanyIndex, this);
    
    if (companySelector) {
                        refreshCompanyList();
                        if (currentCompanyIndex >= 0 && currentCompanyIndex < companySelector->count()) {
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
                          "Use the company selector at the top to create or select a company.")
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


// Table setup helper function
void MainWindow::setupTableWidget(QTableWidget* table, const QStringList& headers,
                                 const QList<int>& columnWidths, bool stretchLast) {
    if (!table) return;
    
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setVisible(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(stretchLast);
    table->setAlternatingRowColors(true);
    table->setStyleSheet(
        "QTableWidget::item:selected { background-color: #c8c8c8; color: black; }"
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
    return ActionButtonHelper::createEmployeeActionButtons(employeeTable, rowIndex, this);
}

QWidget* MainWindow::createProjectActionButtons(int rowIndex) {
    return ActionButtonHelper::createProjectActionButtons(projectTable, rowIndex, this);
}

void MainWindow::clearAllDataFiles() {
    FileHelper::clearAllDataFiles(this);
}

void MainWindow::refreshEmployeeTable() const {
    MainWindow* self = const_cast<MainWindow*>(this);
    DisplayHelper::displayEmployees(employeeTable, currentCompany, self);
}

void MainWindow::refreshProjectTable() const {
    MainWindow* self = const_cast<MainWindow*>(this);
    DisplayHelper::displayProjects(projectTable, currentCompany, self);
    if (projectDetailContainer != nullptr && projectDetailContainer->isVisible()) {
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
    
    // Создаем виджет один раз
    if (statisticsChartInnerWidget == nullptr) {
        statisticsChartInnerWidget = new StatisticsChartWidget(widget);
        QVBoxLayout* layout = new QVBoxLayout(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(statisticsChartInnerWidget);
    }
    
    // Обновляем данные и запускаем анимацию
    StatisticsChartWidget* chartWidget = static_cast<StatisticsChartWidget*>(statisticsChartInnerWidget);
    chartWidget->setData(currentCompany);
}

void MainWindow::refreshAllData() const {
    // Проверяем и исправляем несоответствия в назначениях перед обновлением
    if (currentCompany != nullptr) {
        MainWindow* self = const_cast<MainWindow*>(this);
        self->validateAndFixProjectAssignments(currentCompany);
    }
    
    refreshEmployeeTable();
    refreshProjectTable();
    showStatistics();
    
    // Обновляем детали проекта, если они открыты
    if (projectDetailContainer != nullptr && projectDetailContainer->isVisible() && detailedProjectId >= 0) {
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
        if (project.getAllocatedHours() > 0 && !ProjectHelper::hasAssignedEmployees(company, project.getId())) {
            int oldAllocatedHours = project.getAllocatedHours();
            ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(company, project.getId());
            hasWarnings = true;
            warningMessages.append(QString("Project '%1': Found allocated hours (%2h) but no assigned employees. Hours have been cleared.")
                                   .arg(project.getName(), QString::number(oldAllocatedHours)));
        }
    }
    
    if (hasWarnings) {
        QMessageBox::warning(this, "Data Validation Warning",
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

        if (auto employees = currentCompany->getAllEmployees();
            !employees.empty()) {
            nextEmployeeId = employees.back()->getId() + 1;
        }
        if (auto projects = currentCompany->getAllProjects();
            !projects.empty()) {
            nextProjectId = projects.back().getId() + 1;
        }
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

    TaskDialogHelper::createAddTaskDialog(&dialog, form, taskNameEdit, taskTypeCombo,
                                         taskEstHoursEdit, priorityEdit);

    auto* okButton = new QPushButton("Add Task");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString taskName = taskNameEdit->text().trimmed();
            if (!ValidationHelper::validateNonEmpty(taskName, "Task name", &dialog)) return;

            int taskEst = 0;
            if (!ValidationHelper::validateInt(taskEstHoursEdit->text().trimmed(), taskEst, 1, kMaxEstimatedHours,
                                             "Estimated hours", &dialog)) return;

            int priority = 0;
            if (!ValidationHelper::validateInt(priorityEdit->text().trimmed(), priority, 0, kMaxPriority,
                                             "Priority", &dialog)) return;

            if (!TaskDialogHelper::validateAndAddTask(taskName, taskTypeCombo->currentText(),
                                                     taskEst, priority, projectId, currentCompany, &dialog)) {
                return;
            }

            refreshAllData();
            autoSave();
            QMessageBox::information(&dialog, "Success",
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
    QString projectStatus = (project != nullptr) ? project->getStatus() : "";

    if (projectStatus == "Completed") {
        QMessageBox::warning(this, "Error", "Cannot assign employees to completed project.");
        return;
    }

    auto* taskCombo = new QComboBox();
    taskCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    TaskAssignmentHelper::setupTaskCombo(taskCombo, tasks, pendingTaskSelectionId);

    if (pendingTaskSelectionId > 0 && taskCombo->count() == 1) {
            taskCombo->setEnabled(false);
        taskCombo->setStyleSheet(taskCombo->styleSheet() +
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
    TaskAssignmentHelper::populateEmployeeCombo(employeeCombo, currentCompany, projectId, projectStatus, matchingCount);

    if (employeeCombo->count() == 0) {
        QMessageBox::warning(this, "Error", "No available employees found!");
        return;
    }

    if (matchingCount == 0 && !projectStatus.isEmpty()) {
        QString expectedRole = TaskAssignmentHelper::getExpectedRoleForProjectStatus(projectStatus);
        QMessageBox::information(this, "Note",
            QString("No %1 employees available for %2 phase.")
                                    .arg(expectedRole, projectStatus));
    }

    form->addRow("Employee:", employeeCombo);

    QString expectedRoleText = TaskAssignmentHelper::getExpectedRoleForProjectStatus(projectStatus);
    if (expectedRoleText.isEmpty()) {
        expectedRoleText = "Unknown phase";
    } else if (expectedRoleText == "any role") {
        expectedRoleText = "Any role allowed";
    } else {
        expectedRoleText += " role required";
    }

    auto* taskInfoLabel = new QLabel(QString("Project Phase: %1 | %2")
                       .arg(projectStatus.isEmpty() ? "Unknown" : projectStatus)
                       .arg(expectedRoleText));
    form->addRow(taskInfoLabel);

    TaskAssignmentHelper::setupEmployeeComboUpdate(employeeCombo, taskCombo, taskInfoLabel,
                                                   currentCompany, projectId, projectStatus);

    auto* hoursEdit = new QLineEdit();
    hoursEdit->setPlaceholderText("e.g., 20 (hours per week)");
    TaskAssignmentHelper::setupHoursEdit(hoursEdit, taskCombo, tasks);
    form->addRow("Hours per week:", hoursEdit);

    auto* okButton = new QPushButton("Assign");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            int taskId = taskCombo->currentData().toInt();
            int employeeId = employeeCombo->currentData().toInt();
            int hours = 0;
            if (!ValidationHelper::validateInt(hoursEdit->text().trimmed(), hours, 1, kMaxHoursPerWeek,
                                             "Hours per week", &dialog)) return;

            currentCompany->assignEmployeeToTask(employeeId, projectId, taskId, hours);
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

            QMessageBox::information(&dialog, "Success",
                                    QString("Employee assigned to task successfully!\n\n"
                                           "Task: %1\n"
                                           "Employee: %2\n"
                                           "Hours per week: %3")
                                        .arg(taskName, employeeName, QString::number(hours)));
            dialog.accept();
        } catch (const CompanyException& e) {
            ExceptionHandler::handleCompanyException(e, &dialog, "assign employee to task");
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
            currentCompany->autoAssignEmployeesToProject(projectId);
            refreshAllData();
            
            // Восстанавливаем выделение строки после обновления таблицы
            selectProjectRowById(projectId);
            
            autoSave();
            QMessageBox::information(this, "Success",
                                     "Employees auto-assigned successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to auto-assign: ") + e.what());
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
    
    // Используем detailedProjectId напрямую, чтобы избежать проблем с выделением строки
    autoAssignToProject(detailedProjectId);
    
    // Обновляем детальный вид после автоназначения
    if (projectDetailContainer != nullptr && projectDetailContainer->isVisible()) {
        refreshProjectDetailView();
    }
}

void MainWindow::assignTaskFromDetails(int projectId, int taskId) {
    if (!checkCompanyAndHandleError("assigning tasks")) return;
    
    // Если параметры не переданы, пытаемся получить их из sender()
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

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany, projectId);
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
        projectDetailInfoText->setHtml(HtmlGenerator::generateProjectDetailHtml(*project, currentCompany));
    }
    if (projectListContainer != nullptr) {
        projectListContainer->setVisible(false);
    }
    if (projectDetailContainer != nullptr) {
        projectDetailContainer->setVisible(true);
    }

    populateProjectTasksTable(*project);
    selectProjectRowById(projectId);
    
    // Force redraw to ensure proper screen update
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
    if (projectDetailContainer == nullptr || !projectDetailContainer->isVisible()) {
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

    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany, detailedProjectId);
            project = currentCompany->getProject(detailedProjectId);

    if (projectDetailTitle != nullptr) {
        projectDetailTitle->setText(
            QString("Current Project — %1").arg(project->getName()));
    }
    if (projectDetailInfoText != nullptr) {
        projectDetailInfoText->setVisible(true);
        projectDetailInfoText->setHtml(HtmlGenerator::generateProjectDetailHtml(*project, currentCompany));
    }

    populateProjectTasksTable(*project);
    selectProjectRowById(detailedProjectId);
}

void MainWindow::populateProjectTasksTable(const Project& project) {
    if (projectTasksTable == nullptr || currentCompany == nullptr) return;
    ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(currentCompany, project.getId());
    const Project* updatedProject = currentCompany->getProject(project.getId());
    if (updatedProject) {
        ProjectHelper::populateProjectTasksTable(projectTasksTable, *updatedProject, this);
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
    DialogHelper::createHtmlDialog(&dialog, "Project Team: " + project->getName(),
                                   HtmlGenerator::generateProjectAssignmentsHtml(*project, currentCompany));
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
    auto allProjects = currentCompany->getAllProjects();

    std::vector<const Project*> employeeProjects;
    for (const auto& proj : allProjects) {
        if (std::find(assignedProjectIds.begin(), assignedProjectIds.end(),
                      proj.getId()) != assignedProjectIds.end()) {
            employeeProjects.push_back(&proj);
        }
    }

    QDialog dialog(this);
    DialogHelper::createHtmlDialog(&dialog, "Employee History: " + employee->getName(),
                                   HtmlGenerator::generateEmployeeHistoryHtml(*employee, currentCompany, employeeProjects));
    dialog.exec();
}
