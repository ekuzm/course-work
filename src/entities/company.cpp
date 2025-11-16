#include "entities/company.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <ranges>
#include <tuple>
#include <utility>

#include <QLoggingCategory>

#include "utils/consts.h"
#include "utils/container_utils.h"
#include "exceptions/exceptions.h"

Q_LOGGING_CATEGORY(companyExceptions, "company.exceptions")

static double calculateHourlyRate(double monthlySalary) {
    if (kHoursPerMonth <= 0) return 0.0;
    return monthlySalary / kHoursPerMonth;
}

static double calculateEmployeeCost(double monthlySalary, int hours) {
    return calculateHourlyRate(monthlySalary) * hours;
}

namespace {
    
    struct BuildEmployeePoolParams {
        const std::vector<std::shared_ptr<Employee>>& employeesList;
        const QString& projectPhase;
        const QString& taskType;
        double projectBudget;
        double maxAffordableHourlyRate;
        double projectEstimatedHours;
        const std::map<int, int>& employeeUsage;
    };
    
    struct AssignEmployeeToTaskParams {
        int projectId;
        int& remaining;
        double& currentEmployeeCosts;
        double& remainingBudget;
        std::map<int, int>& employeeUsage;
        std::map<std::tuple<int, int, int>, int>& taskAssignments;
    };
    
    void buildActiveEmployeesList(const std::vector<std::shared_ptr<Employee>>& allEmployees,
                                  std::vector<std::shared_ptr<Employee>>& employeesList) {
        for (const auto& emp : allEmployees) {
            if (emp && emp->getIsActive()) {
                employeesList.emplace_back(emp);
            }
        }
    }
    
    int compareEmployeesForSorting(const std::shared_ptr<Employee>& a,
                                    const std::shared_ptr<Employee>& b,
                                    const std::map<int, int>& employeeUsage) {
        if (!a || !b) return 0;

        double hourlyRateA = calculateHourlyRate(a->getSalary());
        double hourlyRateB = calculateHourlyRate(b->getSalary());

        int usedA = 0;
        if (auto itA = employeeUsage.find(a->getId()); itA != employeeUsage.end()) {
            const auto& [key, hours] = *itA;
            usedA = hours;
        }
        int usedB = 0;
        if (auto itB = employeeUsage.find(b->getId()); itB != employeeUsage.end()) {
            const auto& [key, hours] = *itB;
            usedB = hours;
        }

        int availA = a->getAvailableHours() - usedA;
        int availB = b->getAvailableHours() - usedB;

        double rateDiff = hourlyRateA - hourlyRateB;
        if (rateDiff < -0.01) {
            return -1;
        }
        if (rateDiff > 0.01) {
            return 1;
        }
        if (availA > availB) {
            return -1;
        }
        if (availA < availB) {
            return 1;
        }
        return 0;
    }
    
    [[noreturn]] void throwAvailabilityException(const std::shared_ptr<Employee>& employee, int toAssign) {
        auto availableHours = employee->getAvailableHours();
        auto currentHours = employee->getCurrentWeeklyHours();
        auto capacity = employee->getWeeklyHoursCapacity();
        throw CompanyException(
            QString("Not enough available hours to assign %1 hours.\n\n"
                    "Employee: %2\n"
                    "Weekly capacity: %3h\n"
                    "Currently used: %4h (across all projects)\n"
                    "Available: %5h\n"
                    "Requested: %6h\n\n"
                    "The employee cannot exceed their weekly capacity "
                    "of %3h.")
                .arg(toAssign)
                .arg(employee->getName())
                .arg(capacity)
                .arg(currentHours)
                .arg(availableHours)
                .arg(toAssign));
    }
    
    bool roleMatchesSDLCStage(const QString& employeePosition,
                              const QString& projectPhase) {
        if (projectPhase == "Analysis" || projectPhase == "Planning") {
            return employeePosition == "Manager";
        }
        if (projectPhase == "Design") {
            return employeePosition == "Designer";
        }
        if (projectPhase == "Development") {
            return employeePosition == "Developer";
        }
        if (projectPhase == "Testing") {
            return employeePosition == "QA";
        }
        return true;
    }
    
    void updateTaskAndProjectCosts(std::shared_ptr<Project> projPtr,
                                   int taskId,
                                   int oldHours,
                                   int newHours,
                                   std::shared_ptr<Employee> employee) {
        if (!projPtr || !employee) return;
        
        std::vector<Task>& tasks = projPtr->getTasks();
        for (auto& task : tasks) {
            if (task.getId() == taskId) {
                int currentAllocated = task.getAllocatedHours();
                int diff = newHours - oldHours;
                int newAllocated = currentAllocated + diff;
                if (newAllocated < 0) {
                    newAllocated = 0;
                }
                task.setAllocatedHours(newAllocated);
                
                
                if (const auto costDiff = calculateEmployeeCost(employee->getSalary(), newHours) -
                                calculateEmployeeCost(employee->getSalary(), oldHours); costDiff < 0) {
                    projPtr->removeEmployeeCost(-costDiff);
                } else {
                    projPtr->addEmployeeCost(costDiff);
                }
            }
        }
    }
    
