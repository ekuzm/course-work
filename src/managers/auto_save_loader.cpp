#include "managers/auto_save_loader.h"

#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <ranges>

#include "managers/file_manager.h"
#include "ui/main_window.h"

static bool hasEmployeeTaskAssignments(const Company& company, int employeeId,
                                       int projectId) {
    auto tasks = company.getProjectTasks(projectId);
    return std::ranges::any_of(
        tasks, [&company, employeeId, projectId](const auto& task) {
            return company.getEmployeeHours(employeeId, projectId,
                                                task.getId()) > 0;
        });
}

static void saveCompanyFile(const Company* company, const QString& filePath,
                            MainWindow* mainWindow, const QString& errorMsg) {
    try {
        FileManager::saveCompany(*company, filePath);
    } catch (const FileManagerException& e) {
        if (mainWindow) {
            QMessageBox::warning(mainWindow, "Auto-save Error",
                                 QString("%1: %2").arg(errorMsg).arg(e.what()));
        }
    }
}

static void saveEmployeesFile(const Company* company, const QString& filePath,
                              MainWindow* mainWindow) {
    try {
        FileManager::saveEmployees(*company, filePath);
    } catch (const FileManagerException& e) {
        if (mainWindow) {
            QMessageBox::warning(
                mainWindow, "Auto-save Error",
                QString("Failed to save employees: %1").arg(e.what()));
        }
    }
}

static void saveProjectsFile(const Company* company, const QString& filePath,
                             MainWindow* mainWindow) {
    try {
        FileManager::saveProjects(*company, filePath);
    } catch (const FileManagerException& e) {
        if (mainWindow) {
            QMessageBox::warning(
                mainWindow, "Auto-save Error",
                QString("Failed to save projects: %1").arg(e.what()));
        }
    }
}

static void saveTasksFile(const Company* company, const QString& filePath,
                          MainWindow* mainWindow) {
    try {
        FileManager::saveTasks(*company, filePath);
    } catch (const FileManagerException& e) {
        if (mainWindow) {
            QMessageBox::warning(
                mainWindow, "Auto-save Error",
                QString("Failed to save tasks: %1").arg(e.what()));
        }
    }
}

static void loadCompanyFiles(Company& company, const QString& index,
                             const QDir& employeesDir,
                             const QDir& projectsDir) {
    if (QString employeesFilePath = employeesDir.absoluteFilePath(
            QString("employees_%1.txt").arg(index));
        QFile::exists(employeesFilePath)) {
        FileManager::loadEmployees(company, employeesFilePath);
    }

    if (QString projectsFilePath =
            projectsDir.absoluteFilePath(QString("projects_%1.txt").arg(index));
        QFile::exists(projectsFilePath)) {
        FileManager::loadProjects(company, projectsFilePath);
    }

    if (QString tasksFilePath =
            projectsDir.absoluteFilePath(QString("tasks_%1.txt").arg(index));
        QFile::exists(tasksFilePath)) {
        FileManager::loadTasks(company, tasksFilePath);
    }

    QString taskAssignmentsFilePath = projectsDir.absoluteFilePath(
        QString("task_assignments_%1.txt").arg(index));
    if (QFile::exists(taskAssignmentsFilePath)) {
        FileManager::loadTaskAssignments(company, taskAssignmentsFilePath);
    }
}

static void processInactiveEmployee(const std::shared_ptr<Employee>& emp,
                                    const Company& company) {
    auto assignedProjects = emp->getAssignedProjects();
    auto allProjects = company.getAllProjects();

    for (const auto& project : allProjects) {
        int projectId = project.getId();
        if (hasEmployeeTaskAssignments(company, emp->getId(), projectId)) {
            emp->addToProjectHistory(projectId);
        }
    }

    for (int projectId : assignedProjects) {
        emp->addToProjectHistory(projectId);
    }

    if (emp->getCurrentWeeklyHours() > 0) {
        int currentHours = emp->getCurrentWeeklyHours();
        emp->removeWeeklyHours(currentHours);
    }
}

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
        if (fileName.startsWith("tasks_") || fileName.startsWith("task_assignments_")) {
            continue;
        }
        projectsDir.remove(fileName);
    }
}

