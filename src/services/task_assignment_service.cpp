#include "services/task_assignment_service.h"

#include <QLoggingCategory>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <tuple>
#include <vector>

#include "exceptions/exceptions.h"
#include "services/cost_calculation_service.h"
#include "utils/company_utils.h"
#include "utils/consts.h"
#include "utils/container_utils.h"

Q_LOGGING_CATEGORY(taskAssignmentService, "task.assignment.service")

struct AssignEmployeeToTaskInPoolParams {
    const std::shared_ptr<Employee>& poolEmployee;
    Task& task;
    int projectId;
    int& remaining;
    double& currentEmployeeCosts;
    double& remainingBudget;
    std::map<int, int>& employeeUsage;
    Company* company;
};

struct ProcessTaskAssignmentParams {
    Task& task;
    int projectId;
    const QString& projectPhase;
    double projectBudget;
    double projectEstimatedHours;
    const std::vector<std::shared_ptr<Employee>>& employeesList;
    std::map<int, int>& employeeUsage;
    double& currentEmployeeCosts;
    double& remainingBudget;
    Company* company;
};

static void updateTaskAndProjectCosts(Project* projPtr, int taskId,
                                      int oldHours, int newHours,
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
            
            if (const auto costDiff =
                    CostCalculationService::calculateEmployeeCost(
                        employee->getSalary(), newHours) -
                    CostCalculationService::calculateEmployeeCost(
                        employee->getSalary(), oldHours);
                costDiff < 0) {
                projPtr->removeEmployeeCost(-costDiff);
            } else {
                projPtr->addEmployeeCost(costDiff);
            }
            break;
        }
    }
}
    
static void validateEmployeeAssignment(
    const std::shared_ptr<Employee>& employee, int hours, const Task& task,
                                    const QString& projectPhase) {
    if (!employee->getIsActive()) {
        throw CompanyException("Cannot assign inactive employee");
    }
    
    if (hours > task.getEstimatedHours()) {
        throw CompanyException(QString("Cannot assign %1 hours: task "
                                       "has only %2 estimated hours")
                                   .arg(hours)
                                   .arg(task.getEstimatedHours()));
    }
    
    if (auto employeePosition = employee->getPosition();
        !TaskAssignmentService::roleMatchesSDLCStage(employeePosition,
                                                     projectPhase)) {
        throw CompanyException(QString("Employee role '%1' does not "
                                       "match project SDLC stage '%2'")
                                   .arg(employeePosition)
                                   .arg(projectPhase));
    }
    
    auto taskType = task.getType();
    if (auto employeeType = employee->getEmployeeType();
        !TaskAssignmentService::taskTypeMatchesEmployeeType(taskType,
                                                            employeeType)) {
        QString requiredType = getRequiredEmployeeType(taskType);
        throw CompanyException(
            QString("Employee type '%1' does not match task type '%2'.\n"
                    "Task type '%2' requires employee type '%3'.")
                .arg(employeeType)
                .arg(taskType)
                .arg(requiredType));
    }
    
    int needed = task.getEstimatedHours() - task.getAllocatedHours();
    if (needed <= 0) {
        throw CompanyException("Task already fully allocated");
    }
}
    