    QString getRequiredEmployeeType(const QString& taskType) {
        static const std::map<QString, QString> typeMapping = {
            {"Management", "Manager"},
            {"Development", "Developer"},
            {"Design", "Designer"},
            {"QA", "QA"}
        };
        if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
            return it->second;
        }
        return "Unknown";
    }
    
    void validateBudgetConstraints(const std::shared_ptr<Employee>& employee,
                                   const std::shared_ptr<Project>& projPtr,
                                   int toAssign,
                                   double employeeHourlyRate,
                                   double assignmentCost) {
        if (employee->getSalary() > projPtr->getBudget()) {
            throw CompanyException(
                QString(
                    "Cannot assign employee: monthly salary exceeds "
                    "project budget.\n"
                    "Employee monthly salary: $%1\n"
                    "Project budget: $%2\n"
                    "Employee is too expensive for this project budget.")
                    .arg(employee->getSalary(), 0, 'f', 2)
                    .arg(projPtr->getBudget(), 0, 'f', 2));
        }

        double projectEstimatedHours = projPtr->getEstimatedHours();
        if (projectEstimatedHours > 0) {
            auto averageBudgetPerHour = projPtr->getBudget() / projectEstimatedHours;
            auto maxAffordableHourlyRate = averageBudgetPerHour * kMaxAffordableHourlyRateMultiplier;

            if (employeeHourlyRate > maxAffordableHourlyRate) {
                throw CompanyException(
                    QString("Employee hourly rate is too high for this "
                            "project.\n"
                            "Employee hourly rate: $%1/hour\n"
                            "Max affordable hourly rate (70%% of budget "
                            "avg): $%2/hour\n"
                            "Project budget: $%3\n"
                            "Project estimated hours: %4h\n"
                            "Average budget per hour: $%5/hour")
                        .arg(employeeHourlyRate, 0, 'f', 2)
                        .arg(maxAffordableHourlyRate, 0, 'f', 2)
                        .arg(projPtr->getBudget(), 0, 'f', 2)
                        .arg(projectEstimatedHours)
                        .arg(averageBudgetPerHour, 0, 'f', 2));
            }
        }

        auto currentEmployeeCosts = projPtr->getEmployeeCosts();
        auto remainingBudget = projPtr->getBudget() - currentEmployeeCosts;
        if (currentEmployeeCosts + assignmentCost > projPtr->getBudget()) {
            throw CompanyException(
                QString("Cannot assign employee: cost would exceed project "
                        "budget.\n"
                        "Employee hourly rate: $%1/hour\n"
                        "Assignment cost (%2h): $%3\n"
                        "Current employee costs: $%4\n"
                        "Project budget: $%5\n"
                        "Remaining budget: $%6")
                    .arg(employeeHourlyRate, 0, 'f', 2)
                    .arg(toAssign)
                    .arg(assignmentCost, 0, 'f', 2)
                    .arg(currentEmployeeCosts, 0, 'f', 2)
                    .arg(projPtr->getBudget(), 0, 'f', 2)
                    .arg(remainingBudget, 0, 'f', 2));
        }
    }
}

static bool matchesEmployeeId(const std::shared_ptr<Employee>& employee,
                              int employeeId) {
    return employee && employee->getId() == employeeId;
}

static bool matchesProjectId(const std::shared_ptr<Project>& project,
                             int projectId) {
    return project && project->getId() == projectId;
}

static bool isEmployeeActive(const std::shared_ptr<Employee>& employee) {
    return employee && employee->getIsActive();
}

void EmployeeContainer::add(std::shared_ptr<Employee> employee) {
    employees.emplace_back(employee);
}

void EmployeeContainer::remove(int employeeId) {
    for (size_t i = 0; i < employees.size(); ++i) {
        if (matchesEmployeeId(employees[i], employeeId)) {
            employees.erase(employees.begin() + i);
            break;
        }
    }
}

