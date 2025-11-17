#include "entities/company_managers.h"

#include <QLoggingCategory>
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <ranges>
#include <tuple>
#include <vector>

#include "entities/company.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"
#include "exceptions/exceptions.h"
#include "utils/company_utils.h"
#include "utils/consts.h"
#include "utils/container_utils.h"

Q_LOGGING_CATEGORY(companyManagers, "company.managers")

static std::vector<std::shared_ptr<Employee>> getActiveEmployees(
    const EmployeeContainer& employees) {
    std::vector<std::shared_ptr<Employee>> activeEmployees;
    auto allEmployees = employees.getAll();
    for (const auto& emp : allEmployees) {
        if (emp && emp->getIsActive()) {
            activeEmployees.emplace_back(emp);
        }
    }
    return activeEmployees;
}

static bool isEmployeeAffordable(const std::shared_ptr<Employee>& employee,
                                 double projectBudget,
                                 double maxAffordableHourlyRate,
                                 int projectEstimatedHours) {
    if (employee->getSalary() > projectBudget) {
        return false;
    }
    if (projectEstimatedHours > 0) {
        auto employeeHourlyRate =
            calculateHourlyRate(employee->getSalary());
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
    if (auto it = employeeUsage.find(employeeId); it != employeeUsage.end()) {
        available -= it->second;
    }
    return available;
}

static std::vector<std::shared_ptr<Employee>> buildEmployeePool(
    const std::vector<std::shared_ptr<Employee>>& activeEmployees,
    const QString& projectPhase, const QString& taskType, double projectBudget,
    double maxAffordableHourlyRate, int projectEstimatedHours,
    const std::map<int, int>& employeeUsage) {
    std::vector<std::shared_ptr<Employee>> pool;
    for (const auto& employee : activeEmployees) {
        if (!employee) continue;

        if (QString pos = employee->getPosition();
            !roleMatchesSDLCStage(pos, projectPhase))
            continue;

        if (QString employeeType = employee->getEmployeeType();
            !taskTypeMatchesEmployeeType(taskType, employeeType))
            continue;

        if (!isEmployeeAffordable(employee, projectBudget,
                                  maxAffordableHourlyRate,
                                  projectEstimatedHours)) {
            continue;
        }

        if (int trulyAvailable =
                getTrulyAvailableHours(employee, employeeUsage);
            trulyAvailable > 0) {
            pool.emplace_back(employee);
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

struct ProcessTaskAssignmentParams {
    Task& task;
    int projectId;
    double projectBudget;
    const QString& projectPhase;
    int projectEstimatedHours;
    double maxAffordableHourlyRate;
    const std::vector<std::shared_ptr<Employee>>& activeEmployees;
    std::map<int, int>& employeeUsage;
    std::map<std::tuple<int, int, int>, int>& taskAssignments;
    double& currentEmployeeCosts;
    double& remainingBudget;
};

static void assignEmployeeToTask(
    const std::shared_ptr<Employee>& poolEmployee, Task& task, int projectId,
    int toAssign, std::map<int, int>& employeeUsage,
    std::map<std::tuple<int, int, int>, int>& taskAssignments) {
    int employeeId = poolEmployee->getId();
    poolEmployee->addWeeklyHours(toAssign);
    poolEmployee->addAssignedProject(projectId);
    task.addAllocatedHours(toAssign);
    employeeUsage[employeeId] += toAssign;

    const auto key = std::make_tuple(employeeId, projectId, task.getId());
    taskAssignments[key] += toAssign;
}

static void processTaskAssignment(const ProcessTaskAssignmentParams& params) {
    if (params.task.getAllocatedHours() >= params.task.getEstimatedHours()) {
        return;
    }

    int remaining =
        params.task.getEstimatedHours() - params.task.getAllocatedHours();
    QString taskType = params.task.getType();

    auto pool =
        buildEmployeePool(params.activeEmployees, params.projectPhase, taskType,
                          params.projectBudget, params.maxAffordableHourlyRate,
                          params.projectEstimatedHours, params.employeeUsage);

    std::ranges::sort(pool, [&params](const auto& a, const auto& b) {
        return compareEmployeesForSorting(
                   a, b, params.employeeUsage) < 0;
    });

    for (const auto& poolEmployee : pool) {
        if (remaining <= 0) break;

        int trulyAvailable =
            getTrulyAvailableHours(poolEmployee, params.employeeUsage);
        if (trulyAvailable <= 0) continue;

        double hourlyRate =
            calculateHourlyRate(poolEmployee->getSalary());
        int maxAffordableHours = 0;
        if (hourlyRate > 0 && params.remainingBudget > 0) {
            maxAffordableHours =
                static_cast<int>(params.remainingBudget / hourlyRate);
        }

        int toAssign = calculateToAssignHours(remaining, trulyAvailable,
                                              maxAffordableHours);
        if (toAssign <= 0) continue;

        double assignmentCost = calculateEmployeeCost(
            poolEmployee->getSalary(), toAssign);
        if (double totalCost = params.currentEmployeeCosts + assignmentCost;
            totalCost > params.projectBudget) {
            continue;
        }

        assignEmployeeToTask(poolEmployee, params.task, params.projectId,
                             toAssign, params.employeeUsage,
                             params.taskAssignments);
        params.currentEmployeeCosts += assignmentCost;
        params.remainingBudget -= assignmentCost;
        remaining -= toAssign;
    }
}

static void resetEmployeeHours(const EmployeeContainer& employees) {
    auto allEmployees = employees.getAll();
    for (const auto& emp : allEmployees) {
        if (emp) {
            if (int currentHours = emp->getCurrentWeeklyHours();
                currentHours > 0) {
                try {
                    emp->removeWeeklyHours(currentHours);
                } catch (const EmployeeException& e) {
                    qCWarning(companyManagers)
                        << "Failed to remove weekly hours:" << e.what();
                }
            }
        }
    }
}

static void applyTaskAssignmentsToEmployees(
    const std::map<std::tuple<int, int, int>, int>& taskAssignments,
    const EmployeeContainer& employees) {
    for (const auto& [key, hours] : taskAssignments) {
        const auto& [employeeId, projectId, taskId] = key;
        if (std::shared_ptr<Employee> employee = employees.find(employeeId);
            employee && employee->getIsActive() && hours > 0) {
            try {
                employee->addWeeklyHours(hours);
            } catch (const EmployeeException& e) {
                qCWarning(companyManagers)
                    << "Failed to add weekly hours:" << e.what();
            }
        }
    }
}

static void clearProjectCosts(const ProjectContainer& projects) {
    auto allProjects = projects.getAll();
    for (const auto& proj : allProjects) {
        if (!proj) continue;
        if (double currentCosts = proj->getEmployeeCosts(); currentCosts > 0) {
            proj->removeEmployeeCost(currentCosts);
        }
    }
}

static void calculateTaskAllocatedHoursForProject(
    const std::shared_ptr<Project>& proj, const EmployeeContainer& employees,
    const std::map<std::tuple<int, int, int>, int>& taskAssignments) {
    auto projectId = proj->getId();
    Project* mutableProj = proj.get();
    auto& tasks = mutableProj->getTasks();
    double projectTotalCosts = 0.0;

    for (auto& task : tasks) {
        auto taskId = task.getId();
        int totalAllocated = 0;
        double taskCost = 0.0;

        auto allEmployees = employees.getAll();
        for (const auto& employee : allEmployees) {
            if (!employee || !employee->isAssignedToProject(projectId)) {
                continue;
            }

            auto key = std::make_tuple(employee->getId(), projectId, taskId);
            if (const auto assignmentIt = taskAssignments.find(key);
                assignmentIt != taskAssignments.end()) {
                totalAllocated += assignmentIt->second;
                taskCost += calculateEmployeeCost(
                    employee->getSalary(), assignmentIt->second);
            }
        }
        task.setAllocatedHours(totalAllocated);
        projectTotalCosts += taskCost;
    }

    if (projectTotalCosts > 0) {
        mutableProj->addEmployeeCost(projectTotalCosts);
    }

    mutableProj->recomputeTotalsFromTasks();
}

struct Assignment {
    int projectId;
    int taskId;
    int oldHours;
    size_t storageIndex;
};

static void collectEmployeeAssignments(
    const std::map<std::tuple<int, int, int>, int>& taskAssignments,
    std::map<int, std::vector<int>>& hoursStorage,
    std::map<int, std::vector<Assignment>>& employeeAssignments) {
    for (const auto& [key, hours] : taskAssignments) {
        const auto& [employeeId, projectId, taskId] = key;
        hoursStorage[employeeId].push_back(hours);
        size_t index = hoursStorage[employeeId].size() - 1;
        employeeAssignments[employeeId].emplace_back(projectId, taskId, hours,
                                                     index);
    }
}

static void scaleEmployeeAssignmentsToCapacity(
    std::map<int, std::vector<int>>& hoursStorage,
    const std::vector<Assignment>& assignments, int employeeId, int capacity) {
    int totalHours = 0;
    for (const auto& assignment : assignments) {
        totalHours += assignment.oldHours;
    }

    if (totalHours > capacity && totalHours > 0) {
        const auto scaleFactor = static_cast<double>(capacity) / totalHours;
        for (const auto& assignment : assignments) {
            auto newHours =
                static_cast<int>(std::round(assignment.oldHours * scaleFactor));
            newHours = std::max(0, std::min(newHours, capacity));
            if (assignment.storageIndex < hoursStorage[employeeId].size()) {
                hoursStorage[employeeId][assignment.storageIndex] = newHours;
            }
        }
    }
}

static void collectScaledAssignments(
    const std::map<std::tuple<int, int, int>, int>& taskAssignments,
    int employeeId, double scaleFactor,
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

static void applyScaledAssignments(
    const std::vector<std::tuple<int, int, int, int>>& assignmentsData,
    int employeeId, std::map<std::tuple<int, int, int>, int>& taskAssignments) {
    for (const auto& assignment : assignmentsData) {
        const auto& [projectId, taskId, oldHours, newHours] = assignment;
        const auto key = std::make_tuple(employeeId, projectId, taskId);

        if (newHours > 0) {
            taskAssignments[key] = newHours;
        } else {
            taskAssignments.erase(key);
        }
    }
}

TaskAssignmentManager::TaskAssignmentManager(
    std::map<std::tuple<int, int, int>, int>& assignments,
    EmployeeContainer& empContainer, ProjectContainer& projContainer)
    : taskAssignments(assignments),
      employees(empContainer),
      projects(projContainer) {}

CompanyStatistics::CompanyStatistics(const EmployeeContainer& empContainer,
                                     const ProjectContainer& projContainer)
    : employees(empContainer), projects(projContainer) {}

void TaskAssignmentManager::assignEmployeeToTask(int employeeId, int projectId,
                                                 int taskId, int hours) {
    std::shared_ptr<Employee> employee = employees.find(employeeId);
    if (!employee) throw CompanyException("Employee not found");
    if (!employee->getIsActive())
        throw CompanyException("Cannot assign inactive employee");

    SafeValue safeHours(hours, 1, kMaxHoursPerWeek);
    if (!safeHours.isValidValue()) {
        throw CompanyException(
            QString("Hours must be between 1 and %1 (week maximum)")
                .arg(kMaxHoursPerWeek));
    }
    hours = safeHours.getValue();

    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    if (QString projectPhase = projPtr->getPhase();
        projectPhase == "Completed") {
        throw CompanyException("Cannot assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    bool found = false;
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
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
                throw CompanyException(
                    QString("Not enough available hours to assign %1 hours")
                        .arg(toAssign));
            }

            auto assignmentCost = calculateEmployeeCost(
                employee->getSalary(), toAssign);

            if (assignmentCost >
                projPtr->getBudget() - projPtr->getEmployeeCosts()) {
                throw CompanyException(
                    "Assignment would exceed project budget");
            }

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

void TaskAssignmentManager::restoreTaskAssignment(int employeeId, int projectId,
                                                  int taskId, int hours) {
    std::shared_ptr<Employee> employee = employees.find(employeeId);
    if (!employee) return;

    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) return;

    const std::vector<Task>& tasks = projPtr->getTasks();
    for (const auto& task : tasks) {
        if (task.getId() == taskId) {
            auto key = std::make_tuple(employeeId, projectId, taskId);
            int existingHours = 0;
            if (auto it = taskAssignments.find(key);
                it != taskAssignments.end()) {
                existingHours = it->second;
            }

            int newHours = hours - existingHours;
            taskAssignments[key] = hours;

            employee->addToProjectHistory(projectId);

            if (employee->getIsActive()) {
                employee->addAssignedProject(projectId);
            }

            if (employee->getIsActive() && newHours > 0) {
                try {
                    employee->addWeeklyHours(newHours);
                } catch (const EmployeeException& e) {
                    qCWarning(companyManagers)
                        << "Failed to add weekly hours:" << e.what();
                }
            }
            break;
        }
    }
}

void TaskAssignmentManager::removeEmployeeTaskAssignments(int employeeId) {
    std::erase_if(taskAssignments, [employeeId](const auto& pair) {
        const auto& [key, hours] = pair;
        const auto& [empId, projectId, taskId] = key;
        return empId == employeeId;
    });
}

void TaskAssignmentManager::recalculateEmployeeHours() const {
    resetEmployeeHours(employees);
    applyTaskAssignmentsToEmployees(taskAssignments, employees);
}

void TaskAssignmentManager::recalculateTaskAllocatedHours() const {
    clearProjectCosts(const_cast<ProjectContainer&>(projects));

    auto allProjects = projects.getAll();
    for (const auto& proj : allProjects) {
        if (!proj) continue;
        calculateTaskAllocatedHoursForProject(proj, employees, taskAssignments);
    }
}

static void updateTaskAssignmentsFromStorage(
    const std::map<int, std::vector<int>>& hoursStorage,
    const std::vector<Assignment>& assignments, int employeeId,
    std::map<std::tuple<int, int, int>, int>& taskAssignments) {
    auto storageIt = hoursStorage.find(employeeId);
    if (storageIt == hoursStorage.end()) {
        return;
    }
    const auto& employeeStorage = storageIt->second;
    for (const auto& assignment : assignments) {
        auto key = std::make_tuple(employeeId, assignment.projectId,
                                   assignment.taskId);
        if (assignment.storageIndex < employeeStorage.size()) {
            taskAssignments[key] = employeeStorage[assignment.storageIndex];
        }
    }
}

void TaskAssignmentManager::fixTaskAssignmentsToCapacity() {
    std::map<int, std::vector<int>> hoursStorage;
    std::map<int, std::vector<Assignment>> employeeAssignments;
    collectEmployeeAssignments(taskAssignments, hoursStorage,
                               employeeAssignments);

    for (const auto& [employeeId, assignments] : employeeAssignments) {
        std::shared_ptr<Employee> employee = employees.find(employeeId);
        if (!employee) {
            continue;
        }
        int capacity = employee->getWeeklyHoursCapacity();
        scaleEmployeeAssignmentsToCapacity(hoursStorage, assignments,
                                           employeeId, capacity);
        updateTaskAssignmentsFromStorage(hoursStorage, assignments, employeeId,
                                         taskAssignments);
    }
}

void TaskAssignmentManager::autoAssignEmployeesToProject(int projectId) {
    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto& tasks = projPtr->getTasks();
    if (tasks.empty()) return;

    auto activeEmployees = getActiveEmployees(employees);
    if (activeEmployees.empty()) return;

    double projectBudget = projPtr->getBudget();
    QString projectPhase = projPtr->getPhase();
    int projectEstimatedHours = projPtr->getInitialEstimatedHours();
    double maxAffordableHourlyRate = 0.0;
    if (projectEstimatedHours > 0) {
        maxAffordableHourlyRate =
            projectBudget / static_cast<double>(projectEstimatedHours);
    }

    std::map<int, int> employeeUsage;
    double currentEmployeeCosts = 0.0;
    double remainingBudget = projectBudget - projPtr->getEmployeeCosts();

    for (auto& task : tasks) {
        ProcessTaskAssignmentParams params{task,
                                           projectId,
                                           projectBudget,
                                           projectPhase,
                                           projectEstimatedHours,
                                           maxAffordableHourlyRate,
                                           activeEmployees,
                                           employeeUsage,
                                           taskAssignments,
                                           currentEmployeeCosts,
                                           remainingBudget};
        processTaskAssignment(params);
    }
}

int TaskAssignmentManager::getEmployeeProjectHours(int employeeId,
                                                   int projectId) const {
    int total = 0;
    for (const auto& [key, hours] : taskAssignments) {
        const auto& [empId, projId, taskId] = key;
        if (empId == employeeId && projId == projectId) {
            total += hours;
        }
    }
    return total;
}

int TaskAssignmentManager::getEmployeeTaskHours(int employeeId, int projectId,
                                                int taskId) const {
    auto key = std::make_tuple(employeeId, projectId, taskId);
    if (auto it = taskAssignments.find(key); it != taskAssignments.end()) {
        return it->second;
    }
    return 0;
}

void TaskAssignmentManager::scaleEmployeeTaskAssignments(int employeeId,
                                                         double scaleFactor) {
    std::vector<std::tuple<int, int, int, int>> assignmentsData;
    int totalScaledHours = 0;

    collectScaledAssignments(taskAssignments, employeeId, scaleFactor,
                             assignmentsData, totalScaledHours);

    std::shared_ptr<Employee> employee = employees.find(employeeId);
    if (!employee) return;

    int capacity = employee->getWeeklyHoursCapacity();
    adjustAssignmentsToCapacity(assignmentsData, capacity,
                                               totalScaledHours);
    applyScaledAssignments(assignmentsData, employeeId, taskAssignments);
}

int TaskAssignmentManager::getTaskAssignment(int employeeId, int projectId,
                                             int taskId) const {
    return getEmployeeTaskHours(employeeId, projectId, taskId);
}

void TaskAssignmentManager::setTaskAssignment(int employeeId, int projectId,
                                              int taskId, int hours) {
    auto key = std::make_tuple(employeeId, projectId, taskId);
    if (hours > 0) {
        taskAssignments[key] = hours;
    } else {
        taskAssignments.erase(key);
    }
}

void TaskAssignmentManager::addTaskAssignment(int employeeId, int projectId,
                                              int taskId, int hours) {
    auto key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments[key] += hours;
}

void TaskAssignmentManager::removeTaskAssignment(int employeeId, int projectId,
                                                 int taskId) {
    auto key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments.erase(key);
}

std::map<std::tuple<int, int, int>, int>
TaskAssignmentManager::getAllTaskAssignments() const {
    return taskAssignments;
}

int CompanyStatistics::getEmployeeCount() const {
    return static_cast<int>(employees.size());
}

int CompanyStatistics::getProjectCount() const {
    return static_cast<int>(projects.size());
}

double CompanyStatistics::getTotalSalaries() const {
    double total = 0.0;
    auto allEmployees = employees.getAll();
    for (const auto& emp : allEmployees) {
        if (emp) {
            total += emp->getSalary();
        }
    }
    return total;
}

double CompanyStatistics::getTotalBudget() const {
    double total = 0.0;
    auto allProjects = projects.getAll();
    for (const auto& proj : allProjects) {
        if (proj) {
            total += proj->getBudget();
        }
    }
    return total;
}
