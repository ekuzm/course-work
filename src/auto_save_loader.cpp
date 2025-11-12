#include "auto_save_loader.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>

#include "file_manager.h"
#include "main_window.h"

QString AutoSaveLoader::getDataDirectory() {
    QDir buildDir = QDir::current();
    if (buildDir.dirName() != "build") {
        QDir projectRoot = QDir::current();
        if (projectRoot.cd("build")) {
            buildDir = projectRoot;
        } else {
            projectRoot.cdUp();
            if (projectRoot.cd("build")) {
                buildDir = projectRoot;
            }
        }
    }

    QString dataDirPath = buildDir.absoluteFilePath("data");
    return dataDirPath;
}

void AutoSaveLoader::clearDataFiles(const QString& dataDirPath) {
    QDir companiesDir(dataDirPath + "/companies");
    QDir employeesDir(dataDirPath + "/employees");
    QDir projectsDir(dataDirPath + "/projects");

    const QStringList filters{"*.txt"};

    companiesDir.setNameFilters(filters);
    companiesDir.setFilter(QDir::Files);
    for (const QString& fileName : companiesDir.entryList()) {
        companiesDir.remove(fileName);
    }

    employeesDir.setNameFilters(filters);
    employeesDir.setFilter(QDir::Files);
    for (const QString& fileName : employeesDir.entryList()) {
        employeesDir.remove(fileName);
    }

    projectsDir.setNameFilters(filters);
    projectsDir.setFilter(QDir::Files);
    for (const QString& fileName : projectsDir.entryList()) {
        projectsDir.remove(fileName);
    }
}

void AutoSaveLoader::autoSave(const std::vector<Company*>& companies,
                              MainWindow* mainWindow) {
    try {
        QString dataDirPath = getDataDirectory();
        QDir dataDir(dataDirPath);
        if (!dataDir.exists()) {
            dataDir.mkpath(".");
        }

        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (!companiesDir.exists()) companiesDir.mkpath(".");
        if (!employeesDir.exists()) employeesDir.mkpath(".");
        if (!projectsDir.exists()) projectsDir.mkpath(".");

        bool hasValidCompanies = false;
        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] != nullptr) {
                hasValidCompanies = true;
                break;
            }
        }

        if (hasValidCompanies) {
            clearDataFiles(dataDirPath);
        }

        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] == nullptr) continue;

            QString index = QString::number(i + 1);
            QString companyFileName = QString("company_%1.txt").arg(index);
            QString companyFilePath =
                companiesDir.absoluteFilePath(companyFileName);
            try {
                FileManager::saveCompany(*companies[i], companyFilePath);
            } catch (const FileManagerException& e) {
                if (mainWindow) {
                    QMessageBox::warning(
                        mainWindow, "Auto-save Error",
                        QString("Failed to save company data: %1")
                            .arg(e.what()));
                }
                continue;
            }

            QString employeesFileName = QString("employees_%1.txt").arg(index);
            QString employeesFilePath =
                employeesDir.absoluteFilePath(employeesFileName);
            try {
                FileManager::saveEmployees(*companies[i], employeesFilePath);
            } catch (const FileManagerException& e) {
                if (mainWindow) {
                    QMessageBox::warning(
                        mainWindow, "Auto-save Error",
                        QString("Failed to save employees: %1").arg(e.what()));
                }
            }

            QString projectsFileName = QString("projects_%1.txt").arg(index);
            QString projectsFilePath =
                projectsDir.absoluteFilePath(projectsFileName);
            try {
                FileManager::saveProjects(*companies[i], projectsFilePath);
            } catch (const FileManagerException& e) {
                if (mainWindow) {
                    QMessageBox::warning(
                        mainWindow, "Auto-save Error",
                        QString("Failed to save projects: %1").arg(e.what()));
                }
            }

            QString tasksFileName = QString("tasks_%1.txt").arg(index);
            QString tasksFilePath = projectsDir.absoluteFilePath(tasksFileName);
            try {
                FileManager::saveTasks(*companies[i], tasksFilePath);
            } catch (const FileManagerException& e) {
                if (mainWindow) {
                    QMessageBox::warning(
                        mainWindow, "Auto-save Error",
                        QString("Failed to save tasks: %1").arg(e.what()));
                }
            }
        }
    } catch (const FileManagerException& e) {
        if (mainWindow) {
            QMessageBox::warning(
                mainWindow, "Auto-save Error",
                QString("Failed to auto-save: %1").arg(e.what()));
        }
    }
}

