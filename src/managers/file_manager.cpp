#include "managers/file_manager.h"

#include <QDate>
#include <QFile>
#include <fstream>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "entities/derived_employees.h"

std::map<int, bool> FileManager::employeeStatusesFromFile;

static void parseTaskField(const std::string& lineContent, int& projectId, int& taskId,
                   QString& taskName, QString& taskType,
                   int& estimatedHours, int& allocatedHours,
                   int& priority, QString& phase) {
    if (lineContent.find("PROJECT_ID:") == 0) {
        projectId = std::stoi(lineContent.substr(11));
    } else if (lineContent.find("TASK_ID:") == 0) {
        taskId = std::stoi(lineContent.substr(8));
    } else if (lineContent.find("NAME:") == 0) {
        taskName = QString::fromStdString(lineContent.substr(5));
    } else if (lineContent.find("TYPE:") == 0) {
        taskType = QString::fromStdString(lineContent.substr(5));
    } else if (lineContent.find("ESTIMATED_HOURS:") == 0) {
        estimatedHours = std::stoi(lineContent.substr(16));
    } else if (lineContent.find("ALLOCATED_HOURS:") == 0) {
        allocatedHours = std::stoi(lineContent.substr(16));
    } else if (lineContent.find("PRIORITY:") == 0) {
        priority = std::stoi(lineContent.substr(9));
    } else if (lineContent.find("PHASE:") == 0) {
        phase = QString::fromStdString(lineContent.substr(6));
    }
}

static void parseAssignmentLine(const std::string& lineContent,
                         std::vector<std::pair<int, int>>& assignments,
                         int& assignmentsRead, int assignmentsCount,
                         bool& readingAssignments) {
    size_t empPos = lineContent.find("EMPLOYEE_ID:");
    size_t hoursPos = lineContent.find("HOURS:");
    
    if (empPos == std::string::npos || hoursPos == std::string::npos) {
        return;
    }
    
    int employeeId = 0;
    int hours = 0;
    try {
        employeeId = std::stoi(lineContent.substr(empPos + 12));
        hours = std::stoi(lineContent.substr(hoursPos + 6));
    } catch (const std::exception&) {
        return;
    }
    
    if (employeeId > 0 && hours > 0) {
        assignments.push_back(std::make_pair(employeeId, hours));
        assignmentsRead++;
        if (assignmentsRead >= assignmentsCount) {
            readingAssignments = false;
        }
    }
}

int FileManager::parseIntFromStream(std::ifstream& fileStream,
                                    const QString& fieldName) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    try {
        return std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException(
            QString("Invalid %1 format in file").arg(fieldName));
    }
}

double FileManager::parseDoubleFromStream(std::ifstream& fileStream,
                                          const QString& fieldName) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    try {
        return std::stod(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException(
            QString("Invalid %1 format in file").arg(fieldName));
    }
}

QString FileManager::parseStringFromStream(std::ifstream& fileStream) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    return QString::fromStdString(lineContent);
}

double FileManager::parseEmploymentRate(std::ifstream& fileStream) {
    double employmentRate = 1.0;
    std::streampos currentPos = fileStream.tellg();
    std::string lineContent;
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
    return employmentRate;
}

void FileManager::saveEmployeeBaseData(std::shared_ptr<Employee> employee,
                                       std::ofstream& fileStream) {
    fileStream << employee->getId() << "\n";
    fileStream << employee->getName().toStdString() << "\n";
    fileStream << employee->getSalary() << "\n";
    fileStream << employee->getDepartment().toStdString() << "\n";
    fileStream << employee->getEmploymentRate() << "\n";
    fileStream << (employee->getIsActive() ? "1" : "0") << "\n";
    if (!fileStream.good()) {
        throw FileManagerException("Error writing employee base data");
    }
}

void FileManager::saveEmployeeTypeSpecificData(
    std::shared_ptr<Employee> employee, std::ofstream& fileStream) {
    if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
        fileStream << manager->getManagedProjectId() << "\n";
    } else if (auto developer =
                   std::dynamic_pointer_cast<Developer>(employee)) {
        fileStream << developer->getProgrammingLanguage().toStdString() << "\n";
        fileStream << std::fixed << std::setprecision(1) << developer->getYearsOfExperience() << "\n";
    } else if (auto designer = std::dynamic_pointer_cast<Designer>(employee)) {
        fileStream << designer->getDesignTool().toStdString() << "\n";
        fileStream << designer->getNumberOfProjects() << "\n";
    } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
        fileStream << qaEmployee->getTestingType().toStdString() << "\n";
        fileStream << qaEmployee->getBugsFound() << "\n";
    }
}

