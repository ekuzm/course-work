#include "file_manager.h"

#include <QDate>
#include <QFile>
#include <fstream>
#include <memory>
#include <utility>
#include <sstream>

#include "derived_employees.h"

// Helper functions for parsing
int FileManager::parseIntFromStream(std::ifstream& fileStream, const QString& fieldName) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    try {
        return std::stoi(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException(QString("Invalid %1 format in file").arg(fieldName));
    }
}

double FileManager::parseDoubleFromStream(std::ifstream& fileStream, const QString& fieldName) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    try {
        return std::stod(lineContent);
    } catch (const std::exception&) {
        throw FileManagerException(QString("Invalid %1 format in file").arg(fieldName));
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

// Helper function to save base employee data
void FileManager::saveEmployeeBaseData(std::shared_ptr<Employee> employee, std::ofstream& fileStream) {
    fileStream << employee->getId() << "\n";
    fileStream << employee->getName().toStdString() << "\n";
    fileStream << employee->getSalary() << "\n";
    fileStream << employee->getDepartment().toStdString() << "\n";
    fileStream << employee->getEmploymentRate() << "\n";
}

// Helper function to save type-specific employee data
void FileManager::saveEmployeeTypeSpecificData(std::shared_ptr<Employee> employee, std::ofstream& fileStream) {
    if (auto manager = std::dynamic_pointer_cast<Manager>(employee)) {
        fileStream << manager->getManagedProjectId() << "\n";
    } else if (auto developer = std::dynamic_pointer_cast<Developer>(employee)) {
        fileStream << developer->getProgrammingLanguage().toStdString() << "\n";
        fileStream << developer->getYearsOfExperience() << "\n";
    } else if (auto designer = std::dynamic_pointer_cast<Designer>(employee)) {
        fileStream << designer->getDesignTool().toStdString() << "\n";
        fileStream << designer->getNumberOfProjects() << "\n";
    } else if (auto qaEmployee = std::dynamic_pointer_cast<QA>(employee)) {
        fileStream << qaEmployee->getTestingType().toStdString() << "\n";
        fileStream << qaEmployee->getBugsFound() << "\n";
    }
}

// Helper function to load base employee data
FileManager::EmployeeBaseData FileManager::loadEmployeeBaseData(std::ifstream& fileStream) {
    EmployeeBaseData data;
    data.id = parseIntFromStream(fileStream, "employee ID");
    data.name = parseStringFromStream(fileStream);
    data.salary = parseDoubleFromStream(fileStream, "salary");
    data.department = parseStringFromStream(fileStream);
    data.employmentRate = parseEmploymentRate(fileStream);
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
    return company;
}

Company FileManager::loadSingleCompany(std::ifstream& fileStream) {
    std::string lineContent{};
    std::getline(fileStream, lineContent); // Skip [COMPANY]
    
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
        } else if (auto developer = std::dynamic_pointer_cast<Developer>(employee)) {
            fileStream << "DEVELOPER\n";
        } else if (auto designer = std::dynamic_pointer_cast<Designer>(employee)) {
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

        if (employeeType == "MANAGER") {
            int managedProjectId = parseIntFromStream(fileStream, "managed project ID");
            auto manager = std::make_shared<Manager>(
                baseData.id, baseData.name, baseData.salary, baseData.department,
                managedProjectId, baseData.employmentRate);
            company.addEmployee(manager);
        } else if (employeeType == "DEVELOPER") {
            QString programmingLanguage = parseStringFromStream(fileStream);
            int developerYearsOfExperience = parseIntFromStream(fileStream, "experience");
            auto developer = std::make_shared<Developer>(
                baseData.id, baseData.name, baseData.salary, baseData.department,
                programmingLanguage, developerYearsOfExperience, baseData.employmentRate);
            company.addEmployee(developer);
        } else if (employeeType == "DESIGNER") {
            QString designerTool = parseStringFromStream(fileStream);
            int designerNumberOfProjects = parseIntFromStream(fileStream, "number of projects");
            auto designer = std::make_shared<Designer>(
                baseData.id, baseData.name, baseData.salary, baseData.department,
                designerTool, designerNumberOfProjects, baseData.employmentRate);
            company.addEmployee(designer);
        } else if (employeeType == "QA") {
            QString qaTestingType = parseStringFromStream(fileStream);
            int qaBugsFound = parseIntFromStream(fileStream, "bugs found");
            auto qaEmployee = std::make_shared<QA>(
                baseData.id, baseData.name, baseData.salary, baseData.department,
                qaTestingType, qaBugsFound, baseData.employmentRate);
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

        int projectId = parseIntFromStream(fileStream, "project ID");
        QString projectName = parseStringFromStream(fileStream);
        QString projectDescription = parseStringFromStream(fileStream);
        QString projectStatus = parseStringFromStream(fileStream);
        
        QDate projectStartDate = QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);
        QDate projectEndDate = QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);

        double projectBudget = parseDoubleFromStream(fileStream, "budget");
        QString clientName = parseStringFromStream(fileStream);

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
    std::getline(fileStream, lineContent); // Skip [COMPANY]

    QString companyName = parseStringFromStream(fileStream);
    QString companyIndustry = parseStringFromStream(fileStream);
    QString companyLocation = parseStringFromStream(fileStream);
    int companyFoundedYear = parseIntFromStream(fileStream, "founded year");

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
    } else if (auto developer = std::dynamic_pointer_cast<Developer>(employee)) {
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
}