static void validateEmployeeAvailability(
    const std::shared_ptr<Employee>& employee, int toAssign) {
    if (!employee->isAvailable(toAssign)) {
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
}
    
static void validateMonthlySalaryBudget(
    const std::shared_ptr<Employee>& employee, const Project* projPtr) {
    if (employee->getSalary() > projPtr->getBudget()) {
        throw CompanyException(
            QString("Cannot assign employee: monthly salary exceeds "
                    "project budget.\n"
                    "Employee monthly salary: $%1\n"
                    "Project budget: $%2\n"
                    "Employee is too expensive for this project budget.")
                .arg(employee->getSalary(), 0, 'f', 2)
                .arg(projPtr->getBudget(), 0, 'f', 2));
    }
}
    
static void validateHourlyRateBudget(const std::shared_ptr<Employee>& employee,
                                  const Project* projPtr) {
    double employeeHourlyRate =
        CostCalculationService::calculateHourlyRate(employee->getSalary());
    
    const double projectEstimatedHours = projPtr->getEstimatedHours();
    if (projectEstimatedHours <= 0) {
        return;
    }
    
    double averageBudgetPerHour = projPtr->getBudget() / projectEstimatedHours;
    double maxAffordableHourlyRate =
        averageBudgetPerHour * kMaxAffordableHourlyRateMultiplier;
    
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
    
static void validateAssignmentCostBudget(
    const std::shared_ptr<Employee>& employee, int toAssign,
    const Project* projPtr) {
    double employeeHourlyRate =
        CostCalculationService::calculateHourlyRate(employee->getSalary());
    double assignmentCost = CostCalculationService::calculateEmployeeCost(
        employee->getSalary(), toAssign);
    double currentEmployeeCosts = projPtr->getEmployeeCosts();
    double remainingBudget = projPtr->getBudget() - currentEmployeeCosts;
    
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
    
static void validateBudgetConstraints(const std::shared_ptr<Employee>& employee,
                                   int toAssign, const Project* projPtr) {
    validateMonthlySalaryBudget(employee, projPtr);
    validateHourlyRateBudget(employee, projPtr);
    validateAssignmentCostBudget(employee, toAssign, projPtr);
}
    
static void applyTaskAssignment(const std::shared_ptr<Employee>& employee,
                                [[maybe_unused]] int employeeId, int projectId,
                                int taskId, int toAssign, Project* projPtr) {
    employee->addWeeklyHours(toAssign);
    employee->addAssignedProject(projectId);
    
    std::vector<Task>& tasks = projPtr->getTasks();
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
            task.addAllocatedHours(toAssign);
            break;
        }
    }
    
    double assignmentCost = CostCalculationService::calculateEmployeeCost(
        employee->getSalary(), toAssign);
    projPtr->addEmployeeCost(assignmentCost);
    projPtr->recomputeTotalsFromTasks();
}
    
static Task* findTaskById(std::vector<Task>& tasks, int taskId) {
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
            return &task;
        }
    }
    return nullptr;
}

static const Task* findTaskById(const std::vector<Task>& tasks, int taskId) {
    for (const auto& task : tasks) {
        if (task.getId() == taskId) {
            return &task;
        }
    }
    return nullptr;
}

TaskAssignmentService::TaskAssignmentService(Company* company)
    : company(company) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
}

bool TaskAssignmentService::roleMatchesSDLCStage(
    const QString& employeePosition, const QString& projectPhase) {
    if (projectPhase == "Deployment") {
        return employeePosition == "Manager";
    }
    if (projectPhase == "Maintenance") {
        return true;
    }
    return roleMatchesSDLCStage(employeePosition, projectPhase);
}

bool TaskAssignmentService::taskTypeMatchesEmployeeType(
    const QString& taskType, const QString& employeeType) {
    static const std::map<QString, QString> typeMapping = {
        {"Management", "Manager"},
        {"Development", "Developer"},
        {"Design", "Designer"},
        {"QA", "QA"}};
    if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
        return it->second == employeeType;
    }
    return false;
}

void TaskAssignmentService::assignEmployeeToTask(int employeeId, int projectId,
                                                 int taskId, int hours) {
    auto employee = company->getEmployee(employeeId);
    if (!employee) throw CompanyException("Employee not found");

    SafeValue safeHours(hours, 1, kMaxHoursPerWeek);
    if (!safeHours.isValidValue()) {
        throw CompanyException(
            QString("Hours must be between 1 and %1 (week maximum)")
                .arg(kMaxHoursPerWeek));
    }
    hours = safeHours.getValue();

    Project* projPtr = company->getProject(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot assign to project with phase: " +
                               projectPhase);
    }

    const std::vector<Task>& tasks = projPtr->getTasks();
    const Task* task = findTaskById(tasks, taskId);
    if (!task) {
        throw CompanyException("Task not found");
    }

    validateEmployeeAssignment(employee, hours, *task, projectPhase);
    
    int needed = task->getEstimatedHours() - task->getAllocatedHours();
    int toAssign = needed < hours ? needed : hours;
    
    if (toAssign <= 0) {
        return;
    }

    validateEmployeeAvailability(employee, toAssign);
    validateBudgetConstraints(employee, toAssign,
                              const_cast<const Project*>(projPtr));
    
    applyTaskAssignment(employee, employeeId, projectId, taskId, toAssign,
                        projPtr);
    company->getTaskManager().addTaskAssignment(employeeId, projectId, taskId,
                                                toAssign);
}