FileManager::EmployeeBaseData FileManager::loadEmployeeBaseData(
    std::ifstream& fileStream) {
    EmployeeBaseData data;
    data.id = parseIntFromStream(fileStream, "employee ID");
    data.name = parseStringFromStream(fileStream);
    data.salary = parseDoubleFromStream(fileStream, "salary");
    data.department = parseStringFromStream(fileStream);
    data.employmentRate = parseEmploymentRate(fileStream);
    
    std::streampos currentPos = fileStream.tellg();
    std::string lineContent;
    if (std::getline(fileStream, lineContent)) {
        lineContent.erase(0, lineContent.find_first_not_of(" \t\n\r"));
        lineContent.erase(lineContent.find_last_not_of(" \t\n\r") + 1);
        
        if (lineContent == "0" || lineContent == "1") {
            data.isActive = (lineContent == "1");
        } else {
            fileStream.seekg(currentPos);
            data.isActive = true;
        }
    } else {
        fileStream.seekg(currentPos);
        data.isActive = true;
    }
    
    return data;
}

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
    
    
    company.fixTaskAssignmentsToCapacity();
    
    company.recalculateTaskAllocatedHours();
    
    return company;
}

Company FileManager::loadSingleCompany(std::ifstream& fileStream) {
    std::string lineContent{};
    std::getline(fileStream, lineContent);

    QString companyName = parseStringFromStream(fileStream);
    QString companyIndustry = parseStringFromStream(fileStream);
    QString companyLocation = parseStringFromStream(fileStream);
    int companyFoundedYear = parseIntFromStream(fileStream, "founded year");

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
        } else if (auto developer =
                       std::dynamic_pointer_cast<Developer>(employee)) {
            fileStream << "DEVELOPER\n";
        } else if (auto designer =
                       std::dynamic_pointer_cast<Designer>(employee)) {
            fileStream << "DESIGNER\n";
        } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
            fileStream << "QA\n";
        } else {
            continue;
        }
        saveEmployeeBaseData(employee, fileStream);
        saveEmployeeTypeSpecificData(employee, fileStream);
    }
    fileStream << "[END_EMPLOYEES]\n";
}

