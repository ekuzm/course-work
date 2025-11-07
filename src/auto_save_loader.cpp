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

void AutoSaveLoader::autoSave(const std::vector<Company*>& companies, MainWindow* mainWindow) {
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

        clearDataFiles(dataDirPath);

        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] == nullptr) continue;

            QString index = QString::number(i + 1);
            QString companyFileName = QString("company_%1.txt").arg(index);
            QString companyFilePath = companiesDir.absoluteFilePath(companyFileName);
            FileManager::saveCompany(*companies[i], companyFilePath);

            QString employeesFileName = QString("employees_%1.txt").arg(index);
            QString employeesFilePath = employeesDir.absoluteFilePath(employeesFileName);
            FileManager::saveEmployees(*companies[i], employeesFilePath);

            QString projectsFileName = QString("projects_%1.txt").arg(index);
            QString projectsFilePath = projectsDir.absoluteFilePath(projectsFileName);
            FileManager::saveProjects(*companies[i], projectsFilePath);

            QString tasksFileName = QString("tasks_%1.txt").arg(index);
            QString tasksFilePath = projectsDir.absoluteFilePath(tasksFileName);
            FileManager::saveTasks(*companies[i], tasksFilePath);
        }
    } catch (const FileManagerException&) {
        // Ignore save errors
    }
}

void AutoSaveLoader::autoLoad(std::vector<Company*>& companies, Company*& currentCompany,
                              int& currentCompanyIndex, MainWindow* mainWindow) {
    try {
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
            int companyIndex = index.toInt(&conversionOk);
            if (!conversionOk) continue;

            try {
                QString companyFilePath = companiesDir.absoluteFilePath(fileName);
                Company company = FileManager::loadCompany(companyFilePath);

                QString employeesFileName = QString("employees_%1.txt").arg(index);
                QString employeesFilePath = employeesDir.absoluteFilePath(employeesFileName);
                if (QFile::exists(employeesFilePath)) {
                    FileManager::loadEmployees(company, employeesFilePath);
                }

                QString projectsFileName = QString("projects_%1.txt").arg(index);
                QString projectsFilePath = projectsDir.absoluteFilePath(projectsFileName);
                if (QFile::exists(projectsFilePath)) {
                    FileManager::loadProjects(company, projectsFilePath);
                }

                QString tasksFileName = QString("tasks_%1.txt").arg(index);
                QString tasksFilePath = projectsDir.absoluteFilePath(tasksFileName);
                if (QFile::exists(tasksFilePath)) {
                    FileManager::loadTasks(company, tasksFilePath);
                }

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
        // Ignore load errors
    }
}