void AutoSaveLoader::autoLoad(std::vector<Company*>& companies,
                              Company*& currentCompany,
                              int& currentCompanyIndex,
                              MainWindow* mainWindow) {
    try {
        QString dataDirPath = getDataDirectory();
        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (!companiesDir.exists()) return;

        companiesDir.setNameFilters(QStringList() << "company_*.txt");
        companiesDir.setFilter(QDir::Files);
        QStringList companyFiles =
            companiesDir.entryList(QDir::Files, QDir::Name);

        if (companyFiles.isEmpty()) return;

        std::vector<Company*> loadedCompanies;

        for (const QString& fileName : companyFiles) {
            QString index = fileName;
            index.replace("company_", "").replace(".txt", "");
            bool conversionOk = false;
            int companyIndex = index.toInt(&conversionOk);
            if (!conversionOk) continue;

            try {
                QString companyFilePath =
                    companiesDir.absoluteFilePath(fileName);
                Company company = FileManager::loadCompany(companyFilePath);

                QString employeesFileName =
                    QString("employees_%1.txt").arg(index);
                QString employeesFilePath =
                    employeesDir.absoluteFilePath(employeesFileName);
                if (QFile::exists(employeesFilePath)) {
                    FileManager::loadEmployees(company, employeesFilePath);
                }

                QString projectsFileName =
                    QString("projects_%1.txt").arg(index);
                QString projectsFilePath =
                    projectsDir.absoluteFilePath(projectsFileName);
                if (QFile::exists(projectsFilePath)) {
                    FileManager::loadProjects(company, projectsFilePath);
                }

                QString tasksFileName = QString("tasks_%1.txt").arg(index);
                QString tasksFilePath =
                    projectsDir.absoluteFilePath(tasksFileName);
                if (QFile::exists(tasksFilePath)) {
                    FileManager::loadTasks(company, tasksFilePath);
                }

                QString taskAssignmentsFileName =
                    QString("task_assignments_%1.txt").arg(index);
                QString taskAssignmentsFilePath =
                    projectsDir.absoluteFilePath(taskAssignmentsFileName);
                if (QFile::exists(taskAssignmentsFilePath)) {
                    FileManager::loadTaskAssignments(company,
                                                     taskAssignmentsFilePath);
                }

                company.recalculateEmployeeHours();

                auto employees = company.getAllEmployees();
                for (const auto& emp : employees) {
                    if (emp) {
                        auto it = FileManager::employeeStatusesFromFile.find(
                            emp->getId());
                        if (it != FileManager::employeeStatusesFromFile.end()) {
                            bool shouldBeActive = it->second;

                            if (!shouldBeActive) {
                                auto assignedProjects =
                                    emp->getAssignedProjects();

                                auto allProjects = company.getAllProjects();
                                for (const auto& project : allProjects) {
                                    int projectId = project.getId();
                                    auto tasks =
                                        company.getProjectTasks(projectId);
                                    bool hasAssignments = false;
                                    for (const auto& task : tasks) {
                                        if (company.getEmployeeTaskHours(
                                                emp->getId(), projectId,
                                                task.getId()) > 0) {
                                            hasAssignments = true;
                                            break;
                                        }
                                    }
                                    if (hasAssignments) {
                                        emp->addToProjectHistory(projectId);
                                    }
                                }

                                for (int projectId : assignedProjects) {
                                    emp->addToProjectHistory(projectId);
                                }
                            }

                            if (!shouldBeActive &&
                                emp->getCurrentWeeklyHours() > 0) {
                                try {
                                    int currentHours =
                                        emp->getCurrentWeeklyHours();
                                    emp->removeWeeklyHours(currentHours);
                                } catch (const EmployeeException&) {
                                    continue;
                                }
                            }
                            try {
                                emp->setIsActive(shouldBeActive);
                            } catch (const EmployeeException&) {
                            }
                        }
                    }
                }
                FileManager::employeeStatusesFromFile.clear();

                if (mainWindow) {
                    mainWindow->validateAndFixProjectAssignments(&company);
                }

                Company* companyPtr = new Company(std::move(company));
                loadedCompanies.push_back(companyPtr);
            } catch (const FileManagerException& e) {
                continue;
            }
        }

        companies = loadedCompanies;
        if (!companies.empty()) {
            currentCompany = companies[0];
            currentCompanyIndex = 0;
        }
    } catch (const FileManagerException&) {
    }
}