void FileManager::loadEmployeesFromStream(Company& company,
                                          std::ifstream& fileStream) {
    std::string lineContent{};
    while (std::getline(fileStream, lineContent)) {
        if (lineContent == "[END_EMPLOYEES]") break;

        QString employeeType = QString::fromStdString(lineContent);
        EmployeeBaseData baseData = loadEmployeeBaseData(fileStream);

        std::shared_ptr<Employee> employee;
        
        if (employeeType == "MANAGER") {
            int managedProjectId =
                parseIntFromStream(fileStream, "managed project ID");
            employee = std::make_shared<Manager>(
                baseData.id, baseData.name, baseData.salary,
                baseData.department, managedProjectId, baseData.employmentRate);
        } else if (employeeType == "DEVELOPER") {
            QString programmingLanguage = parseStringFromStream(fileStream);
            double developerYearsOfExperience =
                parseDoubleFromStream(fileStream, "experience");
            employee = std::make_shared<Developer>(
                baseData.id, baseData.name, baseData.salary,
                baseData.department, programmingLanguage,
                developerYearsOfExperience, baseData.employmentRate);
        } else if (employeeType == "DESIGNER") {
            QString designerTool = parseStringFromStream(fileStream);
            int designerNumberOfProjects =
                parseIntFromStream(fileStream, "number of projects");
            employee = std::make_shared<Designer>(
                baseData.id, baseData.name, baseData.salary,
                baseData.department, designerTool, designerNumberOfProjects,
                baseData.employmentRate);
        } else if (employeeType == "QA") {
            QString qaTestingType = parseStringFromStream(fileStream);
            int qaBugsFound = parseIntFromStream(fileStream, "bugs found");
            employee = std::make_shared<QA>(
                baseData.id, baseData.name, baseData.salary,
                baseData.department, qaTestingType, qaBugsFound,
                baseData.employmentRate);
        }
        
        if (employee) {
            employee->setIsActive(baseData.isActive);
            company.addEmployee(employee);
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
        fileStream << project.getPhase().toStdString() << "\n";
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

        int projectId = parseIntFromStream(fileStream, "project ID");
        QString projectName = parseStringFromStream(fileStream);
        QString projectDescription = parseStringFromStream(fileStream);
        QString projectPhase = parseStringFromStream(fileStream);

        QDate projectStartDate =
            QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);
        QDate projectEndDate =
            QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);

        double projectBudget = parseDoubleFromStream(fileStream, "budget");
        QString clientName = parseStringFromStream(fileStream);

        Project project(projectId, projectName, projectDescription,
                        projectPhase, projectStartDate, projectEndDate,
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

    QString companyName = parseStringFromStream(fileStream);
    QString companyIndustry = parseStringFromStream(fileStream);
    QString companyLocation = parseStringFromStream(fileStream);
    int companyFoundedYear = parseIntFromStream(fileStream, "founded year");

    fileStream.close();

    return {companyName, companyIndustry, companyLocation, companyFoundedYear};
}

void FileManager::saveEmployees(const Company& company,
                                const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString(), std::ios::out | std::ios::trunc);
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto employees = company.getAllEmployees();
    fileStream << employees.size() << "\n";
    
    if (!fileStream.good()) {
        fileStream.close();
        throw FileManagerException("Error writing employee count to file: " + fileName);
    }

    for (const auto& employee : employees) {
        if (!employee) continue;
        saveEmployeeToStream(employee, fileStream);
        if (!fileStream.good()) {
            fileStream.close();
            throw FileManagerException("Error writing employee data to file: " + fileName);
        }
    }
    
    fileStream.flush();
    if (!fileStream.good()) {
        fileStream.close();
        throw FileManagerException("Error flushing data to file: " + fileName);
    }
    
    fileStream.close();
}

void FileManager::saveEmployeeToStream(std::shared_ptr<Employee> employee,
                                       std::ofstream& fileStream) {
    if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
        fileStream << "[MANAGER]\n";
    } else if (auto developer =
                   std::dynamic_pointer_cast<Developer>(employee)) {
        fileStream << "[DEVELOPER]\n";
    } else if (auto designer = std::dynamic_pointer_cast<Designer>(employee)) {
        fileStream << "[DESIGNER]\n";
    } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
        fileStream << "[QA]\n";
    } else {
        return;
    }
    saveEmployeeBaseData(employee, fileStream);
    saveEmployeeTypeSpecificData(employee, fileStream);
    fileStream.flush();
}

