#include "managers/file_manager.h"

#include <QDate>
#include <QFile>
#include <format>
#include <fstream>
#include <iomanip>
#include <memory>
#include <ranges>
#include <set>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <utility>

#include "entities/derived_employees.h"
#include "utils/consts.h"

std::map<int, bool> FileManager::employeeStatusesFromFile;

struct TaskFieldData {
    int projectId = 0;
    int taskId = 0;
    QString taskName;
    QString taskType;
    int estimatedHours = 0;
    int allocatedHours = 0;
    int priority = 0;
    QString phase;
};

struct TaskData {
    int projectId = 0;
    int taskId = 0;
    QString taskName;
    QString taskType;
    int estimatedHours = 0;
    int allocatedHours = 0;
    int priority = 0;
    QString phase;
    std::vector<std::pair<int, int>> assignments;
};

struct ProcessTaskParams {
    const std::vector<std::string>& lines;
    int& lineIndex;
    int taskIndex;
    int taskCount;
    TaskData& taskData;
};

static void updateTaskDataFromField(TaskData& taskData, const TaskFieldData& fieldData) {
    if (fieldData.projectId > 0) {
        taskData.projectId = fieldData.projectId;
    }
    if (fieldData.taskId > 0) {
        taskData.taskId = fieldData.taskId;
    }
    if (!fieldData.taskName.isEmpty()) {
        taskData.taskName = fieldData.taskName;
    }
    if (!fieldData.taskType.isEmpty()) {
        taskData.taskType = fieldData.taskType;
    }
    if (fieldData.estimatedHours >= 0) {
        taskData.estimatedHours = fieldData.estimatedHours;
    }
    if (fieldData.allocatedHours >= 0) {
        taskData.allocatedHours = fieldData.allocatedHours;
    }
    if (fieldData.priority >= 0) {
        taskData.priority = fieldData.priority;
    }
    if (!fieldData.phase.isEmpty()) {
        taskData.phase = fieldData.phase;
    }
}

static void parseTaskField(std::string_view lineContent, TaskFieldData& data) {
    if (lineContent.find("PROJECT_ID:") == 0) {
        std::string subStr(lineContent.substr(11));
        data.projectId = std::stoi(subStr);
    } else if (lineContent.find("TASK_ID:") == 0) {
        std::string subStr(lineContent.substr(8));
        data.taskId = std::stoi(subStr);
    } else if (lineContent.find("NAME:") == 0) {
        data.taskName =
            QString::fromStdString(std::string(lineContent.substr(5)));
    } else if (lineContent.find("TYPE:") == 0) {
        data.taskType =
            QString::fromStdString(std::string(lineContent.substr(5)));
    } else if (lineContent.find("ESTIMATED_HOURS:") == 0) {
        std::string subStr(lineContent.substr(16));
        data.estimatedHours = std::stoi(subStr);
    } else if (lineContent.find("ALLOCATED_HOURS:") == 0) {
        std::string subStr(lineContent.substr(16));
        data.allocatedHours = std::stoi(subStr);
    } else if (lineContent.find("PRIORITY:") == 0) {
        std::string subStr(lineContent.substr(9));
        data.priority = std::stoi(subStr);
    } else if (lineContent.find("PHASE:") == 0) {
        data.phase = QString::fromStdString(std::string(lineContent.substr(6)));
    }
}

static void parseAssignmentLine(std::string_view lineContent,
                         std::vector<std::pair<int, int>>& assignments,
                         int& assignmentsRead, int assignmentsCount,
                         bool& readingAssignments) {
    if (assignmentsRead >= assignmentsCount || assignments.size() >= static_cast<size_t>(kMaxSmallAssignments)) {
        readingAssignments = false;
        return;
    }
    
    size_t empPos = lineContent.find("EMPLOYEE_ID:");
    size_t hoursPos = lineContent.find("HOURS:");
    
    if (empPos == std::string_view::npos ||
        hoursPos == std::string_view::npos) {
        return;
    }
    
    int employeeId = 0;
    int hours = 0;
    try {
        std::string empStr(lineContent.substr(empPos + 12));
        std::string hoursStr(lineContent.substr(hoursPos + 6));
        employeeId = std::stoi(empStr);
        hours = std::stoi(hoursStr);
    } catch (const std::invalid_argument&) {
        return;
    } catch (const std::out_of_range&) {
        return;
    }
    
    if (employeeId > 0 && hours > 0) {
        assignments.emplace_back(employeeId, hours);
        assignmentsRead++;
        if (assignmentsRead >= assignmentsCount) {
            readingAssignments = false;
        }
    }
}