int TaskAssignmentService::getEmployeeTaskHours(int employeeId, int projectId,
                                                int taskId) const {
    return company->getEmployeeTaskHours(employeeId, projectId, taskId);
}

int TaskAssignmentService::getEmployeeProjectHours(int employeeId,
                                                   int projectId) const {
    auto employee = company->getEmployee(employeeId);
    if (!employee || !employee->isAssignedToProject(projectId)) {
        return 0;
    }

    int totalHours = 0;
    auto projectTasks = company->getProjectTasks(projectId);

    for (const auto& task : projectTasks) {
        totalHours += getEmployeeTaskHours(employeeId, projectId, task.getId());
    }

    if (const int capacity = employee->getWeeklyHoursCapacity();
        totalHours > capacity) {
        totalHours = capacity;
    }

    return totalHours;
}

void TaskAssignmentService::restoreTaskAssignment(int employeeId, int projectId,
                                                  int taskId, int hours) {
    auto employee = company->getEmployee(employeeId);
    if (!employee) return;  

    Project* projPtr = company->getProject(projectId);
    if (!projPtr) return;  

    const std::vector<Task>& tasks = projPtr->getTasks();
    for (const Task& task : tasks) {
        if (task.getId() == taskId) {
            const int newHours = hours - company->getEmployeeTaskHours(
                                             employeeId, projectId, taskId);
            
            company->getTaskManager().setTaskAssignment(employeeId, projectId,
                                                        taskId, hours);
            
            employee->addToProjectHistory(projectId);
            
            if (employee->getIsActive()) {
                employee->addAssignedProject(projectId);
            }
            if (employee->getIsActive() && newHours > 0) {
                try {
                    employee->addWeeklyHours(newHours);
                } catch (const EmployeeException& e) {
                    qCWarning(taskAssignmentService)
                        << "Failed to add weekly hours:" << e.what();
                }
            }

            break;
        }
    }
}

void TaskAssignmentService::removeEmployeeTaskAssignments(int employeeId) {
    auto allAssignments = company->getAllTaskAssignments();
    
    for (const auto& [key, hours] : allAssignments) {
        const auto [empId, projectId, taskId] = key;
        if (empId == employeeId) {
            company->getTaskManager().removeTaskAssignment(employeeId,
                                                           projectId, taskId);
        }
    }
}

static void collectEmployeeAssignments(
    const Company* company,
    std::map<int, std::vector<std::tuple<int, int, int, int>>>&
        employeeAssignments) {
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& [key, hours] : allAssignments) {
        const auto& [employeeId, projectId, taskId] = key;
        employeeAssignments[employeeId].emplace_back(projectId, taskId, hours,
                                                     0);
    }
}

static int calculateTotalHours(
    const std::vector<std::tuple<int, int, int, int>>& assignments) {
    int totalHours = 0;
    for (const auto& assignment : assignments) {
        const auto& [projectId, taskId, oldHours, newHours] = assignment;
        totalHours += oldHours;
    }
    return totalHours;
}

static void scaleAndUpdateAssignments(
    Company* company, int employeeId,
    std::vector<std::tuple<int, int, int, int>>& assignments, int capacity,
    double scaleFactor, const std::shared_ptr<Employee>& employee) {
    for (auto& assignment : assignments) {
        auto& [projectId, taskId, oldHours, newHours] = assignment;
        newHours = static_cast<int>(std::round(oldHours * scaleFactor));
        
        if (newHours < 0) newHours = 0;
        if (newHours > capacity) newHours = capacity;
        
        company->getTaskManager().setTaskAssignment(employeeId, projectId,
                                                    taskId, newHours);
        
        auto projPtr = company->getProject(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours,
                                  employee);
    }
}