void FileManager::loadEmployees(Company& company, const QString& fileName) {
    employeeStatusesFromFile.clear();
    
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    fileStream.seekg(0, std::ios::end);
    if (fileStream.tellg() == 0) {
        fileStream.close();
        return;
    }
    fileStream.seekg(0, std::ios::beg);

    int employeeCount = parseIntFromStream(fileStream, "employee count");

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

    EmployeeBaseData baseData = loadEmployeeBaseData(fileStream);

    std::shared_ptr<Employee> employee;
    
    if (employeeType == "[MANAGER]") {
        int managedProjectId =
            parseIntFromStream(fileStream, "managed project ID");
        employee = std::make_shared<Manager>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            managedProjectId, baseData.employmentRate);
    } else if (employeeType == "[DEVELOPER]") {
        QString programmingLanguage = parseStringFromStream(fileStream);
        int developerYearsOfExperience =
            parseIntFromStream(fileStream, "experience");
        employee = std::make_shared<Developer>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            programmingLanguage, developerYearsOfExperience,
            baseData.employmentRate);
    } else if (employeeType == "[DESIGNER]") {
        QString designerTool = parseStringFromStream(fileStream);
        int designerNumberOfProjects =
            parseIntFromStream(fileStream, "number of projects");
        employee = std::make_shared<Designer>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            designerTool, designerNumberOfProjects, baseData.employmentRate);
    } else if (employeeType == "[QA]") {
        QString qaTestingType = parseStringFromStream(fileStream);
        int qaBugsFound = parseIntFromStream(fileStream, "bugs found");
        employee = std::make_shared<QA>(baseData.id, baseData.name, baseData.salary,
                                    baseData.department, qaTestingType,
                                    qaBugsFound, baseData.employmentRate);
    }
    
    if (employee) {
        employeeStatusesFromFile[baseData.id] = baseData.isActive;
        employee->setIsActive(baseData.isActive);
    }
    
    return employee;
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
    fileStream << project.getPhase().toStdString() << "\n";
    fileStream << project.getStartDate().toString(Qt::ISODate).toStdString()
               << "\n";
    fileStream << project.getEndDate().toString(Qt::ISODate).toStdString()
               << "\n";
    fileStream << project.getBudget() << "\n";
    fileStream << project.getClientName().toStdString() << "\n";
    fileStream << project.getInitialEstimatedHours() << "\n";
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
    int projectId = parseIntFromStream(fileStream, "project ID");
    QString projectName = parseStringFromStream(fileStream);
    QString projectDescription = parseStringFromStream(fileStream);
    QString projectPhase = parseStringFromStream(fileStream);

    QDate projectStartDate =
        QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);
    QDate projectEndDate =
        QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);

    double projectBudget = parseDoubleFromStream(fileStream, "budget");
    QString clientName = parseStringFromStream(fileStream);

    int estimatedHours = 0;
    if (!fileStream.eof()) {
        std::streampos pos = fileStream.tellg();
        try {
            estimatedHours = parseIntFromStream(fileStream, "estimated hours");
        } catch (const FileManagerException&) {
            fileStream.seekg(pos);
            estimatedHours = 0;
        }
    }

    Project project(projectId, projectName, projectDescription, projectPhase,
                    projectStartDate, projectEndDate, projectBudget, clientName,
                    estimatedHours);

    return project;
}

void FileManager::saveTasks(const Company& company, const QString& fileName) {
    QString tempFileName = fileName + ".tmp";
    std::ofstream fileStream(tempFileName.toStdString(), std::ios::out | std::ios::trunc);
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open temporary file for writing: " + tempFileName);
    }

    auto projects = company.getAllProjects();
    auto employees = company.getAllEmployees();

    std::vector<std::tuple<int, int, QString, QString, int, int, int, QString,
                           std::vector<std::pair<int, int>>>>
        allTasks;

    for (const auto& project : projects) {
        auto tasks = company.getProjectTasks(project.getId());
        for (const auto& task : tasks) {
            std::vector<std::pair<int, int>> assignments;

            for (const auto& emp : employees) {
                if (!emp) continue;
                
                auto taskHours = company.getEmployeeTaskHours(
                    emp->getId(), project.getId(), task.getId());
                
                if (taskHours <= 0) {
                    continue;
                }
                
                assignments.push_back(
                    std::make_pair(emp->getId(), taskHours));
            }

            allTasks.push_back(std::make_tuple(
                project.getId(), task.getId(), task.getName(), task.getType(),
                task.getEstimatedHours(), task.getAllocatedHours(),
                task.getPriority(), task.getPhase(), assignments));
        }
    }

    fileStream << "TASKS_COUNT:" << allTasks.size() << "\n";
    fileStream << "FORMAT_VERSION:2\n";
    fileStream << "---\n";
    
    if (!fileStream.good()) {
        fileStream.close();
        QFile::remove(tempFileName);
        throw FileManagerException("Error writing task header to file: " + fileName);
    }
    
    for (size_t i = 0; i < allTasks.size(); ++i) {
        const auto& taskData = allTasks[i];
        
        fileStream << "\n[TASK " << (i + 1) << "]\n";
        fileStream << "PROJECT_ID:" << std::get<0>(taskData) << "\n";
        fileStream << "TASK_ID:" << std::get<1>(taskData) << "\n";
        fileStream << "NAME:" << std::get<2>(taskData).toStdString() << "\n";
        fileStream << "TYPE:" << std::get<3>(taskData).toStdString() << "\n";
        fileStream << "ESTIMATED_HOURS:" << std::get<4>(taskData) << "\n";
        fileStream << "ALLOCATED_HOURS:" << std::get<5>(taskData) << "\n";
        fileStream << "PRIORITY:" << std::get<6>(taskData) << "\n";
        fileStream << "PHASE:" << std::get<7>(taskData).toStdString() << "\n";

        const std::vector<std::pair<int, int>>& assignments =
            std::get<8>(taskData);
        fileStream << "ASSIGNMENTS_COUNT:" << assignments.size() << "\n";
        
        if (!assignments.empty()) {
            fileStream << "ASSIGNMENTS:\n";
            int j = 1;
            for (const auto& [empId, hours] : assignments) {
                fileStream << "  [" << j << "] EMPLOYEE_ID:" << empId 
                           << " HOURS:" << hours << "\n";
                j++;
            }
        }
        
        if (!fileStream.good()) {
            fileStream.close();
            QFile::remove(tempFileName);
            throw FileManagerException("Error writing task data to file: " + fileName);
        }
    }

    fileStream.flush();
    if (!fileStream.good()) {
        fileStream.close();
        QFile::remove(tempFileName);
        throw FileManagerException("Error flushing data to file: " + fileName);
    }
    
    fileStream.close();
    
    if (QFile::exists(fileName)) {
        QFile::remove(fileName);
    }
    if (!QFile::rename(tempFileName, fileName)) {
        throw FileManagerException("Error replacing file: " + fileName);
    }
}

