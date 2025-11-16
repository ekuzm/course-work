#include "services/task_assignment_service.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <numeric>
#include <ranges>
#include <tuple>
#include <vector>

#include <QLoggingCategory>

#include "utils/consts.h"
#include "utils/container_utils.h"
#include "exceptions/exceptions.h"
#include "services/cost_calculation_service.h"

Q_LOGGING_CATEGORY(taskAssignmentService, "task.assignment.service")

TaskAssignmentService::TaskAssignmentService(Company* company) : company(company) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
}

bool TaskAssignmentService::roleMatchesSDLCStage(const QString& employeePosition,
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
    if (projectPhase == "Deployment") {
        return employeePosition == "Manager";
    }
    if (projectPhase == "Maintenance") {
        return true;
    }
    return false;
}

bool TaskAssignmentService::taskTypeMatchesEmployeeType(const QString& taskType,
                                                        const QString& employeeType) {
    if (taskType == "Management" && employeeType == "Manager") {
        return true;
    }
    if (taskType == "Development" && employeeType == "Developer") {
        return true;
    }
    if (taskType == "Design" && employeeType == "Designer") {
        return true;
    }
    if (taskType == "QA" && employeeType == "QA") {
        return true;
    }
    return false;
}

namespace {
    QString getRequiredEmployeeTypeForTask(const QString& taskType) {
        if (taskType == "Management") {
            return "Manager";
        }
        if (taskType == "Development") {
            return "Developer";
        }
        if (taskType == "Design") {
            return "Designer";
        }
        return "QA";
    }
    
    void validateTaskAssignmentDetails(
        const std::shared_ptr<Employee>& employee,
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
        if (bool roleMatches = TaskAssignmentService::roleMatchesSDLCStage(employeePosition, projectPhase);
            !roleMatches) {
            throw CompanyException(QString("Employee role '%1' does not "
                                           "match project SDLC stage '%2'")
                                       .arg(employeePosition)
                                       .arg(projectPhase));
        }

        auto taskType = task.getType();
        auto employeeType = employee->getEmployeeType();
        if (bool taskTypeMatches = TaskAssignmentService::taskTypeMatchesEmployeeType(taskType, employeeType);
            !taskTypeMatches) {
            QString requiredType = getRequiredEmployeeTypeForTask(taskType);
            throw CompanyException(
                QString(
                    "Employee type '%1' does not match task type '%2'.\n"
                    "Task type '%2' requires employee type '%3'.")
                    .arg(employeeType)
                    .arg(taskType)
                    .arg(requiredType));
        }
    }
    