static void collectTaskAssignments(
    const Company& company, int projectId, int taskId,
                                   const std::vector<std::shared_ptr<Employee>>& employees,
                                   std::vector<std::pair<int, int>>& assignments) {
    if (assignments.size() >= static_cast<size_t>(kMaxSmallAssignments)) {
        return;
    }
    
    for (const auto& emp : employees) {
        if (!emp) continue;
        
        if (assignments.size() >= static_cast<size_t>(kMaxSmallAssignments)) {
            break;
        }

        auto taskHours =
            company.getEmployeeHours(emp->getId(), projectId, taskId);
        
        if (taskHours <= 0) {
            continue;
        }
        
        assignments.push_back(std::make_pair(emp->getId(), taskHours));
    }
}

static void processTaskAssignmentLine(
    const std::string& lineContent,
    std::vector<std::pair<int, int>>& assignments, int& assignmentsRead,
    int& assignmentsCount, bool& readingAssignments) {
    if (lineContent.find("ASSIGNMENTS_COUNT:") == 0) {
        try {
            int parsedCount = std::stoi(lineContent.substr(18));
            if (parsedCount >= 0 && parsedCount <= kMaxSmallAssignments) {
                assignmentsCount = parsedCount;
            readingAssignments = assignmentsCount > 0;
            assignmentsRead = 0;
            } else {
                assignmentsCount = 0;
                readingAssignments = false;
            }
        } catch (const std::invalid_argument&) {
            assignmentsCount = 0;
        } catch (const std::out_of_range&) {
            assignmentsCount = 0;
        }
    }
    
    if (readingAssignments && lineContent.find("  [") == 0) {
        parseAssignmentLine(lineContent, assignments, assignmentsRead,
                           assignmentsCount, readingAssignments);
    }
}

static void processTaskAssignments(
    Company& company, int projectId, int taskId,
                                  const std::vector<std::pair<int, int>>& assignments) {
    for (const auto& assignment : assignments) {
        const auto& [empId, hours] = assignment;
        try {
            company.assignEmployeeToTask(empId, projectId, taskId, hours);
        } catch (const CompanyException&) {
            try {
                company.restoreTaskAssignment(empId, projectId, taskId, hours);
            } catch (const CompanyException&) {
                continue;
            }
        }
    }
}

struct AddTaskParams {
    int projectId;
    int taskId;
    QString taskName;
    QString taskType;
    int estimatedHours;
    int allocatedHours;
    int priority;
    QString phase;
    std::vector<std::pair<int, int>> assignments;
};

static void addTaskToCompany(Company& company, const AddTaskParams& params) {
    Task task(params.taskId, params.taskName, params.taskType,
              params.estimatedHours, params.priority);
    task.setPhase(params.phase);
    task.setAllocatedHours(params.allocatedHours);
    company.addTaskToProject(params.projectId, task);
    processTaskAssignments(company, params.projectId, params.taskId,
                           params.assignments);
}

static std::vector<std::string> readFileLines(const QString& fileName) {
    std::ifstream fileStream(fileName.toStdString());
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for reading: " + fileName);
    }

    fileStream.seekg(0, std::ios::end);
    std::streampos fileSize = fileStream.tellg();
    if (fileSize == 0) {
        fileStream.close();
        return {};
    }
    
    if (const auto maxFileSize = std::streampos(kMaxFileSizeBytes); fileSize > maxFileSize) {
        fileStream.close();
        throw FileManagerException("File too large: " + fileName);
    }
    
    fileStream.seekg(0, std::ios::beg);

    std::vector<std::string> lines;
    lines.reserve(static_cast<size_t>(kReserveCapacity));
    std::string lineContent;
    size_t lineCount = 0;
    
    while (std::getline(fileStream, lineContent) && lineCount < static_cast<size_t>(kMaxLines)) {
        lines.push_back(lineContent);
        lineCount++;
    }
    fileStream.close();
    
    return lines;
}