void FileManager::loadEmployees(Company& company, const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

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

    if (employeeType == "[MANAGER]") {
        int managedProjectId = parseIntFromStream(fileStream, "managed project ID");
        return std::make_shared<Manager>(baseData.id, baseData.name,
                                         baseData.salary, baseData.department,
                                         managedProjectId, baseData.employmentRate);
    }
    if (employeeType == "[DEVELOPER]") {
        QString programmingLanguage = parseStringFromStream(fileStream);
        int developerYearsOfExperience = parseIntFromStream(fileStream, "experience");
        return std::make_shared<Developer>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            programmingLanguage, developerYearsOfExperience, baseData.employmentRate);
    }
    if (employeeType == "[DESIGNER]") {
        QString designerTool = parseStringFromStream(fileStream);
        int designerNumberOfProjects = parseIntFromStream(fileStream, "number of projects");
        return std::make_shared<Designer>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            designerTool, designerNumberOfProjects, baseData.employmentRate);
    }
    if (employeeType == "[QA]") {
        QString qaTestingType = parseStringFromStream(fileStream);
        int qaBugsFound = parseIntFromStream(fileStream, "bugs found");
        return std::make_shared<QA>(baseData.id, baseData.name, baseData.salary,
                                    baseData.department, qaTestingType,
                                    qaBugsFound, baseData.employmentRate);
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
    // Задачи больше не сохраняются здесь - они сохраняются в отдельном файле
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
    QString projectStatus = parseStringFromStream(fileStream);
    
    QDate projectStartDate = QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);
    QDate projectEndDate = QDate::fromString(parseStringFromStream(fileStream), Qt::ISODate);

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

    Project project(projectId, projectName, projectDescription, projectStatus,
                    projectStartDate, projectEndDate, projectBudget, clientName,
                    estimatedHours);

    return project;
}