    void validateBudgetConstraintsForAssignment(
        const std::shared_ptr<Employee>& employee,
        Project* projPtr,
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

        if (double projectEstimatedHours = projPtr->getEstimatedHours();
            projectEstimatedHours > 0) {
            double averageBudgetPerHour = projPtr->getBudget() / projectEstimatedHours;
            double maxAffordableHourlyRate = averageBudgetPerHour * kMaxAffordableHourlyRateMultiplier;

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
    
    void performTaskAssignment(
        const std::shared_ptr<Employee>& employee,
        Task& task,
        int employeeId,
        int projectId,
        int taskId,
        int toAssign,
        Project* projPtr,
        double assignmentCost,
        Company* company) {
        employee->addWeeklyHours(toAssign);
        employee->addAssignedProject(projectId);
        task.addAllocatedHours(toAssign);
        company->addTaskAssignment(employeeId, projectId, taskId, toAssign);
        projPtr->addEmployeeCost(assignmentCost);
        projPtr->recomputeTotalsFromTasks();
    }
    
    void collectScaledAssignmentsForEmployee(
        int employeeId,
        double scaleFactor,
        const std::map<std::tuple<int, int, int>, int>& allAssignments,
        std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        int& totalScaledHours) {
        for (const auto& assignment : allAssignments) {
            const auto [empId, projectId, taskId] = assignment.first;
            if (empId != employeeId) continue;
            
            const int oldHours = assignment.second;
            auto scaledHours = static_cast<int>(std::round(oldHours * scaleFactor));
            if (scaledHours < 0) {
                scaledHours = 0;
            }
            
            assignmentsData.emplace_back(projectId, taskId, oldHours, scaledHours);
            totalScaledHours += scaledHours;
        }
    }
    
    void adjustScaledHoursToCapacity(
        std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        int capacity,
        int& totalScaledHours) {
        if (assignmentsData.empty() || totalScaledHours <= capacity) {
            return;
        }
        
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
            
            std::ranges::sort(assignmentsData, [](const auto& a, const auto& b) {
                const auto& [projectIdA, taskIdA, oldHoursA, adjustedHoursA] = a;
                const auto& [projectIdB, taskIdB, oldHoursB, adjustedHoursB] = b;
                return adjustedHoursA > adjustedHoursB;
            });
            
            for (auto& assignment : assignmentsData) {
                if (excess <= 0) break;
                auto& [projectId, taskId, oldHours, adjustedHours] = assignment;
                if (adjustedHours > 0) {
                    int reduction = std::min(excess, adjustedHours);
                    adjustedHours = adjustedHours - reduction;
                    totalScaledHours -= reduction;
                    excess -= reduction;
                }
            }
        }
    }
    
    void updateTaskAssignmentsFromScaledData(
        int employeeId,
        const std::vector<std::tuple<int, int, int, int>>& assignmentsData,
        Company* company,
        const std::shared_ptr<Employee>& employee) {
        for (const auto& assignment : assignmentsData) {
            const auto& [projectId, taskId, oldHours, newHours] = assignment;
            
            if (newHours > 0) {
                company->setTaskAssignment(employeeId, projectId, taskId, newHours);
            } else {
                company->removeTaskAssignment(employeeId, projectId, taskId);
            }
            
            Project* projPtr = company->getMutableProject(projectId);
            if (projPtr && employee) {
                std::vector<Task>& tasks = projPtr->getTasks();
                for (auto& task : tasks) {
                    if (task.getId() == taskId) {
                        double oldCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), oldHours);
                        double newCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), newHours);
                        double costDiff = newCost - oldCost;
                        if (costDiff != 0.0) {
                            projPtr->addEmployeeCost(costDiff);
                        }
                        break;
                    }
                }
            }
        }
    }
    
    void updateEmployeeHoursAfterScaling(
        const std::shared_ptr<Employee>& employee,
        int employeeId,
        Company* company) {
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
        
        if (int currentHours = employee->getCurrentWeeklyHours(); currentHours > 0) {
            try {
                employee->removeWeeklyHours(currentHours);
            } catch (const EmployeeException& e) {
                qCWarning(taskAssignmentService) << "Failed to remove weekly hours:" << e.what();
            }
        }
        
        if (totalHours > 0) {
            try {
                employee->addWeeklyHours(totalHours);
            } catch (const EmployeeException& e) {
                qCWarning(taskAssignmentService) << "Failed to add weekly hours:" << e.what();
            }
            
            if (totalHours > currentCapacity) {
                try {
                    employee->addWeeklyHours(currentCapacity);
                } catch (const EmployeeException& e) {
                    qCWarning(taskAssignmentService) << "Failed to add weekly hours:" << e.what();
                }
            }
        }
    }
    bool isEmployeeEligibleForTask(
        const std::shared_ptr<Employee>& employee,
        const QString& projectPhase,
        const QString& taskType,
        double projectBudget,
        double maxAffordableHourlyRate,
        double projectEstimatedHours,
        const std::map<int, int>& employeeUsage) {
        if (!employee) return false;
        if (!TaskAssignmentService::roleMatchesSDLCStage(employee->getPosition(), projectPhase)) return false;
        if (!TaskAssignmentService::taskTypeMatchesEmployeeType(taskType, employee->getEmployeeType())) return false;
        if (employee->getSalary() > projectBudget) return false;
        
        if (projectEstimatedHours > 0) {
            double employeeHourlyRate = CostCalculationService::calculateHourlyRate(employee->getSalary());
            if (employeeHourlyRate > maxAffordableHourlyRate) {
                return false;
            }
        }
        
        int available = employee->getAvailableHours();
        int employeeId = employee->getId();
        int alreadyUsed = 0;
        if (auto it = employeeUsage.find(employeeId); it != employeeUsage.end()) {
            alreadyUsed = it->second;
        }
        int trulyAvailable = available - alreadyUsed;
        return trulyAvailable > 0;
    }
    
    void buildEmployeePoolForTask(
        const std::vector<std::shared_ptr<Employee>>& employeesList,
        const QString& projectPhase,
        const QString& taskType,
        double projectBudget,
        double maxAffordableHourlyRate,
        double projectEstimatedHours,
        const std::map<int, int>& employeeUsage,
        std::vector<std::shared_ptr<Employee>>& pool) {
        for (const auto& employee : employeesList) {
            if (isEmployeeEligibleForTask(employee, projectPhase, taskType, projectBudget,
                                          maxAffordableHourlyRate, projectEstimatedHours,
                                          employeeUsage)) {
                pool.push_back(employee);
            }
        }
    }
    
    int calculateToAssignHours(
        int remaining,
        const std::shared_ptr<Employee>& poolEmployee,
        const std::map<int, int>& employeeUsage,
        double remainingBudget) {
        int employeeId = poolEmployee->getId();
        int trulyAvailable = poolEmployee->getAvailableHours();
        if (auto it = employeeUsage.find(employeeId); it != employeeUsage.end()) {
            trulyAvailable -= it->second;
        }
        if (trulyAvailable <= 0) return 0;
        
        const double hourlyRate = CostCalculationService::calculateHourlyRate(poolEmployee->getSalary());
        int maxAffordableHours = 0;
        if (hourlyRate > 0 && remainingBudget > 0) {
            maxAffordableHours = static_cast<int>(remainingBudget / hourlyRate);
        }
        
        int toAssign = remaining;
        if (trulyAvailable < toAssign) {
            toAssign = trulyAvailable;
        }
        if (maxAffordableHours < toAssign) {
            toAssign = maxAffordableHours;
        }
        if (toAssign <= 0) return 0;
        
        return toAssign;
    }
    
    bool canAffordAssignment(
        double assignmentCost,
        double currentEmployeeCosts,
        double projectBudget) {
        return currentEmployeeCosts + assignmentCost <= projectBudget;
    }
    
    void assignEmployeeToTaskInAutoAssign(
        const std::shared_ptr<Employee>& poolEmployee,
        Task& task,
        int projectId,
        int toAssign,
        std::map<int, int>& employeeUsage,
        double& currentEmployeeCosts,
        double& remainingBudget,
        int& remaining,
        Company* company) {
        poolEmployee->addWeeklyHours(toAssign);
        poolEmployee->addAssignedProject(projectId);
        task.addAllocatedHours(toAssign);
        employeeUsage[poolEmployee->getId()] += toAssign;
        
        company->addTaskAssignment(poolEmployee->getId(), projectId, task.getId(), toAssign);
        double assignmentCost = CostCalculationService::calculateEmployeeCost(poolEmployee->getSalary(), toAssign);
        currentEmployeeCosts += assignmentCost;
        remainingBudget -= assignmentCost;
        remaining -= toAssign;
    }
    
    void clearProjectEmployeeCosts(const std::vector<Project>& allProjects, Company* company) {
        for (const auto& project : allProjects) {
            Project* mutableProject = company->getMutableProject(project.getId());
            if (!mutableProject) continue;
            
            double currentCosts = mutableProject->getEmployeeCosts();
            if (currentCosts > 0) {
                mutableProject->removeEmployeeCost(currentCosts);
            }
        }
    }
    
    void calculateTaskAllocatedHoursAndCost(
        Task& task,
        int projectId,
        int taskId,
        Company* company,
        double& projectTotalCosts) {
        int totalAllocated = 0;
        auto allEmployees = company->getAllEmployees();
        
        for (const auto& employee : allEmployees) {
            if (!employee || !employee->isAssignedToProject(projectId)) {
                continue;
            }
            
            int hours = company->getTaskAssignment(employee->getId(), projectId, taskId);
            if (hours > 0) {
                totalAllocated += hours;
                double cost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), hours);
                projectTotalCosts += cost;
            }
        }
        
        task.setAllocatedHours(totalAllocated);
    }
    
    void recalculateProjectTasks(
        Project* mutableProject,
        int projectId,
        Company* company) {
        if (!mutableProject) return;
        
        std::vector<Task>& tasks = mutableProject->getTasks();
        double projectTotalCosts = 0.0;
        
        for (auto& task : tasks) {
            int taskId = task.getId();
            calculateTaskAllocatedHoursAndCost(task, projectId, taskId, company, projectTotalCosts);
        }
        
        if (projectTotalCosts > 0) {
            mutableProject->addEmployeeCost(projectTotalCosts);
        }
        
        mutableProject->recomputeTotalsFromTasks();
    }
}

