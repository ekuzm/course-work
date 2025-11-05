#include "file_manager.h"

#include <QDate>
#include <fstream>
#include <memory>
#include <utility>

#include "derived_employees.h"

void FileManager::saveSingleCompany(const Company& company,
                                    std::ofstream& fileStream) {
    fileStream << "[COMPANY]\n";
    fileStream << company.getName().toStdString() << "\n";
    fileStream << company.getIndustry().toStdString() << "\n";
    fileStream << company.getLocation().toStdString() << "\n";
    fileStream << company.getFoundedYear() << "\n";

    fileStream << "[EMPLOYEES]\n";
    saveEmployeesToStream(company, fileStream);

    fileStream << "[PROJECTS]\n";
    saveProjectsToStream(company, fileStream);

    fileStream << "[END_COMPANY]\n";
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
    std::string lineContent{};
    QString companyName;
    QString companyIndustry;
    QString companyLocation;
    int companyFoundedYear = 0;

    std::getline(fileStream, lineContent);
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

    std::getline(fileStream, lineContent);
    loadEmployeesFromStream(company, fileStream);

    std::getline(fileStream, lineContent);
    loadProjectsFromStream(company, fileStream);

    std::streampos filePosition = fileStream.tellg();
    std::getline(fileStream, lineContent);
    if (lineContent != "[END_COMPANY]") {
        fileStream.seekg(filePosition);
    }

    return company;
}

void FileManager::saveEmployeesToStream(const Company& company,
                                        std::ofstream& fileStream) {
    auto employees = company.getAllEmployees();
    for (const auto& employee : employees) {
        if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
            fileStream << "MANAGER\n";
            fileStream << manager->getId() << "\n";
            fileStream << manager->getName().toStdString() << "\n";
            fileStream << manager->getSalary() << "\n";
            fileStream << manager->getDepartment().toStdString() << "\n";
            fileStream << manager->getEmploymentRate() << "\n";
            fileStream << manager->getManagedProjectId() << "\n";
        } else if (auto developer =
                       std::dynamic_pointer_cast<Developer>(employee)) {
            fileStream << "DEVELOPER\n";
            fileStream << developer->getId() << "\n";
            fileStream << developer->getName().toStdString() << "\n";
            fileStream << developer->getSalary() << "\n";
            fileStream << developer->getDepartment().toStdString() << "\n";
            fileStream << developer->getEmploymentRate() << "\n";
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
            fileStream << designer->getEmploymentRate() << "\n";
            fileStream << designer->getDesignTool().toStdString() << "\n";
            fileStream << designer->getNumberOfProjects() << "\n";
        } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
            fileStream << "QA\n";
            fileStream << qaEmployee->getId() << "\n";
            fileStream << qaEmployee->getName().toStdString() << "\n";
            fileStream << qaEmployee->getSalary() << "\n";
            fileStream << qaEmployee->getDepartment().toStdString() << "\n";
            fileStream << qaEmployee->getEmploymentRate() << "\n";
            fileStream << qaEmployee->getTestingType().toStdString() << "\n";
            fileStream << qaEmployee->getBugsFound() << "\n";
        }
    }
    fileStream << "[END_EMPLOYEES]\n";
}

