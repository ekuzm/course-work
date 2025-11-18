#include "entities/company.h"

#include <QLoggingCategory>
#include <algorithm>
#include <cmath>
#include <functional>
#include <map>
#include <numeric>
#include <ranges>
#include <tuple>
#include <utility>

#include "exceptions/exceptions.h"
#include "utils/company_utils.h"
#include "utils/consts.h"
#include "utils/container_utils.h"

Q_LOGGING_CATEGORY(companyExceptions, "company.exceptions")

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
    
static void buildActiveEmployeesList(
    const std::vector<std::shared_ptr<Employee>>& allEmployees,
                                  std::vector<std::shared_ptr<Employee>>& employeesList) {
        for (const auto& emp : allEmployees) {
            if (emp && emp->getIsActive()) {
                employeesList.emplace_back(emp);
            }
        }
    }
    
[[noreturn]] static void throwAvailabilityException(
    const std::shared_ptr<Employee>& employee, int toAssign) {
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
    
static void updateTaskAndProjectCosts(std::shared_ptr<Project> projPtr,
                                      int taskId, int oldHours, int newHours,
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
                
            const auto costDiff = calculateEmployeeCost(
                                       employee->getSalary(), newHours) -
                                   calculateEmployeeCost(
                                       employee->getSalary(), oldHours);
                if (costDiff < 0) {
                    projPtr->removeEmployeeCost(-costDiff);
                } else {
                    projPtr->addEmployeeCost(costDiff);
                }
            }
        }
    }
    
static void validateBudgetConstraints(const std::shared_ptr<Employee>& employee,
                                   const std::shared_ptr<Project>& projPtr,
                                      int toAssign, double employeeHourlyRate,
                                   double assignmentCost) {
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

    if (double projectEstimatedHours = projPtr->getEstimatedHours();
        projectEstimatedHours > 0) {
        auto averageBudgetPerHour =
            projPtr->getBudget() / projectEstimatedHours;
        auto maxAffordableHourlyRate =
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
    if (employees.size() >= static_cast<size_t>(kMaxEmployees)) {
        return;
    }
    employees.emplace_back(employee);
}

void EmployeeContainer::remove(int employeeId) {
    std::erase_if(employees, [employeeId](const auto& emp) {
        return matchesEmployeeId(emp, employeeId);
    });
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
    if (employees.size() > static_cast<size_t>(kMaxEmployees)) {
        return {};
    }
    return employees;
}

size_t EmployeeContainer::size() const { return employees.size(); }

void ProjectContainer::add(std::shared_ptr<Project> project) {
    if (projects.size() >= static_cast<size_t>(kMaxProjects)) {
        return;
    }
    projects.emplace_back(project);
}

void ProjectContainer::remove(int projectId) {
    std::erase_if(projects, [projectId](const auto& proj) {
        return matchesProjectId(proj, projectId);
    });
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
    if (projects.size() > static_cast<size_t>(kMaxProjects)) {
        return {};
    }
    return projects;
}

size_t ProjectContainer::size() const { return projects.size(); }

Company::Company(QString companyName, QString companyIndustry,
                 QString companyLocation, int companyFoundedYear)
    : name(std::move(companyName)),
      industry(std::move(companyIndustry)),
      location(std::move(companyLocation)),
      foundedYear(companyFoundedYear),
      taskManager(taskAssignments, employees, projects),
      statistics(employees, projects) {}

Company::Company(Company&& other) noexcept
    : name(std::move(other.name)),
      industry(std::move(other.industry)),
      location(std::move(other.location)),
      foundedYear(other.foundedYear),
      employees(std::move(other.employees)),
      projects(std::move(other.projects)),
      taskAssignments(std::move(other.taskAssignments)),
      taskManager(taskAssignments, employees, projects),
      statistics(employees, projects) {}

Company::~Company() {
    // Explicit destructor for resource management
    // All resources are managed automatically:
    // - EmployeeContainer and ProjectContainer use smart pointers (automatic cleanup)
    // - taskAssignments is a std::map (automatic cleanup)
    // - taskManager and statistics are value types (automatic cleanup)
    // This explicit destructor ensures proper resource management order
}

void Company::addEmployee(std::shared_ptr<Employee> employee) {
    if (std::shared_ptr<Employee> existing = getEmployee(employee->getId());
        existing) {
        throw CompanyException("Employee with this ID already exists");
    }
    employees.add(employee);
}

static void removeEmployeeTaskAssignmentsFromProjects(
    int employeeId, const std::vector<int>& assignedProjects,
    const ProjectContainer& projects, const Company& company,
        std::map<std::tuple<int, int, int>, int>& taskAssignments) {
        for (int projectId : assignedProjects) {
            std::shared_ptr<Project> projPtr = projects.find(projectId);
            if (!projPtr) continue;
            
            auto projectTasks = company.getProjectTasks(projectId);
                for (const auto& task : projectTasks) {
                    auto key = std::make_tuple(employeeId, projectId, task.getId());
                    taskAssignments.erase(key);
                }
                projPtr->recomputeTotalsFromTasks();
            }
        }

void Company::removeEmployee(int employeeId) {
    if (std::shared_ptr<Employee> employee = employees.find(employeeId);
        employee) {
        const std::vector<int>& assignedProjects =
            employee->getAssignedProjects();
        removeEmployeeTaskAssignmentsFromProjects(
            employeeId, assignedProjects, projects, *this, taskAssignments);
    }

    std::erase_if(taskAssignments, [employeeId](const auto& pair) {
        const auto& [key, hours] = pair;
        const auto& [empId, projectId, taskId] = key;
        return empId == employeeId;
    });

    employees.remove(employeeId);
}

void Company::addProject(const Project& project) {
    if (const Project* existing = getProject(project.getId());
        existing != nullptr) {
        throw CompanyException("Project with this ID already exists");
    }
    projects.add(std::make_shared<Project>(project));
}

void Company::removeProject(int projectId) {
    std::erase_if(taskAssignments, [projectId](const auto& pair) {
        const auto& [key, hours] = pair;
        const auto& [empId, projId, taskId] = key;
        return projId == projectId;
    });
    projects.remove(projectId);
}

static std::pair<int, double> calculateTaskAllocatedHours(
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
            taskCost += calculateEmployeeCost(
                employee->getSalary(), hours);
            }
        }
        
        return {totalAllocated, taskCost};
    }
    