void FileManager::saveTasks(const Company& company, const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto projects = company.getAllProjects();
    auto employees = company.getAllEmployees();
    
    // Собираем все задачи с назначениями: projectId, taskId, name, type, estimatedHours, allocatedHours, priority, status, assignments (employeeId, hours)
    std::vector<std::tuple<int, int, QString, QString, int, int, int, QString, std::vector<std::pair<int, int>>>> allTasks;
    
    for (const auto& project : projects) {
        auto tasks = company.getProjectTasks(project.getId());
        for (const auto& task : tasks) {
            // Находим работников, назначенных на эту задачу, и их часы
            // Используем пропорциональное распределение часов работника между задачами проекта
            std::vector<std::pair<int, int>> assignments; // (employeeId, hours)
            
            for (const auto& emp : employees) {
                if (!emp || !emp->getIsActive()) continue;
                if (!emp->isAssignedToProject(project.getId())) continue;
                
                // Если у задачи есть выделенные часы, вычисляем часы работника для этой задачи
                if (task.getAllocatedHours() > 0) {
                    auto projectTasks = company.getProjectTasks(project.getId());
                    int totalAllocatedHours = 0;
                    for (const auto& t : projectTasks) {
                        totalAllocatedHours += t.getAllocatedHours();
                    }
                    
                    if (totalAllocatedHours > 0) {
                        int totalWeeklyHours = emp->getCurrentWeeklyHours();
                        // Пропорционально распределяем часы работника между задачами
                        int taskHours = (task.getAllocatedHours() * totalWeeklyHours) / totalAllocatedHours;
                        if (taskHours > 0) {
                            assignments.push_back(std::make_pair(emp->getId(), taskHours));
                        }
                    }
                }
            }
            
            allTasks.push_back(std::make_tuple(
                project.getId(),
                task.getId(),
                task.getName(),
                task.getType(),
                task.getEstimatedHours(),
                task.getAllocatedHours(),
                task.getPriority(),
                task.getStatus(),
                assignments
            ));
        }
    }
    
    fileStream << allTasks.size() << "\n";
    for (const auto& taskData : allTasks) {
        fileStream << std::get<0>(taskData) << "\n";  // projectId
        fileStream << std::get<1>(taskData) << "\n";  // taskId
        fileStream << std::get<2>(taskData).toStdString() << "\n";  // name
        fileStream << std::get<3>(taskData).toStdString() << "\n";  // type
        fileStream << std::get<4>(taskData) << "\n";  // estimatedHours
        fileStream << std::get<5>(taskData) << "\n";  // allocatedHours
        fileStream << std::get<6>(taskData) << "\n";  // priority
        fileStream << std::get<7>(taskData).toStdString() << "\n";  // status
        
        // Сохраняем назначения (employeeId, hours)
        const std::vector<std::pair<int, int>>& assignments = std::get<8>(taskData);
        fileStream << assignments.size() << "\n";
        for (const auto& assignment : assignments) {
            fileStream << assignment.first << "\n";  // employeeId
            fileStream << assignment.second << "\n";  // hours
        }
    }
    
    fileStream.close();
}