void AutoSaveLoader::autoSave(const std::vector<Company*>& companies,
                              MainWindow* mainWindow) {
    try {
        QString dataDirPath = getDataDirectory();
        if (QDir dataDir(dataDirPath); !dataDir.exists()) {
            dataDir.mkpath(".");
        }

        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (!companiesDir.exists()) companiesDir.mkpath(".");
        if (!employeesDir.exists()) employeesDir.mkpath(".");
        if (!projectsDir.exists()) projectsDir.mkpath(".");

        if (bool hasValidCompanies = std::ranges::any_of(
                companies,
                [](const auto* company) { return company != nullptr; });
            hasValidCompanies) {
            clearDataFiles(dataDirPath);
        }

        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] == nullptr) continue;

            QString index = QString::number(i + 1);
            QString companyFilePath = companiesDir.absoluteFilePath(
                QString("company_%1.txt").arg(index));
            saveCompanyFile(companies[i], companyFilePath, mainWindow,
                            "Failed to save company data");
            if (!QFile::exists(companyFilePath)) continue;

            QString employeesFilePath = employeesDir.absoluteFilePath(
                QString("employees_%1.txt").arg(index));
            saveEmployeesFile(companies[i], employeesFilePath, mainWindow);

            QString projectsFilePath = projectsDir.absoluteFilePath(
                QString("projects_%1.txt").arg(index));
            saveProjectsFile(companies[i], projectsFilePath, mainWindow);

            QString tasksFilePath = projectsDir.absoluteFilePath(
                QString("tasks_%1.txt").arg(index));
            saveTasksFile(companies[i], tasksFilePath, mainWindow);
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
    QString dataDirPath = getDataDirectory();
    QDir companiesDir(dataDirPath + "/companies");
    QDir employeesDir(dataDirPath + "/employees");
    QDir projectsDir(dataDirPath + "/projects");

    if (!companiesDir.exists()) return;

    companiesDir.setNameFilters(QStringList() << "company_*.txt");
    companiesDir.setFilter(QDir::Files);
    QStringList companyFiles = companiesDir.entryList(QDir::Files, QDir::Name);

    if (companyFiles.isEmpty()) return;

    std::vector<Company*> loadedCompanies;

    for (const QString& fileName : companyFiles) {
        QString index = fileName;
        index.replace("company_", "").replace(".txt", "");
        bool conversionOk = false;
        [[maybe_unused]] int companyIndex = index.toInt(&conversionOk);
        if (!conversionOk) continue;

        try {
            QString companyFilePath = companiesDir.absoluteFilePath(fileName);
            Company company = FileManager::loadCompany(companyFilePath);

            loadCompanyFiles(company, index, employeesDir, projectsDir);

            company.recalculateEmployeeHours();
            company.fixTaskAssignmentsToCapacity();
            company.recalculateTaskAllocatedHours();

            auto employees = company.getAllEmployees();
            for (const auto& emp : employees) {
                if (!emp) continue;

                auto it =
                    FileManager::employeeStatusesFromFile.find(emp->getId());
                if (it == FileManager::employeeStatusesFromFile.end()) {
                    continue;
                }

                bool shouldBeActive = it->second;
                emp->setIsActive(shouldBeActive);

                if (!shouldBeActive) {
                    processInactiveEmployee(emp, company);
                }
            }
            FileManager::employeeStatusesFromFile.clear();

            if (mainWindow) {
                mainWindow->validateAndFixProjectAssignments(&company);
            }

            auto* companyPtr = new Company(std::move(company));
            loadedCompanies.push_back(companyPtr);
        } catch (const FileManagerException&) {
            continue;
        }
    }

    companies = loadedCompanies;
    if (!companies.empty()) {
        currentCompany = companies[0];
        currentCompanyIndex = 0;
    }
}