void TaskAssignmentService::assignEmployeeToTask(int employeeId, int projectId, int taskId,
                                                 int hours) {
    auto employee = company->getEmployee(employeeId);
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

    Project* projPtr = company->getMutableProject(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    auto projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    Task* targetTask = nullptr;
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
            targetTask = &task;
            break;
        }
    }
    
    if (!targetTask) {
        throw CompanyException("Task not found");
    }
    
    validateTaskAssignmentDetails(employee, *targetTask, hours, projectPhase);
    
    int needed = targetTask->getEstimatedHours() - targetTask->getAllocatedHours();
    if (needed <= 0) {
        throw CompanyException("Task already fully allocated");
    }
    
    if (hours <= 0) {
        return;
    }
    
    int toAssign = needed < hours ? needed : hours;
    
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
    
    double employeeHourlyRate = CostCalculationService::calculateHourlyRate(employee->getSalary());
    double assignmentCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), toAssign);
    
    validateBudgetConstraintsForAssignment(employee, projPtr, toAssign, employeeHourlyRate, assignmentCost);
    performTaskAssignment(employee, *targetTask, employeeId, projectId, taskId, toAssign,
                         projPtr, assignmentCost, company);
}

int TaskAssignmentService::getEmployeeTaskHours(int employeeId, int projectId,
                                                int taskId) const {
    return company->getTaskAssignment(employeeId, projectId, taskId);
}