void TaskAssignmentService::fixTaskAssignmentsToCapacity() {
    std::map<int, std::vector<std::tuple<int, int, int, int>>>
        employeeAssignments;
    collectEmployeeAssignments(company, employeeAssignments);
    
    for (auto& [employeeId, assignments] : employeeAssignments) {
        auto employee = company->getEmployee(employeeId);
        if (!employee) continue;
        
        const int capacity = employee->getWeeklyHoursCapacity();
        int totalHours = calculateTotalHours(assignments);
        
        if (totalHours <= capacity || totalHours <= 0) {
            continue;
        }
        
        const auto scaleFactor = static_cast<double>(capacity) / totalHours;
        scaleAndUpdateAssignments(company, employeeId, assignments, capacity,
                                  scaleFactor, employee);
    }
}

static void clearProjectCosts(Company* company) {
    auto allProjects = company->getAllProjects();
    for (const auto& project : allProjects) {
        Project* mutableProject = company->getProject(project.getId());
        if (!mutableProject) continue;
        
        double currentCosts = mutableProject->getEmployeeCosts();
        if (currentCosts > 0) {
            mutableProject->removeEmployeeCost(currentCosts);
        }
    }
}

static void calculateTaskAllocatedHours(const Company* company, int projectId,
                                     const std::vector<Task>& tasks,
                                     std::vector<Task>& mutableTasks,
                                     double& projectTotalCosts) {
    auto allEmployees = company->getAllEmployees();
    
    for (size_t i = 0; i < tasks.size(); ++i) {
        int taskId = tasks[i].getId();
        int totalAllocated = 0;
        
        for (const auto& employee : allEmployees) {
            if (!employee || !employee->isAssignedToProject(projectId)) {
                continue;
            }
            
            auto hours = company->getEmployeeTaskHours(employee->getId(),
                                                       projectId, taskId);
            if (hours > 0) {
                totalAllocated += hours;
                double cost = CostCalculationService::calculateEmployeeCost(
                    employee->getSalary(), hours);
                projectTotalCosts += cost;
            }
        }
        
        mutableTasks[i].setAllocatedHours(totalAllocated);
    }
}

static void recalculateProjectHours(Company* company, const Project& project) {
    Project* mutableProject = company->getProject(project.getId());
    if (!mutableProject) return;
    
    int projectId = project.getId();
    std::vector<Task>& tasks = mutableProject->getTasks();
    double projectTotalCosts = 0.0;
    
    calculateTaskAllocatedHours(company, projectId, tasks, tasks,
                                projectTotalCosts);
    
    if (projectTotalCosts > 0) {
        mutableProject->addEmployeeCost(projectTotalCosts);
    }
    
    mutableProject->recomputeTotalsFromTasks();
}

void TaskAssignmentService::recalculateTaskAllocatedHours() {
    fixTaskAssignmentsToCapacity();
    clearProjectCosts(company);
    
    auto allProjects = company->getAllProjects();
    for (const auto& project : allProjects) {
        recalculateProjectHours(company, project);
    }
}

