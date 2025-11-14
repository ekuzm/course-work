#include "services/task_assignment_service.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "utils/consts.h"
#include "utils/container_utils.h"
#include "exceptions/exceptions.h"
#include "services/cost_calculation_service.h"

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

void TaskAssignmentService::assignEmployeeToTask(int employeeId, int projectId, int taskId,
                                                 int hours) {
    std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
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

    QString projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    bool found = false;
    for (size_t i = 0; i < tasks.size(); ++i) {
        Task& task = tasks[i];
        if (task.getId() == taskId) {
            if (hours > task.getEstimatedHours()) {
                throw CompanyException(QString("Cannot assign %1 hours: task "
                                               "has only %2 estimated hours")
                                           .arg(hours)
                                           .arg(task.getEstimatedHours()));
            }

            QString employeePosition = employee->getPosition();
            bool roleMatches =
                roleMatchesSDLCStage(employeePosition, projectPhase);

            if (!roleMatches) {
                throw CompanyException(QString("Employee role '%1' does not "
                                               "match project SDLC stage '%2'")
                                           .arg(employeePosition)
                                           .arg(projectPhase));
            }

            QString taskType = task.getType();
            QString employeeType = employee->getEmployeeType();
            bool taskTypeMatches =
                taskTypeMatchesEmployeeType(taskType, employeeType);

            if (!taskTypeMatches) {
                QString requiredType;
                if (taskType == "Management") {
                    requiredType = "Manager";
                } else if (taskType == "Development") {
                    requiredType = "Developer";
                } else if (taskType == "Design") {
                    requiredType = "Designer";
                } else {
                    requiredType = "QA";
                }
                throw CompanyException(
                    QString(
                        "Employee type '%1' does not match task type '%2'.\n"
                        "Task type '%2' requires employee type '%3'.")
                        .arg(employeeType)
                        .arg(taskType)
                        .arg(requiredType));
            }

            
            int existingHours = company->getTaskAssignment(employeeId, projectId, taskId);

            int needed = task.getEstimatedHours() - task.getAllocatedHours();
            if (needed <= 0)
                throw CompanyException("Task already fully allocated");

            
            
            
            int newHoursToAssign = hours;
            
            if (newHoursToAssign <= 0) {
                
                return;
            }

            
            int toAssign =
                needed < newHoursToAssign ? needed : newHoursToAssign;

            if (!employee->isAvailable(toAssign)) {
                int availableHours = employee->getAvailableHours();
                int currentHours = employee->getCurrentWeeklyHours();
                int capacity = employee->getWeeklyHoursCapacity();
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

            double employeeHourlyRate =
                CostCalculationService::calculateHourlyRate(employee->getSalary());
            double assignmentCost =
                CostCalculationService::calculateEmployeeCost(employee->getSalary(), toAssign);
            double currentEmployeeCosts = projPtr->getEmployeeCosts();
            double remainingBudget =
                projPtr->getBudget() - currentEmployeeCosts;

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
                double averageBudgetPerHour =
                    projPtr->getBudget() / projectEstimatedHours;
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

            employee->addWeeklyHours(toAssign);
            employee->addAssignedProject(projectId);
            task.addAllocatedHours(toAssign);

            
            company->addTaskAssignment(employeeId, projectId, taskId, toAssign);

            projPtr->addEmployeeCost(assignmentCost);

            projPtr->recomputeTotalsFromTasks();
            found = true;
            break;
        }
    }
    if (!found) throw CompanyException("Task not found");
}

int TaskAssignmentService::getEmployeeTaskHours(int employeeId, int projectId,
                                                int taskId) const {
    return company->getTaskAssignment(employeeId, projectId, taskId);
}

int TaskAssignmentService::getEmployeeProjectHours(int employeeId, int projectId) const {
    std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
    if (!employee || !employee->isAssignedToProject(projectId)) {
        return 0;
    }

    int totalHours = 0;
    auto projectTasks = company->getProjectTasks(projectId);

    for (const auto& task : projectTasks) {
        totalHours += getEmployeeTaskHours(employeeId, projectId, task.getId());
    }

    
    int capacity = employee->getWeeklyHoursCapacity();
    if (totalHours > capacity) {
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
            
            
            double costDiff = CostCalculationService::calculateEmployeeCost(employee->getSalary(), newHours) -
                            CostCalculationService::calculateEmployeeCost(employee->getSalary(), oldHours);
            if (costDiff < 0) {
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
    std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
    if (!employee) return;  

    Project* projPtr = company->getMutableProject(projectId);
    if (!projPtr) return;  

    std::vector<Task>& tasks = projPtr->getTasks();
    for (size_t i = 0; i < tasks.size(); ++i) {
        Task& task = tasks[i];
        if (task.getId() == taskId) {
            
            int existingHours = company->getTaskAssignment(employeeId, projectId, taskId);

            
            int newHours = hours - existingHours;

            
            
            company->setTaskAssignment(employeeId, projectId, taskId, hours);

            
            
            employee->addToProjectHistory(projectId);

            
            
            if (employee->getIsActive()) {
                
                employee->addAssignedProject(projectId);
            }

            
            
            if (employee->getIsActive() && newHours > 0) {
                try {
                    employee->addWeeklyHours(newHours);
                } catch (const EmployeeException&) {
                    
                    
                }
            }

            break;
        }
    }
}

void TaskAssignmentService::removeEmployeeTaskAssignments(int employeeId) {
    
    auto allAssignments = company->getAllTaskAssignments();
    
    for (const auto& assignment : allAssignments) {
        int empId = std::get<0>(assignment.first);
        if (empId == employeeId) {
            int projectId = std::get<1>(assignment.first);
            int taskId = std::get<2>(assignment.first);
            company->removeTaskAssignment(employeeId, projectId, taskId);
        }
    }
}

void TaskAssignmentService::fixTaskAssignmentsToCapacity() {
    
    
    std::map<int, std::vector<std::tuple<int, int, int, int>>> employeeAssignments;
    
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& assignment : allAssignments) {
        int employeeId = std::get<0>(assignment.first);
        int projectId = std::get<1>(assignment.first);
        int taskId = std::get<2>(assignment.first);
        int hours = assignment.second;
        
        employeeAssignments[employeeId].push_back(
            std::make_tuple(projectId, taskId, hours, 0));
    }
    
    
    for (auto& empAssignments : employeeAssignments) {
        int employeeId = empAssignments.first;
        std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
        if (!employee) continue;
        
        int capacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        for (auto& assignment : empAssignments.second) {
            totalHours += std::get<2>(assignment);
        }
        
        
        if (totalHours > capacity && totalHours > 0) {
            double scaleFactor = static_cast<double>(capacity) / totalHours;
            
            for (auto& assignment : empAssignments.second) {
                int projectId = std::get<0>(assignment);
                int taskId = std::get<1>(assignment);
                int oldHours = std::get<2>(assignment);
                int newHours = static_cast<int>(std::round(oldHours * scaleFactor));
                
                if (newHours < 0) newHours = 0;
                if (newHours > capacity) newHours = capacity;
                
                std::get<3>(assignment) = newHours;  
                
                
                company->setTaskAssignment(employeeId, projectId, taskId, newHours);
                
                
                Project* projPtr = company->getMutableProject(projectId);
                updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
            }
        }
    }
}

void TaskAssignmentService::recalculateTaskAllocatedHours() {
    
    fixTaskAssignmentsToCapacity();
    
    
    auto allProjects = company->getAllProjects();
    for (const auto& project : allProjects) {
        Project* mutableProject = company->getMutableProject(project.getId());
        if (mutableProject) {
            
            double currentCosts = mutableProject->getEmployeeCosts();
            if (currentCosts > 0) {
                mutableProject->removeEmployeeCost(currentCosts);
            }
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
                if (employee && employee->isAssignedToProject(projectId)) {
                    
                    int hours = company->getTaskAssignment(employee->getId(), projectId, taskId);
                    if (hours > 0) {
                        totalAllocated += hours;
                        
                        
                        double cost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), hours);
                        projectTotalCosts += cost;
                    }
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

    std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
    if (!employee) {
        return;
    }

    int capacity = employee->getWeeklyHoursCapacity();

    
    std::vector<std::tuple<int, int, int, int>> assignmentsData;
    int totalScaledHours = 0;
    
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& assignment : allAssignments) {
        int empId = std::get<0>(assignment.first);
        if (empId == employeeId) {
            int projectId = std::get<1>(assignment.first);
            int taskId = std::get<2>(assignment.first);
            int oldHours = assignment.second;
            int scaledHours = static_cast<int>(std::round(oldHours * scaleFactor));
            
            if (scaledHours < 0) {
                scaledHours = 0;
            }
            
            assignmentsData.push_back(std::make_tuple(projectId, taskId, oldHours, scaledHours));
            totalScaledHours += scaledHours;
        }
    }

    
    if (totalScaledHours > capacity && assignmentsData.size() > 0) {
        double adjustFactor = static_cast<double>(capacity) / totalScaledHours;
        totalScaledHours = 0;
        
        for (auto& assignment : assignmentsData) {
            int scaledHours = std::get<3>(assignment);
            int adjustedHours = static_cast<int>(std::round(scaledHours * adjustFactor));
            
            if (adjustedHours < 0) adjustedHours = 0;
            if (adjustedHours > capacity) adjustedHours = capacity;
            
            std::get<3>(assignment) = adjustedHours;
            totalScaledHours += adjustedHours;
        }
        
        
        if (totalScaledHours > capacity) {
            int excess = totalScaledHours - capacity;
            
            std::sort(assignmentsData.begin(), assignmentsData.end(),
                [](const std::tuple<int, int, int, int>& a, const std::tuple<int, int, int, int>& b) {
                    return std::get<3>(a) > std::get<3>(b);
                });
            
            for (auto& assignment : assignmentsData) {
                if (excess <= 0) break;
                int adjustedHours = std::get<3>(assignment);
                if (adjustedHours > 0) {
                    int reduction = std::min(excess, adjustedHours);
                    std::get<3>(assignment) = adjustedHours - reduction;
                    totalScaledHours -= reduction;
                    excess -= reduction;
                }
            }
        }
    }

    
    for (const auto& assignment : assignmentsData) {
        int projectId = std::get<0>(assignment);
        int taskId = std::get<1>(assignment);
        int oldHours = std::get<2>(assignment);
        int newHours = std::get<3>(assignment);

        
        if (newHours > 0) {
            company->setTaskAssignment(employeeId, projectId, taskId, newHours);
        } else {
            company->removeTaskAssignment(employeeId, projectId, taskId);
        }

        
        Project* projPtr = company->getMutableProject(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
    }

    
    recalculateTaskAllocatedHours();

    
    if (employee->getIsActive()) {
        
        int currentCapacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        auto allAssignments = company->getAllTaskAssignments();
        for (const auto& assignment : allAssignments) {
            int empId = std::get<0>(assignment.first);
            if (empId == employeeId) {
                totalHours += assignment.second;
            }
        }
        
        
        if (totalHours > currentCapacity) {
            totalHours = currentCapacity;
        }
        
        
        
        int currentHours = employee->getCurrentWeeklyHours();
        if (currentHours > 0) {
            try {
                employee->removeWeeklyHours(currentHours);
            } catch (const EmployeeException&) {
                
            }
        }
        
        
        if (totalHours > 0) {
            try {
                employee->addWeeklyHours(totalHours);
            } catch (const EmployeeException&) {
                
                
                try {
                    if (totalHours > currentCapacity) {
                        employee->addWeeklyHours(currentCapacity);
                    }
                } catch (const EmployeeException&) {
                    
                }
            }
        }
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

    double hourlyRateA = CostCalculationService::calculateHourlyRate(a->getSalary());
    double hourlyRateB = CostCalculationService::calculateHourlyRate(b->getSalary());

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

    QString projectPhase = projPtr->getPhase();
    if (projectPhase == "Completed") {
        throw CompanyException("Cannot auto-assign to project with phase: " +
                               projectPhase);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    if (tasks.empty()) throw CompanyException("No tasks in project");

    std::vector<size_t> taskIndices;
    for (size_t i = 0; i < tasks.size(); ++i) {
        taskIndices.push_back(i);
    }

    std::sort(taskIndices.begin(), taskIndices.end(),
              [&tasks](size_t a, size_t b) {
                  return compareTaskPriority(tasks[a], tasks[b]) < 0;
              });

    std::vector<std::shared_ptr<Employee>> allEmployees = company->getAllEmployees();
    std::vector<std::shared_ptr<Employee>> employeesList;
    for (size_t i = 0; i < allEmployees.size(); ++i) {
        if (allEmployees[i] && allEmployees[i]->getIsActive()) {
            employeesList.push_back(allEmployees[i]);
        }
    }

    std::map<int, int> employeeUsage;

    double currentEmployeeCosts = projPtr->getEmployeeCosts();
    double remainingBudget = projPtr->getBudget() - currentEmployeeCosts;

    for (size_t idx = 0; idx < taskIndices.size(); ++idx) {
        size_t taskIndex = taskIndices[idx];
        Task& task = tasks[taskIndex];
        int remaining = task.getEstimatedHours() - task.getAllocatedHours();

        if (remaining <= 0) continue;

        std::vector<std::shared_ptr<Employee>> pool;
        double projectBudget = projPtr->getBudget();
        double projectEstimatedHours = projPtr->getEstimatedHours();
        double averageBudgetPerHour = 0.0;
        if (projectEstimatedHours > 0) {
            averageBudgetPerHour = projectBudget / projectEstimatedHours;
        }
        double maxAffordableHourlyRate = averageBudgetPerHour * 0.7;

        QString taskType = task.getType();

        for (size_t i = 0; i < employeesList.size(); ++i) {
            std::shared_ptr<Employee> employee = employeesList[i];
            if (!employee) continue;
            if (!employeeRoleMatchesSDLC(employee, projectPhase)) continue;

            if (!employeeTaskTypeMatches(employee, taskType)) {
                continue;
            }

            if (employee->getSalary() > projectBudget) {
                continue;
            }

            if (projectEstimatedHours > 0) {
                double employeeHourlyRate =
                    CostCalculationService::calculateHourlyRate(employee->getSalary());
                if (employeeHourlyRate > maxAffordableHourlyRate) {
                    continue;
                }
            }

            int available = employee->getAvailableHours();
            int employeeId = employee->getId();
            int alreadyUsed = 0;
            if (employeeUsage.find(employeeId) != employeeUsage.end()) {
                alreadyUsed = employeeUsage[employeeId];
            }
            int trulyAvailable = available - alreadyUsed;
            if (trulyAvailable > 0) {
                pool.push_back(employee);
            }
        }

        std::sort(pool.begin(), pool.end(),
                  [&employeeUsage](const std::shared_ptr<Employee>& a,
                                   const std::shared_ptr<Employee>& b) {
                      return compareEmployeesForSorting(a, b, employeeUsage) <
                             0;
                  });

        for (size_t i = 0; i < pool.size(); ++i) {
            if (remaining <= 0) break;

            std::shared_ptr<Employee> poolEmployee = pool[i];
            int employeeId = poolEmployee->getId();

            int trulyAvailable = poolEmployee->getAvailableHours();
            if (employeeUsage.find(employeeId) != employeeUsage.end()) {
                trulyAvailable -= employeeUsage[employeeId];
            }
            if (trulyAvailable <= 0) continue;

            double hourlyRate = CostCalculationService::calculateHourlyRate(poolEmployee->getSalary());
            int maxAffordableHours = 0;
            if (hourlyRate > 0 && remainingBudget > 0) {
                maxAffordableHours =
                    static_cast<int>(remainingBudget / hourlyRate);
            }

            int toAssign = remaining;
            if (trulyAvailable < toAssign) {
                toAssign = trulyAvailable;
            }
            if (maxAffordableHours < toAssign) {
                toAssign = maxAffordableHours;
            }
            if (toAssign <= 0) continue;

            double assignmentCost =
                CostCalculationService::calculateEmployeeCost(poolEmployee->getSalary(), toAssign);
            if (currentEmployeeCosts + assignmentCost > projPtr->getBudget()) {
                continue;
            }

            poolEmployee->addWeeklyHours(toAssign);
            poolEmployee->addAssignedProject(projectId);
            task.addAllocatedHours(toAssign);
            employeeUsage[employeeId] += toAssign;

            company->addTaskAssignment(employeeId, projectId, task.getId(), toAssign);
            currentEmployeeCosts += assignmentCost;
            remainingBudget -= assignmentCost;
            remaining -= toAssign;
        }
    }

    double totalNewCosts = 0.0;
    int totalAssignedHours = 0;
    for (std::map<int, int>::const_iterator it = employeeUsage.begin();
         it != employeeUsage.end(); ++it) {
        int employeeId = it->first;
        int hours = it->second;
        totalAssignedHours += hours;
        std::shared_ptr<Employee> emp = company->getEmployee(employeeId);
        if (emp) {
            totalNewCosts += CostCalculationService::calculateEmployeeCost(emp->getSalary(), hours);
        }
    }
    projPtr->addEmployeeCost(totalNewCosts);
    projPtr->recomputeTotalsFromTasks();
}

bool TaskAssignmentService::validateAssignment(std::shared_ptr<Employee> employee,
                                               std::shared_ptr<Project> project,
                                               const Task& task, int hours) {
    if (!employee || !project) return false;
    
    if (!employee->getIsActive()) return false;
    if (project->getPhase() == "Completed") return false;
    
    if (hours > task.getEstimatedHours()) return false;
    
    QString employeePosition = employee->getPosition();
    if (!roleMatchesSDLCStage(employeePosition, project->getPhase())) return false;
    
    QString taskType = task.getType();
    QString employeeType = employee->getEmployeeType();
    if (!taskTypeMatchesEmployeeType(taskType, employeeType)) return false;
    
    if (!employee->isAvailable(hours)) return false;
    
    double hourlyRate = CostCalculationService::calculateHourlyRate(employee->getSalary());
    double assignmentCost = CostCalculationService::calculateEmployeeCost(employee->getSalary(), hours);
    if (project->getEmployeeCosts() + assignmentCost > project->getBudget()) return false;
    
    return true;
}