static bool findTaskHeader(const std::vector<std::string>& lines,
                          int& lastHeaderIndex, int& taskCount) {
    for (int i = lines.size() - 1; i >= 0; --i) {
        if (lines[i].find("TASKS_COUNT:") == 0) {
            try {
                int parsedCount = std::stoi(lines[i].substr(12));
                if (parsedCount >= 0 && parsedCount <= kMaxTasks) {
                    taskCount = parsedCount;
                lastHeaderIndex = i;
                return true;
                }
            } catch (const std::invalid_argument&) {
                continue;
            } catch (const std::out_of_range&) {
                continue;
            }
        }
    }
    return false;
}

static int calculateStartIndex(const std::vector<std::string>& lines,
                              int lastHeaderIndex) {
    int startIndex = lastHeaderIndex + 2;
    if (startIndex < static_cast<int>(lines.size()) &&
        lines[startIndex] == "---") {
        startIndex++;
    }
    return startIndex;
}

static bool processTask(const ProcessTaskParams& params) {
    while (params.lineIndex < static_cast<int>(params.lines.size()) && 
           params.lines[params.lineIndex].find("[TASK") != 0) {
        params.lineIndex++;
    }
    
    if (params.lineIndex >= static_cast<int>(params.lines.size())) {
        return false;
    }
    
    params.lineIndex++;
    
    bool readingAssignments = false;
    int assignmentsCount = 0;
    int assignmentsRead = 0;
    
    while (params.lineIndex < static_cast<int>(params.lines.size())) {
        std::string lineContent = params.lines[params.lineIndex];
        
        if (lineContent.empty() || lineContent == "---") {
            params.lineIndex++;
            continue;
        }
        
        if (lineContent.find("[TASK") == 0 &&
            params.taskIndex < params.taskCount - 1) {
            break;
        }
        
        TaskFieldData fieldData;
        parseTaskField(lineContent, fieldData);
        updateTaskDataFromField(params.taskData, fieldData);
        
        processTaskAssignmentLine(lineContent, params.taskData.assignments,
                                  assignmentsRead, assignmentsCount,
                                  readingAssignments);
        
        params.lineIndex++;
    }
    
    return true;
}

static bool isValidTaskData(const TaskData& taskData) {
    return taskData.projectId > 0 && taskData.taskId > 0 && !taskData.taskName.isEmpty();
}

static bool taskExistsInProject(const Company& company, int projectId, int taskId) {
    auto existingTasks = company.getProjectTasks(projectId);
    return std::ranges::contains(existingTasks, taskId, &Task::getId);
}

static void addTaskWithDeadlineException(Company& company, const TaskData& taskData) {
    Project* project = company.getProject(taskData.projectId);
    if (project) {
        Task task(taskData.taskId, taskData.taskName, taskData.taskType,
                  taskData.estimatedHours, taskData.priority);
        task.setPhase(taskData.phase);
        task.setAllocatedHours(taskData.allocatedHours);
        project->getTasks().push_back(task);
        project->recomputeTotalsFromTasks();
        processTaskAssignments(company, taskData.projectId, taskData.taskId,
                               taskData.assignments);
    }
}

static AddTaskParams createAddTaskParams(const TaskData& taskData) {
    AddTaskParams addParams;
    addParams.projectId = taskData.projectId;
    addParams.taskId = taskData.taskId;
    addParams.taskName = taskData.taskName;
    addParams.taskType = taskData.taskType;
    addParams.estimatedHours = taskData.estimatedHours;
    addParams.allocatedHours = taskData.allocatedHours;
    addParams.priority = taskData.priority;
    addParams.phase = taskData.phase;
    addParams.assignments = taskData.assignments;
    return addParams;
}

