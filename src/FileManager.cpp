#include "../include/FileManager.h"
#include "../include/DerivedEmployees.h"
#include <fstream>
#include <memory>
#include <QDate>

void FileManager::saveToFile(Company& company, const QString& filename) {
    std::ofstream file(filename.toStdString());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename.toStdString());
    }
    
    saveSingleCompany(company, file);
    file.close();
}

void FileManager::saveSingleCompany(Company& company, std::ofstream& file) {
    // Save company info
    file << "[COMPANY]\n";
    file << company.getName().toStdString() << "\n";
    file << company.getIndustry().toStdString() << "\n";
    file << company.getLocation().toStdString() << "\n";
    file << company.getFoundedYear() << "\n";

    // Save employees
    file << "[EMPLOYEES]\n";
    saveEmployees(company, file);

    // Save projects
    file << "[PROJECTS]\n";
    saveProjects(company, file);
    
    file << "END_COMPANY\n";
}

Company FileManager::loadFromFile(const QString& filename) {
    std::ifstream file(filename.toStdString());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + filename.toStdString());
    }
    
    Company company = loadSingleCompany(file);
    file.close();
    return company;
}

Company FileManager::loadSingleCompany(std::ifstream& file) {
    std::string line;
    QString name, industry, location;
    int foundedYear = 0;

    // Load company info
    std::getline(file, line); // [COMPANY]
    std::getline(file, line); name = QString::fromStdString(line);
    std::getline(file, line); industry = QString::fromStdString(line);
    std::getline(file, line); location = QString::fromStdString(line);
    std::getline(file, line);
    try {
        foundedYear = std::stoi(line);
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid founded year format in file");
    }

    Company company(name, industry, location, foundedYear);

    // Load employees
    std::getline(file, line); // [EMPLOYEES]
    loadEmployees(company, file);

    // Load projects
    std::getline(file, line); // [PROJECTS]
    loadProjects(company, file);
    
    // Try to read END_COMPANY marker if present
    std::streampos pos = file.tellg();
    std::getline(file, line);
    if (line != "END_COMPANY") {
        // For backward compatibility, if END_COMPANY is not found, seek back
        file.seekg(pos);
    }

    return company;
}

void FileManager::saveEmployees(Company& company, std::ofstream& file) {
    auto employees = company.getAllEmployees();
    for (const auto& emp : employees) {
        if (auto manager = std::dynamic_pointer_cast<Manager>(emp)) {
            file << "MANAGER\n";
            file << manager->getId() << "\n";
            file << manager->getName().toStdString() << "\n";
            file << manager->getSalary() << "\n";
            file << manager->getDepartment().toStdString() << "\n";
            file << manager->getTeamSize() << "\n";
            file << manager->getProjectManaged().toStdString() << "\n";
        } else if (auto dev = std::dynamic_pointer_cast<Developer>(emp)) {
            file << "DEVELOPER\n";
            file << dev->getId() << "\n";
            file << dev->getName().toStdString() << "\n";
            file << dev->getSalary() << "\n";
            file << dev->getDepartment().toStdString() << "\n";
            file << dev->getProgrammingLanguage().toStdString() << "\n";
            file << dev->getYearsOfExperience() << "\n";
        } else if (auto designer = std::dynamic_pointer_cast<Designer>(emp)) {
            file << "DESIGNER\n";
            file << designer->getId() << "\n";
            file << designer->getName().toStdString() << "\n";
            file << designer->getSalary() << "\n";
            file << designer->getDepartment().toStdString() << "\n";
            file << designer->getDesignTool().toStdString() << "\n";
            file << designer->getNumberOfProjects() << "\n";
        } else if (auto qa = std::dynamic_pointer_cast<QA>(emp)) {
            file << "QA\n";
            file << qa->getId() << "\n";
            file << qa->getName().toStdString() << "\n";
            file << qa->getSalary() << "\n";
            file << qa->getDepartment().toStdString() << "\n";
            file << qa->getTestingType().toStdString() << "\n";
            file << qa->getBugsFound() << "\n";
        }
    }
    file << "END_EMPLOYEES\n";
}