static void collectScaledAssignmentsForEmployee(
    const Company* company, int employeeId, double scaleFactor,
    std::vector<std::tuple<int, int, int, int>>& assignmentsData,
    int& totalScaledHours) {
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& assignment : allAssignments) {
        const auto& [key, oldHours] = assignment;
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

static void updateTaskAssignmentsFromScaledData(
    Company* company, int employeeId,
    const std::vector<std::tuple<int, int, int, int>>& assignmentsData,
    const std::shared_ptr<Employee>& employee) {
    for (const auto& assignment : assignmentsData) {
        const auto& [projectId, taskId, oldHours, newHours] = assignment;
        
        if (newHours > 0) {
            company->getTaskManager().setTaskAssignment(employeeId, projectId,
                                                        taskId, newHours);
        } else {
            company->getTaskManager().removeTaskAssignment(employeeId,
                                                           projectId, taskId);
        }

        Project* projPtr = company->getProject(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours,
                                  employee);
    }
}

static void updateEmployeeHoursAfterScaling(
    const std::shared_ptr<Employee>& employee, const Company* company,
    int employeeId) {
    if (!employee->getIsActive()) {
        return;
    }
    
    const int currentCapacity = employee->getWeeklyHoursCapacity();
    
    int totalHours = 0;
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& [key, hours] : allAssignments) {
        const auto& [empId, projId, tId] = key;
        if (empId == employeeId) {
            totalHours += hours;
        }
    }
    
    if (totalHours > currentCapacity) {
        totalHours = currentCapacity;
    }
    
    if (int currentHours = employee->getCurrentWeeklyHours();
        currentHours > 0) {
        try {
            employee->removeWeeklyHours(currentHours);
        } catch (const EmployeeException& e) {
            qCWarning(taskAssignmentService)
                << "Failed to remove weekly hours:" << e.what();
        }
    }
    
    if (totalHours <= 0) {
        return;
    }
    
    try {
        employee->addWeeklyHours(totalHours);
    } catch (const EmployeeException& e) {
        qCWarning(taskAssignmentService)
            << "Failed to add weekly hours:" << e.what();
        if (totalHours > currentCapacity) {
            try {
                employee->addWeeklyHours(currentCapacity);
            } catch (const EmployeeException& e2) {
                qCWarning(taskAssignmentService)
                    << "Failed to add weekly hours (fallback):" << e2.what();
            }
        }
    }
}