void FileManager::loadTasks(Company& company, const QString& fileName) {
    if (!QFile::exists(fileName)) {
        return;
    }

    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    fileStream.seekg(0, std::ios::end);
    if (fileStream.tellg() == 0) {
        fileStream.close();
        return;
    }
    fileStream.seekg(0, std::ios::beg);

    std::vector<std::string> lines;
    std::string lineContent;
    while (std::getline(fileStream, lineContent)) {
        lines.push_back(lineContent);
    }
    fileStream.close();
    
    if (lines.empty()) {
        return;
    }
    
    int lastHeaderIndex = -1;
    int taskCount = 0;
    int formatVersion = 1;
    
    for (int i = lines.size() - 1; i >= 0; --i) {
        if (lines[i].find("TASKS_COUNT:") == 0) {
            try {
                taskCount = std::stoi(lines[i].substr(12));
                lastHeaderIndex = i;
                break;
            } catch (const std::exception&) {
                continue;
            }
        }
    }
    
    if (lastHeaderIndex == -1 || taskCount == 0) {
        return;
    }
    
    if (lastHeaderIndex + 1 < static_cast<int>(lines.size()) && 
        lines[lastHeaderIndex + 1].find("FORMAT_VERSION:") == 0) {
        try {
            formatVersion = std::stoi(lines[lastHeaderIndex + 1].substr(15));
        } catch (const std::exception&) {
            formatVersion = 1;
        }
    }
    
    int startIndex = lastHeaderIndex + 2;
    if (startIndex < static_cast<int>(lines.size()) && lines[startIndex] == "---") {
        startIndex++;
    }

    int lineIndex = startIndex;
    for (int i = 0; i < taskCount && lineIndex < static_cast<int>(lines.size()); ++i) {
        int projectId = 0;
        int taskId = 0;
        QString taskName;
        QString taskType;
        int estimatedHours = 0;
        int allocatedHours = 0;
        int priority = 0;
        QString phase;
        
        if (formatVersion >= 2) {
            while (lineIndex < static_cast<int>(lines.size()) && 
                   lines[lineIndex].find("[TASK") != 0) {
                lineIndex++;
            }
            
            if (lineIndex >= static_cast<int>(lines.size())) {
                break;
            }
            
            lineIndex++;
            
            std::vector<std::pair<int, int>> assignments;
            bool readingAssignments = false;
            int assignmentsCount = 0;
            int assignmentsRead = 0;
            
            while (lineIndex < static_cast<int>(lines.size())) {
                lineContent = lines[lineIndex];
                
                if (lineContent.empty() || lineContent == "---") {
                    lineIndex++;
                    continue;
                }
                
                if (lineContent.find("[TASK") == 0 && i < taskCount - 1) {
                    break;
                }
                
                parseTaskField(lineContent, projectId, taskId, taskName, taskType,
                              estimatedHours, allocatedHours, priority, phase);
                
                if (lineContent.find("ASSIGNMENTS_COUNT:") == 0) {
                    try {
                        assignmentsCount = std::stoi(lineContent.substr(18));
                        readingAssignments = assignmentsCount > 0;
                        assignmentsRead = 0;
                    } catch (const std::exception&) {
                        assignmentsCount = 0;
                    }
                }
                
                if (readingAssignments && lineContent.find("  [") == 0) {
                    parseAssignmentLine(lineContent, assignments, assignmentsRead,
                                       assignmentsCount, readingAssignments);
                }
                
                lineIndex++;
            }
            
            if (projectId <= 0 || taskId <= 0 || taskName.isEmpty()) {
                continue;
            }
            
            try {
                Task task(taskId, taskName, taskType, estimatedHours, priority);
                task.setPhase(phase);
                task.setAllocatedHours(allocatedHours);
                company.addTaskToProject(projectId, task);
                
                for (const auto& assignment : assignments) {
                    const auto& [empId, hours] = assignment;
                    try {
                        company.assignEmployeeToTask(empId, projectId, taskId, hours);
                    } catch (const std::exception&) {
                        try {
                            company.restoreTaskAssignment(empId, projectId, taskId, hours);
                        } catch (const std::exception&) {
                            continue;
                        }
                    }
                }
            } catch (const std::exception&) {
                continue;
            }
        }
    }
}

