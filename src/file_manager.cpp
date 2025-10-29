#include "file_manager.h"

#include <QDate>
#include <fstream>
#include <memory>
#include <utility>

#include "../include/derived_employees.h"

FileManagerException::FileManagerException(QString msg) : message(std::move(msg)) {}

const char* FileManagerException::what() const noexcept {
    return message.toLocal8Bit().constData();
}

void FileManager::saveToFile(Company& company, const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    saveSingleCompany(company, fileStream);
    fileStream.close();
}

void FileManager::saveSingleCompany(const Company& company,
                                    std::ofstream& fileStream) {
    // Save company info
    fileStream << "[COMPANY]\n";
    fileStream << company.getName().toStdString() << "\n";
    fileStream << company.getIndustry().toStdString() << "\n";
    fileStream << company.getLocation().toStdString() << "\n";
    fileStream << company.getFoundedYear() << "\n";

    // Save employees
    fileStream << "[EMPLOYEES]\n";
    saveEmployees(company, fileStream);

    // Save projects
    fileStream << "[PROJECTS]\n";
    saveProjects(company, fileStream);

    fileStream << "END_COMPANY\n";
}

Company FileManager::loadFromFile(const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    Company company = loadSingleCompany(fileStream);
    fileStream.close();
    return company;
}

Company FileManager::loadSingleCompany(std::ifstream& fileStream) {
    std::string lineContent;
    QString companyName, companyIndustry, companyLocation;
    int companyFoundedYear = 0;

    // Load company info
    std::getline(fileStream, lineContent);  // [COMPANY]
    std::getline(fileStream, lineContent);
    companyName = QString::fromStdString(lineContent);
    std::getline(fileStream, lineContent);
    companyIndustry = QString::fromStdString(lineContent);
    std::getline(fileStream, lineContent);
    companyLocation = QString::fromStdString(lineContent);
    std::getline(fileStream, lineContent);
    try {
        companyFoundedYear = std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid founded year format in file");
    }

    Company company(companyName, companyIndustry, companyLocation,
                    companyFoundedYear);

    // Load employees
    std::getline(fileStream, lineContent);  // [EMPLOYEES]
    loadEmployees(company, fileStream);

    // Load projects
    std::getline(fileStream, lineContent);  // [PROJECTS]
    loadProjects(company, fileStream);

    // Try to read END_COMPANY marker if present
    std::streampos filePosition = fileStream.tellg();
    std::getline(fileStream, lineContent);
    if (lineContent != "END_COMPANY") {
        // For backward compatibility, if END_COMPANY is not found, seek back
        fileStream.seekg(filePosition);
    }

    return company;
}

void FileManager::saveEmployees(const Company& company,
                                std::ofstream& fileStream) {
    auto employees = company.getAllEmployees();
    for (const auto& employee : employees) {
        if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
            fileStream << "MANAGER\n";
            fileStream << manager->getId() << "\n";
            fileStream << manager->getName().toStdString() << "\n";
            fileStream << manager->getSalary() << "\n";
            fileStream << manager->getDepartment().toStdString() << "\n";
            fileStream << manager->getTeamSize() << "\n";
            fileStream << manager->getProjectManaged().toStdString() << "\n";
        } else if (auto developer =
                       std::dynamic_pointer_cast<Developer>(employee)) {
            fileStream << "DEVELOPER\n";
            fileStream << developer->getId() << "\n";
            fileStream << developer->getName().toStdString() << "\n";
            fileStream << developer->getSalary() << "\n";
            fileStream << developer->getDepartment().toStdString() << "\n";
            fileStream << developer->getProgrammingLanguage().toStdString()
                       << "\n";
            fileStream << developer->getYearsOfExperience() << "\n";
        } else if (auto designer =
                       std::dynamic_pointer_cast<Designer>(employee)) {
            fileStream << "DESIGNER\n";
            fileStream << designer->getId() << "\n";
            fileStream << designer->getName().toStdString() << "\n";
            fileStream << designer->getSalary() << "\n";
            fileStream << designer->getDepartment().toStdString() << "\n";
            fileStream << designer->getDesignTool().toStdString() << "\n";
            fileStream << designer->getNumberOfProjects() << "\n";
        } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
            fileStream << "QA\n";
            fileStream << qaEmployee->getId() << "\n";
            fileStream << qaEmployee->getName().toStdString() << "\n";
            fileStream << qaEmployee->getSalary() << "\n";
            fileStream << qaEmployee->getDepartment().toStdString() << "\n";
            fileStream << qaEmployee->getTestingType().toStdString() << "\n";
            fileStream << qaEmployee->getBugsFound() << "\n";
        }
    }
    fileStream << "END_EMPLOYEES\n";
}