static bool handleProjectException(Company& company, const ProjectException& e, const TaskData& taskData) {
    QString errorMsg = e.what();
    if (!errorMsg.contains("exceed deadline")) {
        return false;
    }
    try {
        addTaskWithDeadlineException(company, taskData);
        return true;
    } catch (const ProjectException&) {
        return false;
    } catch (const CompanyException&) {
        return false;
    } catch (const TaskException&) {
        return false;
    }
}

static bool tryAddTaskToCompany(Company& company, const TaskData& taskData) {
    try {
        AddTaskParams addParams = createAddTaskParams(taskData);
        addTaskToCompany(company, addParams);
        return true;
    } catch (const ProjectException& e) {
        return handleProjectException(company, e, taskData);
    } catch (const CompanyException&) {
        return false;
    } catch (const TaskException&) {
        return false;
    }
}

static void processTasks(Company& company,
                                 const std::vector<std::string>& lines,
                                int startIndex, int taskCount) {
    int lineIndex = startIndex;
    for (int i = 0; i < taskCount && lineIndex < static_cast<int>(lines.size());
         ++i) {
        TaskData taskData;
        if (ProcessTaskParams params{lines, lineIndex, i, taskCount,
                                             taskData};
            !processTask(params)) {
            break;
        }
        
        if (!isValidTaskData(taskData)) {
            continue;
        }

        if (const Project* project = company.getProject(taskData.projectId);
            !project) {
            continue;
        }

        if (taskExistsInProject(company, taskData.projectId, taskData.taskId)) {
            continue;
        }
        
        tryAddTaskToCompany(company, taskData);
    }
}

static void collectEmployeeTaskAssignments(
    const Company& company, int employeeId,
                                          const std::vector<Project>& projects,
                                          std::vector<std::tuple<int, int, int, int>>& assignments) {
    if (assignments.size() >= static_cast<size_t>(kMaxLargeAssignments)) {
        return;
    }
    
    for (const auto& project : projects) {
        if (assignments.size() >= static_cast<size_t>(kMaxLargeAssignments)) {
            break;
        }
        
        auto projectId = project.getId();
        auto tasks = company.getProjectTasks(projectId);
        
        for (const auto& task : tasks) {
            if (assignments.size() >= static_cast<size_t>(kMaxLargeAssignments)) {
                break;
            }
            
            int taskHours = company.getEmployeeHours(employeeId, projectId,
                                                         task.getId());
            
            if (taskHours <= 0) {
                continue;
            }
            
            assignments.push_back(std::make_tuple(employeeId, projectId,
                                                  task.getId(), taskHours));
        }
    }
}

template <typename T>
T parseNumericFromStream(std::ifstream& fileStream, const QString& fieldName) {
    std::string lineContent;
    std::getline(fileStream, lineContent);
    try {
        if constexpr (std::is_same_v<T, int>) {
        return std::stoi(lineContent);
        } else if constexpr (std::is_same_v<T, double>) {
            return std::stod(lineContent);
        }
    } catch (const std::exception&) {
        throw FileManagerException(
            QString("Invalid %1 format in file").arg(fieldName));
    }
}

int FileManager::parseIntFromStream(std::ifstream& fileStream,
                                    const QString& fieldName) {
    return parseNumericFromStream<int>(fileStream, fieldName);
}