void FileManager::loadEmployeesFromStream(Company& company,
                                          std::ifstream& fileStream) {
    std::string lineContent{};
    while (std::getline(fileStream, lineContent)) {
        if (lineContent == "[END_EMPLOYEES]") break;

        QString employeeType = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        int employeeId = 0;
        try {
            employeeId = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid employee ID format in file");
        }

        std::getline(fileStream, lineContent);
        QString employeeName = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        double employeeSalary = 0.0;
        try {
            employeeSalary = std::stod(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid salary format in file");
        }

        std::getline(fileStream, lineContent);
        QString employeeDepartment = QString::fromStdString(lineContent);

        double employmentRate = 1.0;
        std::streampos currentPos = fileStream.tellg();
        std::getline(fileStream, lineContent);
        try {
            double potentialRate = std::stod(lineContent);
            if (potentialRate > 0 && potentialRate <= 1.0) {
                employmentRate = potentialRate;
            } else {
                fileStream.seekg(currentPos);
            }
        } catch (const std::exception&) {
            fileStream.seekg(currentPos);
        }

        if (employeeType == "[MANAGER]") {
            std::getline(fileStream, lineContent);
            int managedProjectId = -1;
            try {
                managedProjectId = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException(
                    "Invalid managed project ID format in file");
            }
            auto manager = std::make_shared<Manager>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                managedProjectId, employmentRate);
            company.addEmployee(manager);
        } else if (employeeType == "[DEVELOPER]") {
            std::getline(fileStream, lineContent);
            QString programmingLanguage = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int developerYearsOfExperience = 0;
            try {
                developerYearsOfExperience = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException("Invalid experience format in file");
            }
            auto developer = std::make_shared<Developer>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                programmingLanguage, developerYearsOfExperience,
                employmentRate);
            company.addEmployee(developer);
        } else if (employeeType == "[DESIGNER]") {
            std::getline(fileStream, lineContent);
            QString designerTool = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int designerNumberOfProjects = 0;
            try {
                designerNumberOfProjects = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException(
                    "Invalid number of projects format in file");
            }
            auto designer = std::make_shared<Designer>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                designerTool, designerNumberOfProjects, employmentRate);
            company.addEmployee(designer);
        } else if (employeeType == "[QA]") {
            std::getline(fileStream, lineContent);
            QString qaTestingType = QString::fromStdString(lineContent);
            std::getline(fileStream, lineContent);
            int qaBugsFound = 0;
            try {
                qaBugsFound = std::stoi(lineContent);
            } catch (const std::exception&) {
                throw FileManagerException("Invalid bugs found format in file");
            }
            auto qaEmployee = std::make_shared<QA>(
                employeeId, employeeName, employeeSalary, employeeDepartment,
                qaTestingType, qaBugsFound, employmentRate);
            company.addEmployee(qaEmployee);
        }
    }
}

void FileManager::saveProjectsToStream(const Company& company,
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
    fileStream << "[END_PROJECTS]\n";
}

void FileManager::loadProjectsFromStream(Company& company,
                                         std::ifstream& fileStream) {
    std::string lineContent{};
    while (std::getline(fileStream, lineContent)) {
        if (lineContent == "END_PROJECTS]") break;
        if (lineContent != "[PROJECT]") continue;

        std::getline(fileStream, lineContent);
        int projectId = 0;
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
        double projectBudget = 0.0;
        try {
            projectBudget = std::stod(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid budget format in file");
        }

        std::getline(fileStream, lineContent);
        QString clientName = QString::fromStdString(lineContent);

        Project project(projectId, projectName, projectDescription,
                        projectStatus, projectStartDate, projectEndDate,
                        projectBudget, clientName);
        company.addProject(project);
    }
}

void FileManager::saveCompany(const Company& company, const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    fileStream << "[COMPANY]\n";
    fileStream << company.getName().toStdString() << "\n";
    fileStream << company.getIndustry().toStdString() << "\n";
    fileStream << company.getLocation().toStdString() << "\n";
    fileStream << company.getFoundedYear() << "\n";
    fileStream << "[END_COMPANY]\n";

    fileStream.close();
}

Company FileManager::loadCompany(const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::string lineContent{};
    std::getline(fileStream, lineContent);

    std::getline(fileStream, lineContent);
    QString companyName = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    QString companyIndustry = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    QString companyLocation = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    int companyFoundedYear = 0;
    try {
        companyFoundedYear = std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid founded year format in file");
    }

    fileStream.close();

    return {companyName, companyIndustry, companyLocation, companyFoundedYear};
}

void FileManager::saveEmployees(const Company& company,
                                const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto employees = company.getAllEmployees();
    fileStream << employees.size() << "\n";

    for (const auto& employee : employees) {
        saveEmployeeToStream(employee, fileStream);
    }

    fileStream.close();
}

void FileManager::saveEmployeeToStream(std::shared_ptr<Employee> employee,
                                       std::ofstream& fileStream) {
    if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
        fileStream << "[MANAGER]\n";
        fileStream << manager->getId() << "\n";
        fileStream << manager->getName().toStdString() << "\n";
        fileStream << manager->getSalary() << "\n";
        fileStream << manager->getDepartment().toStdString() << "\n";
        fileStream << manager->getEmploymentRate() << "\n";
        fileStream << manager->getManagedProjectId() << "\n";
    } else if (auto developer =
                   std::dynamic_pointer_cast<Developer>(employee)) {
        fileStream << "[DEVELOPER]\n";
        fileStream << developer->getId() << "\n";
        fileStream << developer->getName().toStdString() << "\n";
        fileStream << developer->getSalary() << "\n";
        fileStream << developer->getDepartment().toStdString() << "\n";
        fileStream << developer->getEmploymentRate() << "\n";
        fileStream << developer->getProgrammingLanguage().toStdString() << "\n";
        fileStream << developer->getYearsOfExperience() << "\n";
    } else if (auto designer = std::dynamic_pointer_cast<Designer>(employee)) {
        fileStream << "[DESIGNER]\n";
        fileStream << designer->getId() << "\n";
        fileStream << designer->getName().toStdString() << "\n";
        fileStream << designer->getSalary() << "\n";
        fileStream << designer->getDepartment().toStdString() << "\n";
        fileStream << designer->getEmploymentRate() << "\n";
        fileStream << designer->getDesignTool().toStdString() << "\n";
        fileStream << designer->getNumberOfProjects() << "\n";
    } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
        fileStream << "[QA]\n";
        fileStream << qaEmployee->getId() << "\n";
        fileStream << qaEmployee->getName().toStdString() << "\n";
        fileStream << qaEmployee->getSalary() << "\n";
        fileStream << qaEmployee->getDepartment().toStdString() << "\n";
        fileStream << qaEmployee->getEmploymentRate() << "\n";
        fileStream << qaEmployee->getTestingType().toStdString() << "\n";
        fileStream << qaEmployee->getBugsFound() << "\n";
    }
}