void FileManager::loadEmployees(Company& company, std::ifstream& fileStream) {
    std::string lineContent;
    while (std::getline(fileStream, lineContent)) {
        if (lineContent == "END_EMPLOYEES") break;

        QString employeeType = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        int employeeId;
        try {
            employeeId = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid employee ID format in file");
        }

        std::getline(fileStream, lineContent);
        QString employeeName = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        double employeeSalary;
        try {
            employeeSalary = std::stod(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid salary format in file");
        }

        std::getline(fileStream, lineContent);
        QString employeeDepartment = QString::fromStdString(lineContent);

        if (employeeType == "MANAGER") {
            std::getline(fileStream, lineContent);
            int managerTeamSize;
            try {
                managerTeamSize = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException("Invalid team size format in file");
            }
            std::getline(fileStream, lineContent);
            QString managedProject = QString::fromStdString(lineContent);
            auto manager = std::make_shared<Manager>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                managerTeamSize, managedProject);
            company.addEmployee(manager);
        } else if (employeeType == "DEVELOPER") {
            std::getline(fileStream, lineContent);
            QString programmingLanguage = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int developerYearsOfExperience;
            try {
                developerYearsOfExperience = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException("Invalid experience format in file");
            }
            auto developer = std::make_shared<Developer>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                programmingLanguage, developerYearsOfExperience);
            company.addEmployee(developer);
        } else if (employeeType == "DESIGNER") {
            std::getline(fileStream, lineContent);
            QString designerTool = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int designerNumberOfProjects;
            try {
                designerNumberOfProjects = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException(
                    "Invalid number of projects format in file");
            }
            auto designer = std::make_shared<Designer>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                designerTool, designerNumberOfProjects);
            company.addEmployee(designer);
        } else if (employeeType == "QA") {
            std::getline(fileStream, lineContent);
            QString qaTestingType = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int qaBugsFound;
            try {
                qaBugsFound = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException("Invalid bugs found format in file");
            }
            auto qaEmployee = std::make_shared<QA>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                qaTestingType, qaBugsFound);
            company.addEmployee(qaEmployee);
        }
    }
}

void FileManager::saveProjects(const Company& company,
                               std::ofstream& fileStream) {
    auto projects = company.getAllProjects();
    for (const auto& project : projects) {
        fileStream << "[PROJECT]\n";
        fileStream << project.getId() << "\n";
        fileStream << project.getName().toStdString() << "\n";
        fileStream << project.getDescription().toStdString() << "\n";
        fileStream << project.getStatus().toStdString() << "\n";
        fileStream << project.getStartDate().toString(Qt::ISODate).toStdString()
                   << "\n";
        fileStream << project.getEndDate().toString(Qt::ISODate).toStdString()
                   << "\n";
        fileStream << project.getBudget() << "\n";
        fileStream << project.getClientName().toStdString() << "\n";
    }
    fileStream << "END_PROJECTS\n";
}

void FileManager::loadProjects(Company& company, std::ifstream& fileStream) {
    std::string lineContent;
    while (std::getline(fileStream, lineContent)) {
        if (lineContent == "END_PROJECTS") break;
        if (lineContent != "[PROJECT]") continue;

        std::getline(fileStream, lineContent);
        int projectId;
        try {
            projectId = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid project ID format in file");
        }

        std::getline(fileStream, lineContent);
        QString projectName = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        QString projectDescription = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        QString projectStatus = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        QDate projectStartDate =
            QDate::fromString(QString::fromStdString(lineContent), Qt::ISODate);

        std::getline(fileStream, lineContent);
        QDate projectEndDate =
            QDate::fromString(QString::fromStdString(lineContent), Qt::ISODate);

        std::getline(fileStream, lineContent);
        double projectBudget;
        try {
            projectBudget = std::stod(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid budget format in file");
        }

        std::getline(fileStream, lineContent);
        QString clientName = QString::fromStdString(lineContent);

        ProjectParams params{projectName, projectDescription, projectStatus,
                             projectStartDate, projectEndDate, projectBudget,
                             clientName};
        Project project(projectId, params);
        company.addProject(project);
    }
}

void FileManager::saveCompanies(const std::vector<Company*>& companies,
                                const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    fileStream << "[COMPANIES]\n";
    fileStream << companies.size() << "\n";

    for (const Company* company : companies) {
        if (company) {
            saveSingleCompany(*company, fileStream);
        }
    }

    fileStream << "END_COMPANIES\n";
    fileStream.close();
}

std::vector<Company*> FileManager::loadCompanies(const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::vector<Company*> companies;
    std::string lineContent;

    // Check if it's multi-company format
    std::getline(fileStream, lineContent);
    if (lineContent == "[COMPANIES]") {
        // Multi-company format
        std::getline(fileStream, lineContent);
        int companyCount;
        try {
            companyCount = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid company count format in file");
        }

        for (int index = 0; index < companyCount; ++index) {
            Company company = loadSingleCompany(fileStream);
            companies.push_back(new Company(company));
        }

        std::getline(fileStream, lineContent);  // END_COMPANIES
    } else {
        // Single company format (backward compatibility)
        fileStream.seekg(0, std::ios::beg);
        Company company = loadSingleCompany(fileStream);
        companies.push_back(new Company(company));
    }

    fileStream.close();
    return companies;
}