double FileManager::parseDoubleFromStream(std::ifstream& fileStream,
                                          const QString& fieldName) {
    return parseNumericFromStream<double>(fileStream, fieldName);
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
    } catch (const std::invalid_argument&) {
        fileStream.seekg(currentPos);
    } catch (const std::out_of_range&) {
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
        fileStream << std::format("{:.1f}\n",
                                  developer->getYearsOfExperience());
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
    if (std::string lineContent; std::getline(fileStream, lineContent)) {
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
        saveProjectToStream(project, fileStream);
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

        ProjectParams projectParams{
            projectId,     projectName,      projectDescription,
            projectPhase,  projectStartDate, projectEndDate,
            projectBudget, clientName,       0};
        Project project(projectParams);
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
    std::ofstream fileStream(fileName.toStdString(),
                             std::ios::out | std::ios::trunc);
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open file for writing: " + fileName);
    }

    auto employees = company.getAllEmployees();
    
    if (employees.size() > static_cast<size_t>(kMaxEmployees)) {
        fileStream.close();
        throw FileManagerException("Too many employees to save (max: " + 
                                   QString::number(kMaxEmployees) + "): " + 
                                   fileName);
    }
    
    fileStream << employees.size() << "\n";
    
    if (!fileStream.good()) {
        fileStream.close();
        throw FileManagerException("Error writing employee count to file: " +
                                   fileName);
    }

    for (const auto& employee : employees) {
        if (!employee) continue;
        saveEmployeeToStream(employee, fileStream);
        if (!fileStream.good()) {
            fileStream.close();
            throw FileManagerException("Error writing employee data to file: " +
                                       fileName);
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

    if (employeeCount < 0 || employeeCount > kMaxEmployees) {
        fileStream.close();
        throw FileManagerException("Invalid employee count: " + QString::number(employeeCount));
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
        employee = std::make_shared<QA>(
            baseData.id, baseData.name, baseData.salary, baseData.department,
            qaTestingType, qaBugsFound, baseData.employmentRate);
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
    
    if (projects.size() > static_cast<size_t>(kMaxProjects)) {
        fileStream.close();
        throw FileManagerException("Too many projects to save (max: " + 
                                   QString::number(kMaxProjects) + "): " + 
                                   fileName);
    }
    
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
        fileStream.close();
        throw FileManagerException("Invalid project count format in file");
    }

    if (projectCount < 0 || projectCount > kMaxProjects) {
        fileStream.close();
        throw FileManagerException("Invalid project count: " + QString::number(projectCount));
    }

    for (int i = 0; i < projectCount; ++i) {
        Project project = loadProjectFromStream(fileStream);
        if (const Project* existing = company.getProject(project.getId());
            existing != nullptr) {
            continue;
        }
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

    ProjectParams projectParams{
        projectId,     projectName,      projectDescription,
        projectPhase,  projectStartDate, projectEndDate,
        projectBudget, clientName,       estimatedHours};
    Project project(projectParams);

    return project;
}

void FileManager::saveTasks(const Company& company, const QString& fileName) {
    QString tempFileName = fileName + ".tmp";
    std::ofstream fileStream(tempFileName.toStdString(),
                             std::ios::out | std::ios::trunc);
    if (!fileStream.is_open()) {
        throw FileManagerException("Cannot open temporary file for writing: " +
                                   tempFileName);
    }

    auto projects = company.getAllProjects();
    auto employees = company.getAllEmployees();

    size_t totalTasksCount = 0;
    for (const auto& project : projects) {
        auto tasks = company.getProjectTasks(project.getId());
        totalTasksCount += tasks.size();
        if (totalTasksCount > static_cast<size_t>(kMaxTasks)) {
            throw FileManagerException("Too many tasks to save (max: " + 
                                       QString::number(kMaxTasks) + "): " + 
                                       fileName);
        }
    }

    std::vector<std::tuple<int, int, QString, QString, int, int, int, QString,
                           std::vector<std::pair<int, int>>>>
        allTasks;
    allTasks.reserve(std::min(totalTasksCount, static_cast<size_t>(kMaxTasks)));

    for (const auto& project : projects) {
        auto tasks = company.getProjectTasks(project.getId());
        for (const auto& task : tasks) {
            std::vector<std::pair<int, int>> assignments;
            collectTaskAssignments(company, project.getId(), task.getId(),
                                   employees, assignments);

            allTasks.emplace_back(project.getId(), task.getId(), task.getName(),
                                  task.getType(), task.getEstimatedHours(),
                                  task.getAllocatedHours(), task.getPriority(),
                                  task.getPhase(), assignments);
        }
    }

    fileStream << "TASKS_COUNT:" << allTasks.size() << "\n";
    fileStream << "FORMAT_VERSION:2\n";
    fileStream << "---\n";
    
    if (!fileStream.good()) {
        fileStream.close();
        QFile::remove(tempFileName);
        throw FileManagerException("Error writing task header to file: " +
                                   fileName);
    }
    
    for (size_t i = 0; i < allTasks.size(); ++i) {
        const auto& [projectId, taskId, taskName, taskType, estimatedHours,
                     allocatedHours, priority, phase, assignments] =
            allTasks[i];
        
        fileStream << "\n[TASK " << (i + 1) << "]\n";
        fileStream << "PROJECT_ID:" << projectId << "\n";
        fileStream << "TASK_ID:" << taskId << "\n";
        fileStream << "NAME:" << taskName.toStdString() << "\n";
        fileStream << "TYPE:" << taskType.toStdString() << "\n";
        fileStream << "ESTIMATED_HOURS:" << estimatedHours << "\n";
        fileStream << "ALLOCATED_HOURS:" << allocatedHours << "\n";
        fileStream << "PRIORITY:" << priority << "\n";
        fileStream << "PHASE:" << phase.toStdString() << "\n";
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
            throw FileManagerException("Error writing task data to file: " +
                                       fileName);
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

    std::vector<std::string> lines = readFileLines(fileName);
    if (lines.empty()) {
        return;
    }
    
    int lastHeaderIndex = -1;
    int taskCount = 0;
    if (!findTaskHeader(lines, lastHeaderIndex, taskCount) || taskCount == 0) {
        return;
    }

    if (taskCount < 0 || taskCount > kMaxTasks) {
        return;
    }
    
    int startIndex = calculateStartIndex(lines, lastHeaderIndex);
    processTasks(company, lines, startIndex, taskCount);
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
    assignments.reserve(std::min(static_cast<size_t>(kMaxLargeAssignments), static_cast<size_t>(kMaxSmallAssignments)));

    for (const auto& emp : employees) {
        if (!emp) continue;
        
        if (assignments.size() >= static_cast<size_t>(kMaxLargeAssignments)) {
            break;
        }
        
        collectEmployeeTaskAssignments(company, emp->getId(), projects,
                                       assignments);
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
        throw FileManagerException("Error writing task assignments to file: " +
                                   fileName);
    }

    fileStream.close();
}

static bool parseAssignmentId(const std::string& lineContent, int& id) {
    try {
        id = std::stoi(lineContent);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

static bool loadSingleAssignment(Company& company, std::ifstream& fileStream) {
    std::string lineContent;
    int employeeId = 0;
    int projectId = 0;
    int taskId = 0;
    int hours = 0;

    std::getline(fileStream, lineContent);
    if (!parseAssignmentId(lineContent, employeeId)) {
        return false;
    }

    std::getline(fileStream, lineContent);
    if (!parseAssignmentId(lineContent, projectId)) {
        return false;
    }

    std::getline(fileStream, lineContent);
    if (!parseAssignmentId(lineContent, taskId)) {
        return false;
    }

    std::getline(fileStream, lineContent);
    if (!parseAssignmentId(lineContent, hours)) {
        return false;
    }

    try {
        company.restoreTaskAssignment(employeeId, projectId, taskId, hours);
        return true;
    } catch (const CompanyException&) {
        try {
            company.assignEmployeeToTask(employeeId, projectId, taskId, hours);
            return true;
        } catch (const CompanyException&) {
            return false;
        }
    }
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
    } catch (const std::invalid_argument&) {
        fileStream.close();
        return;
    } catch (const std::out_of_range&) {
        fileStream.close();
        return;
    }

    if (assignmentCount < 0 || assignmentCount > kMaxAssignmentCount) {
        fileStream.close();
        return;
    }

    for (int i = 0; i < assignmentCount; ++i) {
        if (!loadSingleAssignment(company, fileStream)) {
            continue;
        }
    }

    fileStream.close();
    
    company.fixTaskAssignmentsToCapacity();
    
    company.recalculateTaskAllocatedHours();
}