void TaskAssignmentService::scaleEmployeeTaskAssignments(int employeeId,
                                                         double scaleFactor) {
    if (scaleFactor <= 0) {
        return;
    }

    auto employee = company->getEmployee(employeeId);
    if (!employee) {
        return;
    }

    const int capacity = employee->getWeeklyHoursCapacity();
    std::vector<std::tuple<int, int, int, int>> assignmentsData;
    int totalScaledHours = 0;
    
    collectScaledAssignmentsForEmployee(company, employeeId, scaleFactor,
                                        assignmentsData, totalScaledHours);
    
    if (assignmentsData.empty()) {
        return;
    }
    
    adjustAssignmentsToCapacity(assignmentsData, capacity, totalScaledHours);
    updateTaskAssignmentsFromScaledData(company, employeeId, assignmentsData,
                                        employee);
    recalculateTaskAllocatedHours();
    updateEmployeeHoursAfterScaling(employee, company, employeeId);
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

static bool employeeRoleMatchesSDLC(const std::shared_ptr<Employee>& employee,
                                    const QString& projectPhase) {
    if (!employee) return false;
    QString pos = employee->getPosition();
    return TaskAssignmentService::roleMatchesSDLCStage(pos, projectPhase);
}

static bool employeeTaskTypeMatches(const std::shared_ptr<Employee>& employee,
                                    const QString& taskType) {
    if (!employee) return false;
    QString employeeType = employee->getEmployeeType();
    return TaskAssignmentService::taskTypeMatchesEmployeeType(taskType,
                                                              employeeType);
}

static std::vector<size_t> prepareTaskIndices(const std::vector<Task>& tasks) {
    size_t tasksSize = tasks.size();
    if (tasksSize > static_cast<size_t>(kMaxTasksSize)) {
        return {};
    }
    std::vector<size_t> taskIndices(tasksSize);
    std::iota(taskIndices.begin(), taskIndices.end(), 0);
    
    std::ranges::sort(taskIndices, [&tasks](size_t a, size_t b) {
                  return compareTaskPriority(tasks[a], tasks[b]) < 0;
              });
    return taskIndices;
}

static std::vector<std::shared_ptr<Employee>> buildActiveEmployeesList(
    const Company* company) {
    std::vector<std::shared_ptr<Employee>> employeesList;
    auto allEmployees = company->getAllEmployees();
    for (const auto& emp : allEmployees) {
        if (emp && emp->getIsActive()) {
            employeesList.push_back(emp);
        }
    }
    return employeesList;
}

static bool isEmployeeEligibleForTask(const std::shared_ptr<Employee>& employee,
                                    const QString& projectPhase,
                                    const QString& taskType,
                                    double projectBudget,
                                    double maxAffordableHourlyRate,
                                    double projectEstimatedHours) {
    if (!employee) return false;
    if (!employeeRoleMatchesSDLC(employee, projectPhase)) return false;
    if (!employeeTaskTypeMatches(employee, taskType)) return false;
    if (employee->getSalary() > projectBudget) return false;
    
    if (projectEstimatedHours > 0) {
        double employeeHourlyRate =
            CostCalculationService::calculateHourlyRate(employee->getSalary());
        if (employeeHourlyRate > maxAffordableHourlyRate) {
            return false;
        }
    }
    return true;
}

static int getTrulyAvailableHours(const std::shared_ptr<Employee>& employee,
                               const std::map<int, int>& employeeUsage) {
    int available = employee->getAvailableHours();
    int employeeId = employee->getId();
    int alreadyUsed = 0;
    if (auto it = employeeUsage.find(employeeId); it != employeeUsage.end()) {
        alreadyUsed = it->second;
    }
    return available - alreadyUsed;
}

static std::vector<std::shared_ptr<Employee>> buildEmployeePool(
    const std::vector<std::shared_ptr<Employee>>& employeesList,
    const QString& projectPhase, const QString& taskType, double projectBudget,
    double maxAffordableHourlyRate, double projectEstimatedHours,
    const std::map<int, int>& employeeUsage) {
    std::vector<std::shared_ptr<Employee>> pool;
    for (const auto& employee : employeesList) {
        if (!isEmployeeEligibleForTask(employee, projectPhase, taskType,
                                      projectBudget, maxAffordableHourlyRate,
                                      projectEstimatedHours)) {
            continue;
        }
        
        int trulyAvailable = getTrulyAvailableHours(employee, employeeUsage);
        if (trulyAvailable > 0) {
            pool.push_back(employee);
        }
    }
    return pool;
}

static int calculateToAssignHours(int remaining, int trulyAvailable,
                               int maxAffordableHours) {
    int toAssign = remaining;
    if (trulyAvailable < toAssign) {
        toAssign = trulyAvailable;
    }
    if (maxAffordableHours > 0 && maxAffordableHours < toAssign) {
        toAssign = maxAffordableHours;
    }
    return toAssign;
}

static void assignEmployeeToTaskInPool(
    const AssignEmployeeToTaskInPoolParams& params) {
    int employeeId = params.poolEmployee->getId();
    int trulyAvailable =
        getTrulyAvailableHours(params.poolEmployee, params.employeeUsage);
    if (trulyAvailable <= 0) {
        return;
    }

    const double hourlyRate = CostCalculationService::calculateHourlyRate(
        params.poolEmployee->getSalary());
    int maxAffordableHours = 0;
    if (hourlyRate > 0 && params.remainingBudget > 0) {
        maxAffordableHours =
            static_cast<int>(params.remainingBudget / hourlyRate);
    }

    auto toAssign = calculateToAssignHours(params.remaining, trulyAvailable,
                                           maxAffordableHours);
    if (toAssign <= 0) {
        return;
    }

    double assignmentCost = CostCalculationService::calculateEmployeeCost(
        params.poolEmployee->getSalary(), toAssign);
    if (const Project* projPtr = params.company->getProject(params.projectId);
        !projPtr ||
        params.currentEmployeeCosts + assignmentCost > projPtr->getBudget()) {
        return;
    }

    params.poolEmployee->addWeeklyHours(toAssign);
    params.poolEmployee->addAssignedProject(params.projectId);
    params.task.addAllocatedHours(toAssign);
    params.employeeUsage[employeeId] += toAssign;

    params.company->getTaskManager().addTaskAssignment(
        employeeId, params.projectId, params.task.getId(), toAssign);
    params.currentEmployeeCosts += assignmentCost;
    params.remainingBudget -= assignmentCost;
    params.remaining -= toAssign;
}

static void processTaskAssignment(const ProcessTaskAssignmentParams& params) {
    int remaining =
        params.task.getEstimatedHours() - params.task.getAllocatedHours();
    if (remaining <= 0) {
        return;
    }

    double averageBudgetPerHour = 0.0;
    if (params.projectEstimatedHours > 0) {
        averageBudgetPerHour =
            params.projectBudget / params.projectEstimatedHours;
    }
    double maxAffordableHourlyRate = averageBudgetPerHour * 0.7;
    auto taskType = params.task.getType();

    std::vector<std::shared_ptr<Employee>> pool =
        buildEmployeePool(params.employeesList, params.projectPhase, taskType,
                          params.projectBudget, maxAffordableHourlyRate,
                          params.projectEstimatedHours, params.employeeUsage);

    std::ranges::sort(pool, [&params](const std::shared_ptr<Employee>& a,
                               const std::shared_ptr<Employee>& b) {
                  return compareEmployeesForSorting(a, b, params.employeeUsage) < 0;
              });

    for (const auto& poolEmployee : pool) {
        if (remaining <= 0) break;
        AssignEmployeeToTaskInPoolParams assignParams{
            poolEmployee,
            params.task,
            params.projectId,
            remaining,
            params.currentEmployeeCosts,
            params.remainingBudget,
            params.employeeUsage,
            params.company};
        assignEmployeeToTaskInPool(assignParams);
    }
}
    
static void calculateAndApplyTotalCosts(Project* projPtr,
                                     const std::map<int, int>& employeeUsage,
                                     const Company* company) {
    double totalNewCosts = 0.0;
    for (const auto& [employeeId, hours] : employeeUsage) {
        auto emp = company->getEmployee(employeeId);
        if (emp) {
            totalNewCosts += CostCalculationService::calculateEmployeeCost(
                emp->getSalary(), hours);
        }
    }
    projPtr->addEmployeeCost(totalNewCosts);
    projPtr->recomputeTotalsFromTasks();
}

void TaskAssignmentService::autoAssignEmployeesToProject(int projectId) {
    Project* projPtr = company->getProject(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot auto-assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    if (tasks.empty()) throw CompanyException("No tasks in project");

    std::vector<size_t> taskIndices = prepareTaskIndices(tasks);
    std::vector<std::shared_ptr<Employee>> employeesList =
        buildActiveEmployeesList(company);
    std::map<int, int> employeeUsage;

    auto currentEmployeeCosts = projPtr->getEmployeeCosts();
    auto remainingBudget = projPtr->getBudget() - currentEmployeeCosts;
    double projectBudget = projPtr->getBudget();
    double projectEstimatedHours = projPtr->getEstimatedHours();

    for (const auto taskIndex : taskIndices) {
        Task& task = tasks[taskIndex];
        ProcessTaskAssignmentParams processParams{task,
                                                  projectId,
                                                  projectPhase,
                                                  projectBudget,
                                                  projectEstimatedHours,
                                                  employeesList,
                                                  employeeUsage,
                                                  currentEmployeeCosts,
                                                  remainingBudget,
                                                  company};
        processTaskAssignment(processParams);
    }

    calculateAndApplyTotalCosts(projPtr, employeeUsage, company);
}

bool TaskAssignmentService::validateAssignment(
    std::shared_ptr<Employee> employee, std::shared_ptr<Project> project,
                                               const Task& task, int hours) const {
    if (!employee || !project) return false;
    
    if (!employee->getIsActive()) return false;
    if (project->getPhase() == "Completed") return false;
    
    if (hours > task.getEstimatedHours()) return false;
    
    if (const QString employeePosition = employee->getPosition();
        !roleMatchesSDLCStage(employeePosition, project->getPhase()))
        return false;
    
    const QString taskType = task.getType();
    if (const QString employeeType = employee->getEmployeeType();
        !taskTypeMatchesEmployeeType(taskType, employeeType))
        return false;
    
    if (!employee->isAvailable(hours)) return false;
    
    if (const double assignmentCost =
            CostCalculationService::calculateEmployeeCost(employee->getSalary(),
                                                          hours);
        project->getEmployeeCosts() + assignmentCost > project->getBudget())
        return false;
    
    return true;
}