static void updateTaskAssignmentsFromData(
        int employeeId,
        const std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        std::map<std::tuple<int, int, int>, int>& taskAssignments,
        const ProjectContainer& projects,
        const std::shared_ptr<Employee>& employee) {
        for (const auto& assignment : assignmentsData) {
            const auto& [projectId, taskId, oldHours, newHours] = assignment;
            const auto key = std::make_tuple(employeeId, projectId, taskId);
            
            if (newHours > 0) {
                taskAssignments[key] = newHours;
            } else {
                taskAssignments.erase(key);
    }
            
            auto projPtr = projects.find(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours,
                                  employee);
        }
    }
    
static void updateEmployeeHoursAfterScaling(
        const std::shared_ptr<Employee>& employee,
        const std::map<std::tuple<int, int, int>, int>& taskAssignments,
        int employeeId) {
        if (!employee->getIsActive()) {
            return;
        }
        
        const int currentCapacity = employee->getWeeklyHoursCapacity();
        
        auto totalHours = 0;
        for (const auto& [key, hours] : taskAssignments) {
            const auto& [empId, projectId, taskId] = key;
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
            qCWarning(companyExceptions)
                << "Failed to remove weekly hours:" << e.what();
    }
        }
        
        if (totalHours > 0) {
            try {
                employee->addWeeklyHours(totalHours);
            } catch (const EmployeeException& e) {
            qCWarning(companyExceptions)
                << "Failed to add weekly hours:" << e.what();
            }
            
            if (totalHours > currentCapacity) {
                try {
                    employee->addWeeklyHours(currentCapacity);
                } catch (const EmployeeException& e) {
                qCWarning(companyExceptions)
                    << "Failed to add weekly hours:" << e.what();
                }
            }
        }
    }
    
struct Assignment {
    int projectId;
    int taskId;
    int oldHours;
    int* hoursPtr;
};
    
static void scaleAssignmentsToCapacity(
    const std::vector<Assignment>& assignments, double scaleFactor,
    int capacity, const std::shared_ptr<Employee>& employee,
        const ProjectContainer& projects) {
        for (const auto& assignment : assignments) {
            if (assignment.hoursPtr == nullptr) continue;
        auto newHours =
            static_cast<int>(std::round(assignment.oldHours * scaleFactor));
            newHours = std::max(0, std::min(newHours, capacity));
            *assignment.hoursPtr = newHours;
            
            auto projPtr = projects.find(assignment.projectId);
        updateTaskAndProjectCosts(projPtr, assignment.taskId,
                                  assignment.oldHours, newHours, employee);
        }
    }
    
static void processEmployeeAssignments(
    [[maybe_unused]] int employeeId,
    const std::vector<std::tuple<int, int, int, int*>>& assignments,
    int capacity, const std::shared_ptr<Employee>& employee,
    const ProjectContainer& projects) {
    int totalHours = 0;

    for (const auto& assignment : assignments) {
        const auto& [projectId, taskId, oldHours, hoursPtr] = assignment;
        totalHours += oldHours;
    }

    if (totalHours > capacity && totalHours > 0) {
        const double scaleFactor = static_cast<double>(capacity) / totalHours;

        std::vector<Assignment> assignmentStructs;

        for (const auto& assignment : assignments) {
            const auto& [projectId, taskId, oldHours, hoursPtr] = assignment;
            if (hoursPtr != nullptr) {
                assignmentStructs.emplace_back(projectId, taskId, oldHours,
                                               hoursPtr);
            }
        }

        scaleAssignmentsToCapacity(assignmentStructs, scaleFactor, capacity,
                                   employee, projects);
    }
}
    