int TaskAssignmentService::getEmployeeProjectHours(int employeeId, int projectId) const {
    auto employee = company->getEmployee(employeeId);
    if (!employee || !employee->isAssignedToProject(projectId)) {
        return 0;
    }

    int totalHours = 0;
    auto projectTasks = company->getProjectTasks(projectId);

    for (const auto& task : projectTasks) {
        totalHours += getEmployeeTaskHours(employeeId, projectId, task.getId());
    }

    
    if (const int capacity = employee->getWeeklyHoursCapacity(); totalHours > capacity) {
        totalHours = capacity;
    }

    return totalHours;
}

void TaskAssignmentService::updateTaskAndProjectCosts(Project* projPtr, int taskId,
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
            
            
            if (double costDiff = CostCalculationService::calculateEmployeeCost(employee->getSalary(), newHours) -
                            CostCalculationService::calculateEmployeeCost(employee->getSalary(), oldHours); costDiff < 0) {
                projPtr->removeEmployeeCost(-costDiff);
            } else {
                projPtr->addEmployeeCost(costDiff);
            }
            break;
        }
    }
}

void TaskAssignmentService::restoreTaskAssignment(int employeeId, int projectId, int taskId,
                                                   int hours) {
    auto employee = company->getEmployee(employeeId);
    if (!employee) return;  

    Project* projPtr = company->getMutableProject(projectId);
    if (!projPtr) return;  

    std::vector<Task>& tasks = projPtr->getTasks();
    for (auto& task : tasks) {
        if (task.getId() == taskId) {
            
            const int newHours = hours - company->getTaskAssignment(employeeId, projectId, taskId);

            
            
            company->setTaskAssignment(employeeId, projectId, taskId, hours);

            
            
            employee->addToProjectHistory(projectId);

            
            
            if (employee->getIsActive()) {
                
                employee->addAssignedProject(projectId);
            }
            // Update employee hours
            if (employee->getIsActive() && newHours > 0) {
                try {
                    employee->addWeeklyHours(newHours);
                } catch (const EmployeeException& e) {
                    qCWarning(taskAssignmentService) << "Failed to add weekly hours:" << e.what();
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
            company->removeTaskAssignment(employeeId, projectId, taskId);
        }
    }
}

void TaskAssignmentService::fixTaskAssignmentsToCapacity() {
    
    
    std::map<int, std::vector<std::tuple<int, int, int, int>>> employeeAssignments;
    
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& [key, hours] : allAssignments) {
        const auto& [employeeId, projectId, taskId] = key;
        
        employeeAssignments[employeeId].emplace_back(projectId, taskId, hours, 0);
    }
    
    
    for (auto& [employeeId, assignments] : employeeAssignments) {
        auto employee = company->getEmployee(employeeId);
        if (!employee) continue;
        
        const int capacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        for (const auto& assignment : assignments) {
            const auto& [projectId, taskId, oldHours, newHours] = assignment;
            totalHours += oldHours;
        }
        
        
        if (totalHours > capacity && totalHours > 0) {
            const auto scaleFactor = static_cast<double>(capacity) / totalHours;
            
            for (auto& assignment : assignments) {
                auto& [projectId, taskId, oldHours, newHours] = assignment;
                newHours = static_cast<int>(std::round(oldHours * scaleFactor));
                
                if (newHours < 0) newHours = 0;
                if (newHours > capacity) newHours = capacity;  
                
                
                company->setTaskAssignment(employeeId, projectId, taskId, newHours);
                
                
                Project* projPtr = company->getMutableProject(projectId);
                if (projPtr && employee) {
                    std::vector<Task>& tasks = projPtr->getTasks();
                    for (auto& task : tasks) {
                        if (task.getId() == taskId) {
                            double oldCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), oldHours);
                            double newCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), newHours);
                            double costDiff = newCost - oldCost;
                            if (costDiff != 0.0) {
                                projPtr->addEmployeeCost(costDiff);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
}

void TaskAssignmentService::recalculateTaskAllocatedHours() {
    fixTaskAssignmentsToCapacity();
    
    auto allProjects = company->getAllProjects();
    for (const auto& project : allProjects) {
        Project* mutableProject = company->getMutableProject(project.getId());
        if (!mutableProject) continue;
        
        double currentCosts = mutableProject->getEmployeeCosts();
        if (currentCosts > 0) {
            mutableProject->removeEmployeeCost(currentCosts);
        }
    }
    
    for (const auto& project : allProjects) {
        Project* mutableProject = company->getMutableProject(project.getId());
        if (!mutableProject) continue;
        
        int projectId = project.getId();
        std::vector<Task>& tasks = mutableProject->getTasks();
        double projectTotalCosts = 0.0;
        
        for (auto& task : tasks) {
            int taskId = task.getId();
            int totalAllocated = 0;
            auto allEmployees = company->getAllEmployees();
            
            for (const auto& employee : allEmployees) {
                if (!employee || !employee->isAssignedToProject(projectId)) {
                    continue;
                }
                
                int hours = company->getTaskAssignment(employee->getId(), projectId, taskId);
                if (hours > 0) {
                    totalAllocated += hours;
                    double cost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), hours);
                    projectTotalCosts += cost;
                }
            }
            
            task.setAllocatedHours(totalAllocated);
        }
        
        if (projectTotalCosts > 0) {
            mutableProject->addEmployeeCost(projectTotalCosts);
        }
        
        mutableProject->recomputeTotalsFromTasks();
    }
}

void TaskAssignmentService::scaleEmployeeTaskAssignments(int employeeId, double scaleFactor) {
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
    
    auto allAssignments = company->getAllTaskAssignments();
    collectScaledAssignmentsForEmployee(employeeId, scaleFactor, allAssignments,
                                       assignmentsData, totalScaledHours);

    if (assignmentsData.empty()) {
        return;
    }

    adjustScaledHoursToCapacity(assignmentsData, capacity, totalScaledHours);
    updateTaskAssignmentsFromScaledData(employeeId, assignmentsData, company, employee);
    recalculateTaskAllocatedHours();
    updateEmployeeHoursAfterScaling(employee, employeeId, company);
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
    return TaskAssignmentService::taskTypeMatchesEmployeeType(taskType, employeeType);
}

static int compareEmployeesForSorting(const std::shared_ptr<Employee>& a,
                                      const std::shared_ptr<Employee>& b,
                                      const std::map<int, int>& employeeUsage) {
    if (!a || !b) return 0;

    const double hourlyRateA = CostCalculationService::calculateHourlyRate(a->getSalary());
    const double hourlyRateB = CostCalculationService::calculateHourlyRate(b->getSalary());

    auto itA = employeeUsage.find(a->getId());
    auto itB = employeeUsage.find(b->getId());
    int usedA = (itA != employeeUsage.end()) ? itA->second : 0;
    int usedB = (itB != employeeUsage.end()) ? itB->second : 0;

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

void TaskAssignmentService::autoAssignEmployeesToProject(int projectId) {
    Project* projPtr = company->getMutableProject(projectId);
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

    auto allEmployees = company->getAllEmployees();
    std::vector<std::shared_ptr<Employee>> employeesList;
    for (const auto& emp : allEmployees) {
        if (emp && emp->getIsActive()) {
            employeesList.push_back(emp);
        }
    }

    std::map<int, int> employeeUsage;

    auto currentEmployeeCosts = projPtr->getEmployeeCosts();
    auto remainingBudget = projPtr->getBudget() - currentEmployeeCosts;

    for (const auto taskIndex : taskIndices) {
        Task& task = tasks[taskIndex];
        int remaining = task.getEstimatedHours() - task.getAllocatedHours();
        if (remaining <= 0) continue;

        double projectBudget = projPtr->getBudget();
        double projectEstimatedHours = projPtr->getEstimatedHours();
        double averageBudgetPerHour = 0.0;
        if (projectEstimatedHours > 0) {
            averageBudgetPerHour = projectBudget / projectEstimatedHours;
        }
        double maxAffordableHourlyRate = averageBudgetPerHour * 0.7;
        auto taskType = task.getType();

        std::vector<std::shared_ptr<Employee>> pool;
        buildEmployeePoolForTask(employeesList, projectPhase, taskType, projectBudget,
                                maxAffordableHourlyRate, projectEstimatedHours,
                                employeeUsage, pool);

        std::ranges::sort(pool,
                          [&employeeUsage](const std::shared_ptr<Employee>& a,
                                           const std::shared_ptr<Employee>& b) {
                              return compareEmployeesForSorting(a, b, employeeUsage) < 0;
                          });

        for (const auto& poolEmployee : pool) {
            if (remaining <= 0) break;
            
            int toAssign = calculateToAssignHours(remaining, poolEmployee, employeeUsage, remainingBudget);
            if (toAssign <= 0) continue;

            double assignmentCost = CostCalculationService::calculateEmployeeCost(poolEmployee->getSalary(), toAssign);
            if (!canAffordAssignment(assignmentCost, currentEmployeeCosts, projPtr->getBudget())) {
                continue;
            }

            assignEmployeeToTaskInAutoAssign(poolEmployee, task, projectId, toAssign,
                                            employeeUsage, currentEmployeeCosts,
                                            remainingBudget, remaining, company);
        }
    }

    double totalNewCosts = 0.0;
    int totalAssignedHours = 0;
    for (const auto& [employeeId, hours] : employeeUsage) {
        totalAssignedHours += hours;
        auto emp = company->getEmployee(employeeId);
        if (emp) {
            totalNewCosts += CostCalculationService::calculateEmployeeCost(emp->getSalary(), hours);
        }
    }
    projPtr->addEmployeeCost(totalNewCosts);
    projPtr->recomputeTotalsFromTasks();
}

bool TaskAssignmentService::validateAssignment(std::shared_ptr<Employee> employee,
                                               std::shared_ptr<Project> project,
                                               const Task& task, int hours) const {
    if (!employee || !project) return false;
    
    if (!employee->getIsActive()) return false;
    if (project->getPhase() == "Completed") return false;
    
    if (hours > task.getEstimatedHours()) return false;
    
    if (const QString employeePosition = employee->getPosition(); !roleMatchesSDLCStage(employeePosition, project->getPhase())) return false;
    
    const QString taskType = task.getType();
    if (const QString employeeType = employee->getEmployeeType(); !taskTypeMatchesEmployeeType(taskType, employeeType)) return false;
    
    if (!employee->isAvailable(hours)) return false;
    
    if (const double assignmentCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), hours); project->getEmployeeCosts() + assignmentCost > project->getBudget()) return false;
    
    return true;
}