std::shared_ptr<Employee> EmployeeContainer::find(int employeeId) const {
    for (const auto& emp : employees) {
        if (matchesEmployeeId(emp, employeeId)) {
            return emp;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Employee>> EmployeeContainer::getAll() const {
    return employees;
}

size_t EmployeeContainer::size() const { return employees.size(); }

void ProjectContainer::add(std::shared_ptr<Project> project) {
    projects.emplace_back(project);
}

void ProjectContainer::remove(int projectId) {
    for (size_t i = 0; i < projects.size(); ++i) {
        if (matchesProjectId(projects[i], projectId)) {
            projects.erase(projects.begin() + i);
            break;
        }
    }
}

std::shared_ptr<Project> ProjectContainer::find(int projectId) const {
    for (const auto& proj : projects) {
        if (matchesProjectId(proj, projectId)) {
            return proj;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Project>> ProjectContainer::getAll() const {
    return projects;
}

size_t ProjectContainer::size() const { return projects.size(); }

QString Company::getName() const { return name; }

QString Company::getIndustry() const { return industry; }

QString Company::getLocation() const { return location; }

int Company::getFoundedYear() const { return foundedYear; }

Company::Company(QString companyName, QString companyIndustry,
                 QString companyLocation, int companyFoundedYear)
    : name(std::move(companyName)),
      industry(std::move(companyIndustry)),
      location(std::move(companyLocation)),
      foundedYear(companyFoundedYear),
      employees{},
      projects{} {}

void Company::addEmployee(std::shared_ptr<Employee> employee) {
    std::shared_ptr<Employee> existing = getEmployee(employee->getId());
    if (existing) {
        throw CompanyException("Employee with this ID already exists");
    }
    employees.add(employee);
}

void Company::removeEmployee(int employeeId) {
    std::shared_ptr<Employee> employee = employees.find(employeeId);
    if (employee) {
        const std::vector<int>& assignedProjects =
            employee->getAssignedProjects();
        for (int projectId : assignedProjects) {
            std::shared_ptr<Project> projPtr = projects.find(projectId);
            if (projPtr) {
                auto projectTasks = getProjectTasks(projectId);
                for (const auto& task : projectTasks) {
                    auto key = std::make_tuple(employeeId, projectId, task.getId());
                    taskAssignments.erase(key);
                }
                projPtr->recomputeTotalsFromTasks();
            }
        }
    }

    auto it = taskAssignments.begin();
    while (it != taskAssignments.end()) {
        const auto& [key, hours] = *it;
        const auto& [empId, projectId, taskId] = key;
        if (empId == employeeId) {
            it = taskAssignments.erase(it);
        } else {
            ++it;
        }
    }

    employees.remove(employeeId);
}

std::shared_ptr<Employee> Company::getEmployee(int employeeId) const {
    return employees.find(employeeId);
}

std::vector<std::shared_ptr<Employee>> Company::getAllEmployees() const {
    return employees.getAll();
}

void Company::addProject(const Project& project) {
    const Project* existing = getProject(project.getId());
    if (existing != nullptr) {
        throw CompanyException("Project with this ID already exists");
    }
    projects.add(std::make_shared<Project>(project));
}

void Company::removeProject(int projectId) {
    auto it = taskAssignments.begin();
    while (it != taskAssignments.end()) {
        const auto& [key, hours] = *it;
        const auto& [empId, projId, taskId] = key;
        if (projId == projectId) {
            it = taskAssignments.erase(it);
        } else {
            ++it;
        }
    }
    projects.remove(projectId);
}

const Project* Company::getProject(int projectId) const {
    std::shared_ptr<Project> result = projects.find(projectId);
    if (result) {
        return result.get();
    }
    return nullptr;
}

Project* Company::getProject(int projectId) {
    if (std::shared_ptr<Project> result = projects.find(projectId); result) {
        return result.get();
    }
    return nullptr;
}

std::vector<Project> Company::getAllProjects() const {
    std::vector<Project> projectList;
    auto allProjects = projects.getAll();
    for (const auto& proj : allProjects) {
        if (proj) {
            projectList.emplace_back(*proj);
        }
    }
    return projectList;
}

int Company::getEmployeeCount() const {
    return static_cast<int>(employees.size());
}

int Company::getProjectCount() const {
    return static_cast<int>(projects.size());
}

double Company::getTotalSalaries() const {
    auto allEmployees = employees.getAll();
    size_t activeCount = 0;
    for (const auto& emp : allEmployees) {
        if (isEmployeeActive(emp)) {
            activeCount++;
        }
    }

    double total = 0.0;
    for (const auto& emp : allEmployees) {
        if (emp) {
            total += emp->getSalary();
        }
    }
    return total;
}

double Company::getTotalBudget() const {
    double total = 0.0;
    auto allProjects = projects.getAll();
    for (const auto& proj : allProjects) {
        if (proj) {
            total += proj->getBudget();
        }
    }
    return total;
}

QString Company::getCompanyInfo() const {
    return QString(
               "Company: %1\nIndustry: %2\nLocation: %3\nFounded: "
               "%4\nEmployees: %5\nProjects: %6")
        .arg(name)
        .arg(industry)
        .arg(location)
        .arg(foundedYear)
        .arg(getEmployeeCount())
        .arg(getProjectCount());
}

void Company::addTaskToProject(int projectId, const Task& task) {
    std::shared_ptr<Project> proj = projects.find(projectId);
    if (proj) {
        proj->addTask(task);
        return;
    }
    throw CompanyException("Project not found");
}

std::vector<Task> Company::getProjectTasks(int projectId) const {
    std::shared_ptr<Project> proj = projects.find(projectId);
    if (proj) {
        return proj->getTasks();
    }
    return {};
}

namespace {
    std::pair<int, double> calculateTaskAllocatedHours(
        int projectId, int taskId,
        const std::vector<std::shared_ptr<Employee>>& allEmployees,
        const std::map<std::tuple<int, int, int>, int>& taskAssignments) {
        int totalAllocated = 0;
        double taskCost = 0.0;
        
        for (const auto& employee : allEmployees) {
            if (!employee || !employee->isAssignedToProject(projectId)) {
                continue;
            }
            
            auto key = std::make_tuple(employee->getId(), projectId, taskId);
            if (const auto assignmentIt = taskAssignments.find(key);
                assignmentIt != taskAssignments.end()) {
                const auto& [foundKey, hours] = *assignmentIt;
                totalAllocated += hours;
                taskCost += calculateEmployeeCost(employee->getSalary(), hours);
            }
        }
        
        return {totalAllocated, taskCost};
    }
    
    void reduceExcessHours(
        std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        int& excess,
        int& totalScaledHours) {
        std::ranges::sort(assignmentsData, [](const auto& a, const auto& b) {
            const auto& [projectIdA, taskIdA, oldHoursA, adjustedHoursA] = a;
            const auto& [projectIdB, taskIdB, oldHoursB, adjustedHoursB] = b;
            return adjustedHoursA > adjustedHoursB;
        });
        
        for (auto& assignment : assignmentsData) {
            if (excess <= 0) break;
            auto& [projectId, taskId, oldHours, adjustedHours] = assignment;
            if (adjustedHours <= 0) {
                continue;
            }
            
            auto reduction = std::min(excess, adjustedHours);
            adjustedHours = adjustedHours - reduction;
            totalScaledHours -= reduction;
            excess -= reduction;
        }
    }
    
    void scaleAssignmentsToCapacity(
        std::vector<std::tuple<int, int, int, int*>>& assignments,
        double scaleFactor,
        int capacity,
        const std::shared_ptr<Employee>& employee,
        const ProjectContainer& projects) {
        for (auto& assignment : assignments) {
            auto& [projectId, taskId, oldHours, hoursPtr] = assignment;
            auto newHours = static_cast<int>(std::round(oldHours * scaleFactor));
            
            newHours = std::max(0, std::min(newHours, capacity));
            
            *hoursPtr = newHours;
            
            auto projPtr = projects.find(projectId);
            updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
        }
    }
    
    void collectScaledAssignments(
        int employeeId,
        double scaleFactor,
        const std::map<std::tuple<int, int, int>, int>& taskAssignments,
        std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        int& totalScaledHours) {
        for (const auto& [key, oldHours] : taskAssignments) {
            const auto& [empId, projectId, taskId] = key;
            if (empId != employeeId) {
                continue;
            }
            auto scaledHours = static_cast<int>(std::round(oldHours * scaleFactor));
            if (scaledHours < 0) {
                scaledHours = 0;
            }
            assignmentsData.emplace_back(projectId, taskId, oldHours, scaledHours);
            totalScaledHours += scaledHours;
        }
    }
    
    void processEmployeeAssignments(
        [[maybe_unused]] int employeeId,
        std::vector<std::tuple<int, int, int, int*>>& assignments,
        int capacity,
        const std::shared_ptr<Employee>& employee,
        const ProjectContainer& projects) {
        int totalHours = 0;
        for (const auto& assignment : assignments) {
            const auto& [projectId, taskId, oldHours, hoursPtr] = assignment;
            totalHours += oldHours;
        }
        
        if (totalHours > capacity && totalHours > 0) {
            const auto scaleFactor = static_cast<double>(capacity) / totalHours;
            scaleAssignmentsToCapacity(assignments, scaleFactor, capacity, employee, projects);
        }
    }
    
    bool employeeRoleMatchesSDLC(const std::shared_ptr<Employee>& employee,
                                const QString& projectPhase) {
        if (!employee) return false;
        QString pos = employee->getPosition();
        return roleMatchesSDLCStage(pos, projectPhase);
    }
    
    bool employeeTaskTypeMatches(const std::shared_ptr<Employee>& employee,
                                 const QString& taskType) {
        if (!employee) return false;
        QString employeeType = employee->getEmployeeType();
        static const std::map<QString, QString> typeMapping = {
            {"Management", "Manager"},
            {"Development", "Developer"},
            {"Design", "Designer"},
            {"QA", "QA"}
        };
        if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
            const auto& [key, value] = *it;
            return value == employeeType;
        }
        return false;
    }
    
    void buildEmployeePool(
        const BuildEmployeePoolParams& params,
        std::vector<std::shared_ptr<Employee>>& pool) {
        for (const auto& employee : params.employeesList) {
            if (!employee) continue;
            if (!employeeRoleMatchesSDLC(employee, params.projectPhase)) continue;
            if (!employeeTaskTypeMatches(employee, params.taskType)) continue;
            if (employee->getSalary() > params.projectBudget) continue;
            
            if (params.projectEstimatedHours > 0) {
                double employeeHourlyRate = calculateHourlyRate(employee->getSalary());
                if (employeeHourlyRate > params.maxAffordableHourlyRate) {
                    continue;
                }
            }
            
            int available = employee->getAvailableHours();
            int employeeId = employee->getId();
            int alreadyUsed = 0;
            if (auto it = params.employeeUsage.find(employeeId); it != params.employeeUsage.end()) {
                const auto& [key, hours] = *it;
                alreadyUsed = hours;
            }
            int trulyAvailable = available - alreadyUsed;
            if (trulyAvailable > 0) {
                pool.emplace_back(employee);
            }
        }
    }
    
    void assignEmployeeToTaskInPool(
        const std::shared_ptr<Employee>& poolEmployee,
        Task& task,
        AssignEmployeeToTaskParams& params) {
        int employeeId = poolEmployee->getId();
        int trulyAvailable = poolEmployee->getAvailableHours();
        if (auto it = params.employeeUsage.find(employeeId); it != params.employeeUsage.end()) {
            const auto& [key, hours] = *it;
            trulyAvailable -= hours;
        }
        if (trulyAvailable <= 0) return;
        
        double hourlyRate = calculateHourlyRate(poolEmployee->getSalary());
        int maxAffordableHours = 0;
        if (hourlyRate > 0 && params.remainingBudget > 0) {
            maxAffordableHours = static_cast<int>(params.remainingBudget / hourlyRate);
        }
        
        int toAssign = params.remaining;
        if (trulyAvailable < toAssign) {
            toAssign = trulyAvailable;
        }
        if (maxAffordableHours < toAssign) {
            toAssign = maxAffordableHours;
        }
        if (toAssign <= 0) return;
        
        double assignmentCost = calculateEmployeeCost(poolEmployee->getSalary(), toAssign);
        double totalCost = params.currentEmployeeCosts + assignmentCost;
        if (double projectBudget = params.remainingBudget + params.currentEmployeeCosts; totalCost > projectBudget) {
            return;
        }
        
        poolEmployee->addWeeklyHours(toAssign);
        poolEmployee->addAssignedProject(params.projectId);
        task.addAllocatedHours(toAssign);
        params.employeeUsage[employeeId] += toAssign;
        
        const auto key = std::make_tuple(employeeId, params.projectId, task.getId());
        params.taskAssignments[key] += toAssign;
        params.currentEmployeeCosts += assignmentCost;
        params.remainingBudget -= assignmentCost;
        params.remaining -= toAssign;
    }
    


bool taskTypeMatchesEmployeeType(const QString& taskType,
                                  const QString& employeeType) {
    static const std::map<QString, QString> typeMapping = {
        {"Management", "Manager"},
        {"Development", "Developer"},
        {"Design", "Designer"},
        {"QA", "QA"}
    };
    
    if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
        const auto& [key, value] = *it;
        return value == employeeType;
    }
    return false;
}

}

namespace {
    void validateTaskAssignment(const std::shared_ptr<Employee>& employee,
                                [[maybe_unused]] const std::shared_ptr<Project>& projPtr,
                                const Task& task,
                                int hours,
                                const QString& projectPhase) {
        if (hours > task.getEstimatedHours()) {
            throw CompanyException(QString("Cannot assign %1 hours: task "
                                           "has only %2 estimated hours")
                                       .arg(hours)
                                       .arg(task.getEstimatedHours()));
        }

        auto employeePosition = employee->getPosition();
        if (bool roleMatches = roleMatchesSDLCStage(employeePosition, projectPhase); !roleMatches) {
            throw CompanyException(QString("Employee role '%1' does not "
                                           "match project SDLC stage '%2'")
                                       .arg(employeePosition)
                                       .arg(projectPhase));
        }

        auto taskType = task.getType();
        auto employeeType = employee->getEmployeeType();
        if (bool taskTypeMatches = taskTypeMatchesEmployeeType(taskType, employeeType); !taskTypeMatches) {
            QString requiredType = getRequiredEmployeeType(taskType);
            throw CompanyException(
                QString(
                    "Employee type '%1' does not match task type '%2'.\n"
                    "Task type '%2' requires employee type '%3'.")
                    .arg(employeeType)
                    .arg(taskType)
                    .arg(requiredType));
        }
    }
}

void Company::assignEmployeeToTask(int employeeId, int projectId, int taskId,
                                   int hours) {
    std::shared_ptr<Employee> employee = getEmployee(employeeId);
    if (!employee) throw CompanyException("Employee not found");
    if (!employee->getIsActive())
        throw CompanyException("Cannot assign inactive employee");

    SafeValue<int> safeHours(hours, 1, kMaxHoursPerWeek);
    if (!safeHours.isValidValue()) {
        throw CompanyException(
            QString("Hours must be between 1 and %1 (week maximum)")
                .arg(kMaxHoursPerWeek));
    }
    hours = safeHours.getValue();

    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    bool found = false;
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
            validateTaskAssignment(employee, projPtr, task, hours, projectPhase);
            
            auto key = std::make_tuple(employeeId, projectId, taskId);
            
            auto needed = task.getEstimatedHours() - task.getAllocatedHours();
            if (needed <= 0) {
                throw CompanyException("Task already fully allocated");
            }

            int toAssign = std::min(needed, hours);
            if (toAssign <= 0) {
                return;
            }

            if (!employee->isAvailable(toAssign)) {
                throwAvailabilityException(employee, toAssign);
            }

            auto employeeHourlyRate = calculateHourlyRate(employee->getSalary());
            auto assignmentCost = calculateEmployeeCost(employee->getSalary(), toAssign);
            
            validateBudgetConstraints(employee, projPtr, toAssign, employeeHourlyRate, assignmentCost);

            employee->addWeeklyHours(toAssign);
            employee->addAssignedProject(projectId);
            task.addAllocatedHours(toAssign);

            
            taskAssignments[key] += toAssign;

            projPtr->addEmployeeCost(assignmentCost);

            projPtr->recomputeTotalsFromTasks();
            found = true;
            break;
        }
    }
    if (!found) throw CompanyException("Task not found");
}

void Company::restoreTaskAssignment(int employeeId, int projectId, int taskId,
                                    int hours) {
    std::shared_ptr<Employee> employee = getEmployee(employeeId);
    if (!employee) return;  

    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) return;  

    const std::vector<Task>& tasks = projPtr->getTasks();
    for (const auto& task : tasks) {
        if (task.getId() == taskId) {
            
                auto key = std::make_tuple(employeeId, projectId, taskId);
            int existingHours = 0;
            if (auto it = taskAssignments.find(key); it != taskAssignments.end()) {
                const auto& [foundKey, existingHoursValue] = *it;
                existingHours = existingHoursValue;
            }

            
            auto newHours = hours - existingHours;

            
            
            taskAssignments[key] = hours;

            
            
            
            employee->addToProjectHistory(projectId);

            
            
            if (employee->getIsActive()) {
                
                
                employee->addAssignedProject(projectId);
            }

            
            
            if (employee->getIsActive() && newHours > 0) {
                try {
                    employee->addWeeklyHours(newHours);
                } catch (const EmployeeException& e) {
                    // Log error but continue execution
                    qCWarning(companyExceptions) << "Failed to add weekly hours:" << e.what();
                }
            }

            break;
        }
    }
}

void Company::removeEmployeeTaskAssignments(int employeeId) {
    
    auto it = taskAssignments.begin();
    while (it != taskAssignments.end()) {
        const auto& [key, hours] = *it;
        const auto& [empId, projectId, taskId] = key;
        if (empId == employeeId) {
            it = taskAssignments.erase(it);
        } else {
            ++it;
        }
    }
}

void Company::recalculateEmployeeHours() {
    
    auto employees = getAllEmployees();
    for (const auto& emp : employees) {
        if (emp) {
            // Remove existing hours
            if (int currentHours = emp->getCurrentWeeklyHours(); currentHours > 0) {
                try {
                    emp->removeWeeklyHours(currentHours);
                } catch (const EmployeeException& e) {
                    // Log error but continue execution
                    qCWarning(companyExceptions) << "Failed to remove weekly hours:" << e.what();
                }
            }
        }
    }

    
    for (const auto& [key, hours] : taskAssignments) {
        const auto& [employeeId, projectId, taskId] = key;

        std::shared_ptr<Employee> employee = getEmployee(employeeId);
        // Add hours for active employees
        if (employee && employee->getIsActive() && hours > 0) {
            try {
                employee->addWeeklyHours(hours);
            } catch (const EmployeeException& e) {
                qCWarning(companyExceptions) << "Failed to add weekly hours:" << e.what();
            }
        }
    }
}

void Company::fixTaskAssignmentsToCapacity() {
    
    
    std::map<int, std::vector<std::tuple<int, int, int, int*>>> employeeAssignments;
    
    for (auto& [key, hours] : taskAssignments) {
        const auto& [employeeId, projectId, taskId] = key;
        
        employeeAssignments[employeeId].emplace_back(
            projectId, taskId, hours, &hours);
    }
    
    
    for (auto& [employeeId, assignments] : employeeAssignments) {
        std::shared_ptr<Employee> employee = getEmployee(employeeId);
        if (!employee) continue;
        
        int capacity = employee->getWeeklyHoursCapacity();
        processEmployeeAssignments(employeeId, assignments, capacity, employee, projects);
    }
}

void Company::recalculateTaskAllocatedHours() {
    
    fixTaskAssignmentsToCapacity();
    
    
        auto allProjects = getAllProjects();
    for (const auto& proj : allProjects) {
        auto* mutableProject = getProject(proj.getId());
        if (mutableProject) {
            // Clear existing costs
            double currentCosts = mutableProject->getEmployeeCosts();
            if (currentCosts > 0) {
                mutableProject->removeEmployeeCost(currentCosts);
            }
        }
    }
    
    
    
    for (const auto& project : allProjects) {
        Project* mutableProject = getProject(project.getId());
        if (!mutableProject) continue;
        
        auto projectId = project.getId();
        auto& tasks = mutableProject->getTasks();
        auto projectTotalCosts = 0.0;
        
        for (auto& task : tasks) {
            auto taskId = task.getId();
            
            const auto [totalAllocated, taskCost] = calculateTaskAllocatedHours(
                projectId, taskId, getAllEmployees(), taskAssignments);
            task.setAllocatedHours(totalAllocated);
            projectTotalCosts += taskCost;
        }
        
        
        if (projectTotalCosts > 0) {
            mutableProject->addEmployeeCost(projectTotalCosts);
        }
        
        
        mutableProject->recomputeTotalsFromTasks();
    }
}

static int compareTaskPriority(const Task& taskA, const Task& taskB) {
    if (taskA.getPriority() != taskB.getPriority()) {
        if (taskA.getPriority() > taskB.getPriority()) {
            return -1;
        }
        return 1;
    }
    int remainingA = taskA.getEstimatedHours() - taskA.getAllocatedHours();
    int remainingB = taskB.getEstimatedHours() - taskB.getAllocatedHours();
    if (remainingA > remainingB) {
        return -1;
    }
    if (remainingA < remainingB) {
        return 1;
    }
    return 0;
}


void Company::autoAssignEmployeesToProject(int projectId) {
    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot auto-assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    if (tasks.empty()) throw CompanyException("No tasks in project");

    std::vector<size_t> taskIndices(tasks.size());
    std::iota(taskIndices.begin(), taskIndices.end(), 0);
    std::ranges::sort(taskIndices,
                      [&tasks](size_t a, size_t b) {
                          return compareTaskPriority(tasks[a], tasks[b]) < 0;
                      });

    auto allEmployees = getAllEmployees();
    std::vector<std::shared_ptr<Employee>> employeesList;
    buildActiveEmployeesList(allEmployees, employeesList);

    std::map<int, int> employeeUsage;
    auto currentEmployeeCosts = projPtr->getEmployeeCosts();
    auto remainingBudget = projPtr->getBudget() - currentEmployeeCosts;

    for (const auto taskIndex : taskIndices) {
        Task& task = tasks[taskIndex];
        int remaining = task.getEstimatedHours() - task.getAllocatedHours();
        if (remaining <= 0) continue;
        
        std::vector<std::shared_ptr<Employee>> pool;
        double projectBudget = projPtr->getBudget();
        double projectEstimatedHours = projPtr->getEstimatedHours();
        double maxAffordableHourlyRate = 0.0;
        if (projectEstimatedHours > 0) {
            auto averageBudgetPerHour = projectBudget / projectEstimatedHours;
            maxAffordableHourlyRate = averageBudgetPerHour * 0.7;
        }
        
        auto taskType = task.getType();
        
        BuildEmployeePoolParams poolParams{
            employeesList, projectPhase, taskType, projectBudget,
            maxAffordableHourlyRate, projectEstimatedHours, employeeUsage
        };
        buildEmployeePool(poolParams, pool);
        
        std::ranges::sort(pool,
                          [&employeeUsage](const std::shared_ptr<Employee>& a,
                                           const std::shared_ptr<Employee>& b) {
                              return compareEmployeesForSorting(a, b, employeeUsage) < 0;
                          });
        
        AssignEmployeeToTaskParams assignParams{
            projectId, remaining, currentEmployeeCosts, remainingBudget,
            employeeUsage, taskAssignments
        };
        for (const auto& poolEmployee : pool) {
            if (remaining <= 0) break;
            assignEmployeeToTaskInPool(poolEmployee, task, assignParams);
        }
    }

    // Calculate and apply total costs
    double totalNewCosts = 0.0;
    for (const auto& [employeeId, hours] : employeeUsage) {
        auto emp = getEmployee(employeeId);
        if (emp) {
            totalNewCosts += calculateEmployeeCost(emp->getSalary(), hours);
        }
    }
    projPtr->addEmployeeCost(totalNewCosts);
    projPtr->recomputeTotalsFromTasks();
}

int Company::getEmployeeProjectHours(int employeeId, int projectId) const {
    std::shared_ptr<Employee> employee = getEmployee(employeeId);
    if (!employee || !employee->isAssignedToProject(projectId)) {
        return 0;
    }

    int totalHours = 0;
    auto projectTasks = getProjectTasks(projectId);

    for (const auto& task : projectTasks) {
        totalHours += getEmployeeTaskHours(employeeId, projectId, task.getId());
    }

    
    if (int capacity = employee->getWeeklyHoursCapacity(); totalHours > capacity) {
        totalHours = capacity;
    }

    return totalHours;
}

int Company::getEmployeeTaskHours(int employeeId, int projectId,
                                  int taskId) const {
    return getTaskAssignment(employeeId, projectId, taskId);
}

void Company::scaleEmployeeTaskAssignments(int employeeId, double scaleFactor) {
    if (scaleFactor <= 0) {
        return;
    }

    std::shared_ptr<Employee> employee = getEmployee(employeeId);
    if (!employee) {
        return;
    }

    int capacity = employee->getWeeklyHoursCapacity();

    
    std::vector<std::tuple<int, int, int, int>> assignmentsData;
    int totalScaledHours = 0;
    
    collectScaledAssignments(employeeId, scaleFactor, taskAssignments, assignmentsData, totalScaledHours);

    
    if (assignmentsData.empty()) {
        return;
    }
    if (totalScaledHours > capacity) {
        const auto adjustFactor = static_cast<double>(capacity) / totalScaledHours;
        totalScaledHours = 0;
        
        for (auto& assignment : assignmentsData) {
            auto& [projectId, taskId, oldHours, scaledHours] = assignment;
            auto adjustedHours = static_cast<int>(std::round(scaledHours * adjustFactor));
            
            if (adjustedHours < 0) adjustedHours = 0;
            if (adjustedHours > capacity) adjustedHours = capacity;
            
            scaledHours = adjustedHours;
            totalScaledHours += adjustedHours;
        }
        
        
        if (totalScaledHours > capacity) {
            int excess = totalScaledHours - capacity;
            reduceExcessHours(assignmentsData, excess, totalScaledHours);
        }
    }

    
    for (const auto& assignment : assignmentsData) {
        const auto& [projectId, taskId, oldHours, newHours] = assignment;
        const auto key = std::make_tuple(employeeId, projectId, taskId);
        
        
        if (newHours > 0) {
            taskAssignments[key] = newHours;
        } else {
            taskAssignments.erase(key);
        }

        
        auto projPtr = projects.find(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
    }

    
    recalculateTaskAllocatedHours();

    
    if (employee->getIsActive()) {
        
        const int currentCapacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        for (const auto& [key, hours] : taskAssignments) {
            const auto& [empId, projectId, taskId] = key;
            if (empId == employeeId) {
                totalHours += hours;
            }
        }
        
        
        if (totalHours > currentCapacity) {
            totalHours = currentCapacity;
        }
        
        
        
        if (int currentHours = employee->getCurrentWeeklyHours(); currentHours > 0) {
            try {
                employee->removeWeeklyHours(currentHours);
            } catch (const EmployeeException& e) {
                qCWarning(companyExceptions) << "Failed to remove weekly hours:" << e.what();
            }
        }
        
        
        if (totalHours > 0) {
            try {
                employee->addWeeklyHours(totalHours);
            } catch (const EmployeeException& e) {
                qCWarning(companyExceptions) << "Failed to add weekly hours:" << e.what();
            }
            
            if (totalHours > currentCapacity) {
                try {
                    employee->addWeeklyHours(currentCapacity);
                } catch (const EmployeeException& e) {
                    // Log error but continue execution
                    qCWarning(companyExceptions) << "Failed to add weekly hours:" << e.what();
                }
            }
        }
    }
}

int Company::getTaskAssignment(int employeeId, int projectId, int taskId) const {
    const auto key = std::make_tuple(employeeId, projectId, taskId);
    if (const auto it = taskAssignments.find(key); it != taskAssignments.end()) {
        return it->second;
    }
    return 0;
}

void Company::setTaskAssignment(int employeeId, int projectId, int taskId, int hours) {
    const auto key = std::make_tuple(employeeId, projectId, taskId);
    if (hours > 0) {
        taskAssignments[key] = hours;
    } else {
        taskAssignments.erase(key);
    }
}

void Company::addTaskAssignment(int employeeId, int projectId, int taskId, int hours) {
    const auto key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments[key] += hours;
}

void Company::removeTaskAssignment(int employeeId, int projectId, int taskId) {
    const auto key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments.erase(key);
}

std::map<std::tuple<int, int, int>, int> Company::getAllTaskAssignments() const {
    return taskAssignments;
}

Project* Company::getMutableProject(int projectId) {
    return getProject(projectId);
}