void FileManager::loadTasks(Company& company, const QString& fileName) {
    if (!QFile::exists(fileName)) {
        return;  // Файл не существует, нет задач для загрузки
    }
    
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    std::string lineContent;
    std::getline(fileStream, lineContent);
    int taskCount = 0;
    try {
        taskCount = std::stoi(lineContent);
    } catch (const std::exception&) {
        fileStream.close();
        return;  // Неверный формат, пропускаем
    }

    for (int i = 0; i < taskCount; ++i) {
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
        QString taskName = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        QString taskType = QString::fromStdString(lineContent);

        std::getline(fileStream, lineContent);
        int estimatedHours = 0;
        try {
            estimatedHours = std::stoi(lineContent);
        } catch (const std::exception&) {
            estimatedHours = 0;
        }

        std::getline(fileStream, lineContent);
        int allocatedHours = 0;
        try {
            allocatedHours = std::stoi(lineContent);
        } catch (const std::exception&) {
            allocatedHours = 0;
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

        // Читаем назначения работников на задачу (employeeId, hours)
        std::vector<std::pair<int, int>> assignments;
        if (!fileStream.eof()) {
            std::getline(fileStream, lineContent);
            int assignmentCount = 0;
            try {
                assignmentCount = std::stoi(lineContent);
            } catch (const std::exception&) {
                assignmentCount = 0;
            }
            
            for (int j = 0; j < assignmentCount; ++j) {
                if (fileStream.eof()) break;
                
                // Читаем employeeId
                std::getline(fileStream, lineContent);
                int empId = 0;
                try {
                    empId = std::stoi(lineContent);
                } catch (const std::exception&) {
                    continue;
                }
                
                // Читаем hours
                if (fileStream.eof()) break;
                std::getline(fileStream, lineContent);
                int hours = 0;
                try {
                    hours = std::stoi(lineContent);
                } catch (const std::exception&) {
                    continue;
                }
                
                if (empId > 0 && hours > 0) {
                    assignments.push_back(std::make_pair(empId, hours));
                }
            }
        }

        // Добавляем задачу в проект
        try {
            Task task(taskId, taskName, taskType, estimatedHours, priority);
            task.setStatus(status);
            
            if (assignments.empty()) {
                // Старый формат файла - используем сохраненные allocatedHours
                task.setAllocatedHours(allocatedHours);
            } else {
                // Новый формат - сначала добавляем задачу с нулевыми allocatedHours
                // allocatedHours будут установлены при восстановлении назначений
                task.setAllocatedHours(0);
            }
            
            company.addTaskToProject(projectId, task);
            
            // Восстанавливаем назначения работников на задачу
            // assignEmployeeToTask правильно установит allocatedHours
            for (const auto& assignment : assignments) {
                try {
                    company.assignEmployeeToTask(assignment.first, projectId, taskId, assignment.second);
                } catch (const std::exception&) {
                    // Игнорируем ошибки при восстановлении назначений
                    continue;
                }
            }
        } catch (const std::exception&) {
            // Игнорируем ошибки при добавлении задач
            continue;
        }
    }

    fileStream.close();
}

void FileManager::saveTaskAssignments(const Company& company,
                                     const QString& fileName) {
    std::ofstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto employees = company.getAllEmployees();
    auto projects = company.getAllProjects();
    
    // Собираем все назначения: employeeId, projectId, taskId, hours
    std::vector<std::tuple<int, int, int, int>> assignments;
    
    for (const auto& emp : employees) {
        if (!emp || !emp->getIsActive()) continue;
        
        auto assignedProjectIds = emp->getAssignedProjects();
        int totalWeeklyHours = emp->getCurrentWeeklyHours();
        
        for (int projectId : assignedProjectIds) {
            auto tasks = company.getProjectTasks(projectId);
            int projectTasksWithAllocation = 0;
            int totalAllocatedHours = 0;
            
            // Подсчитываем задачи с выделенными часами
            for (const auto& task : tasks) {
                if (task.getAllocatedHours() > 0) {
                    projectTasksWithAllocation++;
                    totalAllocatedHours += task.getAllocatedHours();
                }
            }
            
            // Распределяем часы работника между задачами проекта
            if (projectTasksWithAllocation > 0 && totalAllocatedHours > 0) {
                int remainingHours = totalWeeklyHours;
                for (const auto& task : tasks) {
                    if (task.getAllocatedHours() > 0 && remainingHours > 0) {
                        // Пропорционально распределяем часы
                        int taskHours = (task.getAllocatedHours() * totalWeeklyHours) / totalAllocatedHours;
                        if (taskHours > remainingHours) {
                            taskHours = remainingHours;
                        }
                        if (taskHours > 0) {
                            assignments.push_back(std::make_tuple(
                                emp->getId(), projectId, task.getId(), taskHours));
                            remainingHours -= taskHours;
                        }
                    }
                }
            }
        }
    }
    
    fileStream << assignments.size() << "\n";
    for (const auto& assignment : assignments) {
        fileStream << std::get<0>(assignment) << "\n";  // employeeId
        fileStream << std::get<1>(assignment) << "\n";  // projectId
        fileStream << std::get<2>(assignment) << "\n";  // taskId
        fileStream << std::get<3>(assignment) << "\n";  // hours
    }
    
    fileStream.close();
}

void FileManager::loadTaskAssignments(Company& company,
                                     const QString& fileName) {
    if (!QFile::exists(fileName)) {
        return;  // Файл не существует, нет назначений для загрузки
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
        return;  // Неверный формат, пропускаем
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

        // Восстанавливаем назначение
        try {
            company.assignEmployeeToTask(employeeId, projectId, taskId, hours);
        } catch (const std::exception&) {
            // Игнорируем ошибки при восстановлении назначений
            continue;
        }
    }

    fileStream.close();
}