void FileManager::saveTaskAssignments(const Company& company,
                                      const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto employees = company.getAllEmployees();
    auto projects = company.getAllProjects();

    std::vector<std::tuple<int, int, int, int>> assignments;

    for (const auto& emp : employees) {
        if (!emp) continue;

        for (const auto& project : projects) {
            auto projectId = project.getId();
            auto tasks = company.getProjectTasks(projectId);
            
            for (const auto& task : tasks) {
                int taskHours = company.getEmployeeTaskHours(
                    emp->getId(), projectId, task.getId());
                
                if (taskHours <= 0) {
                    continue;
                }
                
                assignments.push_back(
                    std::make_tuple(emp->getId(), projectId,
                                    task.getId(), taskHours));
            }
        }
    }

    fileStream << assignments.size() << "\n";
    for (const auto& assignment : assignments) {
        const auto [empId, projId, taskId, hours] = assignment;
        fileStream << empId << "\n";
        fileStream << projId << "\n";
        fileStream << taskId << "\n";
        fileStream << hours << "\n";
    }

    fileStream.flush();
    if (!fileStream.good()) {
        fileStream.close();
        throw FileManagerException("Error writing task assignments to file: " + fileName);
    }

    fileStream.close();
}

void FileManager::loadTaskAssignments(Company& company,
                                      const QString& fileName) {
    if (!QFile::exists(fileName)) {
        return;
    }

    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::string lineContent;
    std::getline(fileStream, lineContent);
    int assignmentCount = 0;
    try {
        assignmentCount = std::stoi(lineContent);
    } catch (const std::exception&) {
        fileStream.close();
        return;
    }

    for (int i = 0; i < assignmentCount; ++i) {
        std::getline(fileStream, lineContent);
        int employeeId = 0;
        try {
            employeeId = std::stoi(lineContent);
        } catch (const std::exception&) {
            continue;
        }

        std::getline(fileStream, lineContent);
        int projectId = 0;
        try {
            projectId = std::stoi(lineContent);
        } catch (const std::exception&) {
            continue;
        }

        std::getline(fileStream, lineContent);
        int taskId = 0;
        try {
            taskId = std::stoi(lineContent);
        } catch (const std::exception&) {
            continue;
        }

        std::getline(fileStream, lineContent);
        int hours = 0;
        try {
            hours = std::stoi(lineContent);
        } catch (const std::exception&) {
            continue;
        }

        try {
            company.restoreTaskAssignment(employeeId, projectId, taskId, hours);
        } catch (const std::exception&) {
            try {
                company.assignEmployeeToTask(employeeId, projectId, taskId, hours);
            } catch (const std::exception&) {
                continue;
            }
        }
    }

    fileStream.close();
    
    
    company.fixTaskAssignmentsToCapacity();
    
    company.recalculateTaskAllocatedHours();
}