void FileManager::loadEmployees(Company& company, const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::string lineContent{};
    std::getline(fileStream, lineContent);
    int employeeCount = 0;
    try {
        employeeCount = std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid employee count format in file");
    }

    for (int i = 0; i < employeeCount; ++i) {
        auto employee = loadEmployeeFromStream(fileStream);
        if (employee) {
            company.addEmployee(employee);
        }
    }

    fileStream.close();
}

std::shared_ptr<Employee> FileManager::loadEmployeeFromStream(
    std::ifstream& fileStream) {
    std::string lineContent{};
    std::getline(fileStream, lineContent);
    QString employeeType = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    int employeeId = 0;
    try {
        employeeId = std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid employee ID format in file");
    }

    std::getline(fileStream, lineContent);
    QString employeeName = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    double employeeSalary = 0.0;
    try {
        employeeSalary = std::stod(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid salary format in file");
    }

    std::getline(fileStream, lineContent);
    QString employeeDepartment = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    double employmentRate = 1.0;
    try {
        employmentRate = std::stod(lineContent);
    } catch (const std::exception&) {
        employmentRate = 1.0;
    }

    if (employeeType == "[MANAGER]") {
        std::getline(fileStream, lineContent);
        int managedProjectId = -1;
        try {
            managedProjectId = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException(
                "Invalid managed project ID format in file");
        }
        return std::make_shared<Manager>(employeeId, employeeName,
                                         employeeSalary, employeeDepartment,
                                         managedProjectId, employmentRate);
    }
    if (employeeType == "[DEVELOPER]") {
        std::getline(fileStream, lineContent);
        QString programmingLanguage = QString::fromStdString(lineContent);
        std::getline(fileStream, lineContent);
        int developerYearsOfExperience = 0;
        try {
            developerYearsOfExperience = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid experience format in file");
        }
        return std::make_shared<Developer>(
            employeeId, employeeName, employeeSalary, employeeDepartment,
            programmingLanguage, developerYearsOfExperience, employmentRate);
    }
    if (employeeType == "[DESIGNER]") {
        std::getline(fileStream, lineContent);
        QString designerTool = QString::fromStdString(lineContent);
        std::getline(fileStream, lineContent);
        int designerNumberOfProjects = 0;
        try {
            designerNumberOfProjects = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException(
                "Invalid number of projects format in file");
        }
        return std::make_shared<Designer>(
            employeeId, employeeName, employeeSalary, employeeDepartment,
            designerTool, designerNumberOfProjects, employmentRate);
    }
    if (employeeType == "[QA]") {
        std::getline(fileStream, lineContent);
        QString qaTestingType = QString::fromStdString(lineContent);
        std::getline(fileStream, lineContent);
        int qaBugsFound = 0;
        try {
            qaBugsFound = std::stoi(lineContent);
        } catch (const std::exception&) {
            throw FileManagerException("Invalid bugs found format in file");
        }
        return std::make_shared<QA>(employeeId, employeeName, employeeSalary,
                                    employeeDepartment, qaTestingType,
                                    qaBugsFound, employmentRate);
    }

    return nullptr;
}

void FileManager::saveProjects(const Company& company,
                               const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto projects = company.getAllProjects();
    fileStream << projects.size() << "\n";

    for (const auto& project : projects) {
        saveProjectToStream(project, fileStream);
    }

    fileStream.close();
}

void FileManager::saveProjectToStream(const Project& project,
                                      std::ofstream& fileStream) {
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
    fileStream << project.getInitialEstimatedHours() << "\n";

    const auto& tasks = project.getTasks();
    fileStream << tasks.size() << "\n";
    for (const auto& task : tasks) {
        fileStream << task.getId() << "\n";
        fileStream << task.getName().toStdString() << "\n";
        fileStream << task.getType().toStdString() << "\n";
        fileStream << task.getEstimatedHours() << "\n";
        fileStream << task.getAllocatedHours() << "\n";
        fileStream << task.getPriority() << "\n";
        fileStream << task.getStatus().toStdString() << "\n";
    }
}

void FileManager::loadProjects(Company& company, const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::string lineContent{};
    std::getline(fileStream, lineContent);
    int projectCount = 0;
    try {
        projectCount = std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid project count format in file");
    }

    for (int i = 0; i < projectCount; ++i) {
        Project project = loadProjectFromStream(fileStream);
        company.addProject(project);
    }

    fileStream.close();
}

Project FileManager::loadProjectFromStream(std::ifstream& fileStream) {
    std::string lineContent{};

    std::getline(fileStream, lineContent);
    int projectId = 0;
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
    double projectBudget = 0.0;
    try {
        projectBudget = std::stod(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException("Invalid budget format in file");
    }

    std::getline(fileStream, lineContent);
    QString clientName = QString::fromStdString(lineContent);

    std::getline(fileStream, lineContent);
    int estimatedHours = 0;
    try {
        estimatedHours = std::stoi(lineContent);
    } catch (const std::exception&) {
        estimatedHours = 0;
    }

    Project project(projectId, projectName, projectDescription, projectStatus,
                    projectStartDate, projectEndDate, projectBudget, clientName,
                    estimatedHours);

    std::getline(fileStream, lineContent);
    int taskCount = 0;
    try {
        taskCount = std::stoi(lineContent);
    } catch (const std::exception&) {
        taskCount = 0;
    }

    for (int taskIndex = 0; taskIndex < taskCount; ++taskIndex) {
        std::getline(fileStream, lineContent);
        int taskId = 0;
        try {
            taskId = std::stoi(lineContent);
        } catch (const std::exception&) {
            continue;
        }

        std::getline(fileStream, lineContent);
        QString taskName = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        QString taskType = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        int estimatedTaskHours = 0;
        try {
            estimatedTaskHours = std::stoi(lineContent);
        } catch (const std::exception&) {
            estimatedTaskHours = 0;
        }

        std::getline(fileStream, lineContent);
        int allocatedTaskHours = 0;
        try {
            allocatedTaskHours = std::stoi(lineContent);
        } catch (const std::exception&) {
            allocatedTaskHours = 0;
        }

        std::getline(fileStream, lineContent);
        int priority = 0;
        try {
            priority = std::stoi(lineContent);
        } catch (const std::exception&) {
            priority = 0;
        }

        std::getline(fileStream, lineContent);
        QString status = QString::fromStdString(lineContent);

        Task task(taskId, taskName, taskType, estimatedTaskHours, priority);
        task.setAllocatedHours(allocatedTaskHours);
        task.setStatus(status);

        project.addTask(task);
    }

    return project;
}