void FileManager::loadEmployees(Company& company, std::ifstream& file) {
    std::string line;
    while (std::getline(file, line)) {
        if (line == "END_EMPLOYEES") break;
        
        QString type = QString::fromStdString(line);
        
        std::getline(file, line);
        int id;
        try {
            id = std::stoi(line);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid employee ID format in file");
        }
        
        std::getline(file, line);
        QString name = QString::fromStdString(line);
        
        std::getline(file, line);
        double salary;
        try {
            salary = std::stod(line);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid salary format in file");
        }
        
        std::getline(file, line);
        QString department = QString::fromStdString(line);

        if (type == "MANAGER") {
            std::getline(file, line);
            int teamSize;
            try {
                teamSize = std::stoi(line);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid team size format in file");
            }
            std::getline(file, line);
            QString project = QString::fromStdString(line);
            auto manager = std::make_shared<Manager>(id, name, salary, department, teamSize, project);
            company.addEmployee(manager);
        } else if (type == "DEVELOPER") {
            std::getline(file, line);
            QString language = QString::fromStdString(line);
            std::getline(file, line);
            int experience;
            try {
                experience = std::stoi(line);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid experience format in file");
            }
            auto dev = std::make_shared<Developer>(id, name, salary, department, language, experience);
            company.addEmployee(dev);
        } else if (type == "DESIGNER") {
            std::getline(file, line);
            QString tool = QString::fromStdString(line);
            std::getline(file, line);
            int numProjects;
            try {
                numProjects = std::stoi(line);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid number of projects format in file");
            }
            auto designer = std::make_shared<Designer>(id, name, salary, department, tool, numProjects);
            company.addEmployee(designer);
        } else if (type == "QA") {
            std::getline(file, line);
            QString testType = QString::fromStdString(line);
            std::getline(file, line);
            int bugs;
            try {
                bugs = std::stoi(line);
            } catch (const std::exception&) {
                throw std::runtime_error("Invalid bugs found format in file");
            }
            auto qa = std::make_shared<QA>(id, name, salary, department, testType, bugs);
            company.addEmployee(qa);
        }
    }
}

void FileManager::saveProjects(Company& company, std::ofstream& file) {
    auto projects = company.getAllProjects();
    for (const auto& proj : projects) {
        file << "[PROJECT]\n";
        file << proj.getId() << "\n";
        file << proj.getName().toStdString() << "\n";
        file << proj.getDescription().toStdString() << "\n";
        file << proj.getStatus().toStdString() << "\n";
        file << proj.getStartDate().toString(Qt::ISODate).toStdString() << "\n";
        file << proj.getEndDate().toString(Qt::ISODate).toStdString() << "\n";
        file << proj.getBudget() << "\n";
        file << proj.getClientName().toStdString() << "\n";
    }
    file << "END_PROJECTS\n";
}

void FileManager::loadProjects(Company& company, std::ifstream& file) {
    std::string line;
    while (std::getline(file, line)) {
        if (line == "END_PROJECTS") break;
        if (line != "[PROJECT]") continue;
        
        std::getline(file, line);
        int id;
        try {
            id = std::stoi(line);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid project ID format in file");
        }
        
        std::getline(file, line);
        QString name = QString::fromStdString(line);
        
        std::getline(file, line);
        QString description = QString::fromStdString(line);
        
        std::getline(file, line);
        QString status = QString::fromStdString(line);
        
        std::getline(file, line);
        QDate startDate = QDate::fromString(QString::fromStdString(line), Qt::ISODate);
        
        std::getline(file, line);
        QDate endDate = QDate::fromString(QString::fromStdString(line), Qt::ISODate);
        
        std::getline(file, line);
        double budget;
        try {
            budget = std::stod(line);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid budget format in file");
        }
        
        std::getline(file, line);
        QString client = QString::fromStdString(line);
        
        Project project(id, name, description, status, startDate, endDate, budget, client);
        company.addProject(project);
    }
}

void FileManager::saveCompanies(const std::vector<Company*>& companies, const QString& filename) {
    std::ofstream file(filename.toStdString());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filename.toStdString());
    }
    
    file << "[COMPANIES]\n";
    file << companies.size() << "\n";
    
    for (Company* company : companies) {
        if (company) {
            saveSingleCompany(*company, file);
        }
    }
    
    file << "END_COMPANIES\n";
    file.close();
}

std::vector<Company*> FileManager::loadCompanies(const QString& filename) {
    std::ifstream file(filename.toStdString());
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for reading: " + filename.toStdString());
    }
    
    std::vector<Company*> companies;
    std::string line;
    
    // Check if it's multi-company format
    std::getline(file, line);
    if (line == "[COMPANIES]") {
        // Multi-company format
        std::getline(file, line);
        int count;
        try {
            count = std::stoi(line);
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid company count format in file");
        }
        
        for (int i = 0; i < count; ++i) {
            Company company = loadSingleCompany(file);
            companies.push_back(new Company(company));
        }
        
        std::getline(file, line); // END_COMPANIES
    } else {
        // Single company format (backward compatibility)
        file.seekg(0, std::ios::beg);
        Company company = loadSingleCompany(file);
        companies.push_back(new Company(company));
    }
    
    file.close();
    return companies;
}