static bool employeeRoleMatchesSDLC(const std::shared_ptr<Employee>& employee,
                                const QString& projectPhase) {
        if (!employee) return false;
        QString pos = employee->getPosition();
        return roleMatchesSDLCStage(pos, projectPhase);
    }
    
static bool employeeTaskTypeMatches(const std::shared_ptr<Employee>& employee,
                                 const QString& taskType) {
        if (!employee) return false;
    return taskTypeMatchesEmployeeType(
        taskType, employee->getEmployeeType());
}

static void buildEmployeePool(const BuildEmployeePoolParams& params,
        std::vector<std::shared_ptr<Employee>>& pool) {
        for (const auto& employee : params.employeesList) {
            if (!employee) continue;
            if (!employeeRoleMatchesSDLC(employee, params.projectPhase)) continue;
            if (!employeeTaskTypeMatches(employee, params.taskType)) continue;
            if (employee->getSalary() > params.projectBudget) continue;
            
            if (params.projectEstimatedHours > 0) {
            auto employeeHourlyRate =
                calculateHourlyRate(employee->getSalary());
                if (employeeHourlyRate > params.maxAffordableHourlyRate) {
                    continue;
                }
            }
            
            int available = employee->getAvailableHours();
            int employeeId = employee->getId();
            int alreadyUsed = 0;
        if (auto it = params.employeeUsage.find(employeeId);
            it != params.employeeUsage.end()) {
                const auto& [key, hours] = *it;
                alreadyUsed = hours;
            }
            int trulyAvailable = available - alreadyUsed;
            if (trulyAvailable > 0) {
                pool.emplace_back(employee);
            }
        }
    }
    
static void assignEmployeeToTaskInPool(
    const std::shared_ptr<Employee>& poolEmployee, Task& task,
        AssignEmployeeToTaskParams& params) {
        int employeeId = poolEmployee->getId();
        int trulyAvailable = poolEmployee->getAvailableHours();
    if (auto it = params.employeeUsage.find(employeeId);
        it != params.employeeUsage.end()) {
            const auto& [key, hours] = *it;
            trulyAvailable -= hours;
        }
        if (trulyAvailable <= 0) return;
        
    double hourlyRate =
        calculateHourlyRate(poolEmployee->getSalary());
        int maxAffordableHours = 0;
        if (hourlyRate > 0 && params.remainingBudget > 0) {
        maxAffordableHours =
            static_cast<int>(params.remainingBudget / hourlyRate);
        }
        
        int toAssign = params.remaining;
        if (trulyAvailable < toAssign) {
            toAssign = trulyAvailable;
        }
        if (maxAffordableHours < toAssign) {
            toAssign = maxAffordableHours;
        }
        if (toAssign <= 0) return;
        
    double assignmentCost = calculateEmployeeCost(
        poolEmployee->getSalary(), toAssign);
        double totalCost = params.currentEmployeeCosts + assignmentCost;
    if (double projectBudget =
            params.remainingBudget + params.currentEmployeeCosts;
        totalCost > projectBudget) {
            return;
        }
        
        poolEmployee->addWeeklyHours(toAssign);
        poolEmployee->addAssignedProject(params.projectId);
        task.addAllocatedHours(toAssign);
        params.employeeUsage[employeeId] += toAssign;
        
    const auto key =
        std::make_tuple(employeeId, params.projectId, task.getId());
        params.taskAssignments[key] += toAssign;
        params.currentEmployeeCosts += assignmentCost;
        params.remainingBudget -= assignmentCost;
        params.remaining -= toAssign;
    }

static void validateTaskAssignment(
    const std::shared_ptr<Employee>& employee,
    [[maybe_unused]] const std::shared_ptr<Project>& projPtr, const Task& task,
    int hours, const QString& projectPhase) {
            if (hours > task.getEstimatedHours()) {
                throw CompanyException(QString("Cannot assign %1 hours: task "
                                               "has only %2 estimated hours")
                                           .arg(hours)
                                           .arg(task.getEstimatedHours()));
            }

            auto employeePosition = employee->getPosition();
    if (bool roleMatches =
            roleMatchesSDLCStage(employeePosition, projectPhase);
        !roleMatches) {
                throw CompanyException(QString("Employee role '%1' does not "
                                               "match project SDLC stage '%2'")
                                           .arg(employeePosition)
                                           .arg(projectPhase));
            }

            auto taskType = task.getType();
            auto employeeType = employee->getEmployeeType();
    if (bool taskTypeMatches =
            taskTypeMatchesEmployeeType(taskType, employeeType);
        !taskTypeMatches) {
            QString requiredType = getRequiredEmployeeType(taskType);
                throw CompanyException(
            QString("Employee type '%1' does not match task type '%2'.\n"
                        "Task type '%2' requires employee type '%3'.")
                        .arg(employeeType)
                        .arg(taskType)
                        .arg(requiredType));
            }
    }
