#include "entities/company.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <tuple>
#include <utility>

#include "utils/consts.h"
#include "utils/container_utils.h"
#include "exceptions/exceptions.h"

static double calculateHourlyRate(double monthlySalary) {
    if (kHoursPerMonth <= 0) return 0.0;
    return monthlySalary / kHoursPerMonth;
}

static double calculateEmployeeCost(double monthlySalary, int hours) {
    return calculateHourlyRate(monthlySalary) * hours;
}

namespace {
    
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
                
                
                if (double costDiff = calculateEmployeeCost(employee->getSalary(), newHours) -
                                calculateEmployeeCost(employee->getSalary(), oldHours); costDiff < 0) {
                    projPtr->removeEmployeeCost(-costDiff);
                } else {
                    projPtr->addEmployeeCost(costDiff);
                }
                break;
            }
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
    employees.push_back(employee);
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
    for (size_t i = 0; i < employees.size(); ++i) {
        if (matchesEmployeeId(employees[i], employeeId)) {
            return employees[i];
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<Employee>> EmployeeContainer::getAll() const {
    return employees;
}

size_t EmployeeContainer::size() const { return employees.size(); }

void ProjectContainer::add(std::shared_ptr<Project> project) {
    projects.push_back(project);
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
    for (size_t i = 0; i < projects.size(); ++i) {
        if (matchesProjectId(projects[i], projectId)) {
            return projects[i];
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
        for (size_t i = 0; i < assignedProjects.size(); ++i) {
            int projectId = assignedProjects[i];
            std::shared_ptr<Project> projPtr = projects.find(projectId);
            if (projPtr) {
                auto projectTasks = getProjectTasks(projectId);
                for (const auto& task : projectTasks) {
                    std::tuple<int, int, int> key =
                        std::make_tuple(employeeId, projectId, task.getId());
                    taskAssignments.erase(key);
                }
                projPtr->recomputeTotalsFromTasks();
            }
        }
    }

    auto it = taskAssignments.begin();
    while (it != taskAssignments.end()) {
        if (std::get<0>(it->first) == employeeId) {
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
        if (std::get<1>(it->first) == projectId) {
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

std::vector<Project> Company::getAllProjects() const {
    std::vector<Project> projectList;
    std::vector<std::shared_ptr<Project>> allProjects = projects.getAll();
    for (size_t i = 0; i < allProjects.size(); ++i) {
        if (allProjects[i]) {
            projectList.push_back(*allProjects[i]);
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
    std::vector<std::shared_ptr<Employee>> allEmployees = employees.getAll();
    size_t activeCount = 0;
    for (size_t i = 0; i < allEmployees.size(); ++i) {
        if (isEmployeeActive(allEmployees[i])) {
            activeCount++;
        }
    }

    double total = 0.0;
    for (size_t i = 0; i < allEmployees.size(); ++i) {
        if (allEmployees[i]) {
            total += allEmployees[i]->getSalary();
        }
    }
    return total;
}

double Company::getTotalBudget() const {
    double total = 0.0;
    std::vector<std::shared_ptr<Project>> allProjects = projects.getAll();
    for (size_t i = 0; i < allProjects.size(); ++i) {
        if (allProjects[i]) {
            total += allProjects[i]->getBudget();
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

static bool roleMatchesSDLCStage(const QString& employeePosition,
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

static bool taskTypeMatchesEmployeeType(const QString& taskType,
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

            
            std::tuple<int, int, int> key =
                std::make_tuple(employeeId, projectId, taskId);
            int existingHours = 0;
            auto it = taskAssignments.find(key);
            if (it != taskAssignments.end()) {
                existingHours = it->second;
            }

            int needed = task.getEstimatedHours() - task.getAllocatedHours();
            if (needed <= 0)
                throw CompanyException("Task already fully allocated");

            
            
            
            int newHoursToAssign;
            if (existingHours > 0) {
                
                newHoursToAssign = hours;
            } else {
                
                newHoursToAssign = hours;
            }
            
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
                calculateHourlyRate(employee->getSalary());
            double assignmentCost =
                calculateEmployeeCost(employee->getSalary(), toAssign);
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

    std::vector<Task>& tasks = projPtr->getTasks();
    for (size_t i = 0; i < tasks.size(); ++i) {
        Task& task = tasks[i];
        if (task.getId() == taskId) {
            
            std::tuple<int, int, int> key =
                std::make_tuple(employeeId, projectId, taskId);
            int existingHours = 0;
            auto it = taskAssignments.find(key);
            if (it != taskAssignments.end()) {
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
                } catch (const EmployeeException&) {
                    
                    // Ignore exception
                }
            }

            break;
        }
    }
}

void Company::removeEmployeeTaskAssignments(int employeeId) {
    
    auto it = taskAssignments.begin();
    while (it != taskAssignments.end()) {
        if (std::get<0>(it->first) == employeeId) {
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
            
            int currentHours = emp->getCurrentWeeklyHours();
            if (currentHours > 0) {
                try {
                    emp->removeWeeklyHours(currentHours);
                } catch (const EmployeeException&) {
                    
                    // Ignore exception
                }
            }
        }
    }

    
    for (const auto& assignment : taskAssignments) {
        auto [employeeId, projectId, taskId] = assignment.first;
        int hours = assignment.second;

        std::shared_ptr<Employee> employee = getEmployee(employeeId);
        if (employee && employee->getIsActive() && hours > 0) {
            try {
                employee->addWeeklyHours(hours);
            } catch (const EmployeeException&) {
                
                    // Ignore exception
            }
        }
    }
}

void Company::fixTaskAssignmentsToCapacity() {
    
    
    std::map<int, std::vector<std::tuple<int, int, int, int*>>> employeeAssignments;
    
    for (auto& assignment : taskAssignments) {
        auto [employeeId, projectId, taskId] = assignment.first;
        
        employeeAssignments[employeeId].push_back(
            std::make_tuple(projectId, taskId, assignment.second, &assignment.second));
    }
    
    
    for (auto& [employeeId, assignments] : employeeAssignments) {
        std::shared_ptr<Employee> employee = getEmployee(employeeId);
        if (!employee) continue;
        
        int capacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        for (const auto& assignment : assignments) {
            auto [projectId, taskId, oldHours, hoursPtr] = assignment;
            totalHours += oldHours;
        }
        
        
        if (totalHours > capacity && totalHours > 0) {
            double scaleFactor = static_cast<double>(capacity) / totalHours;
            
            for (auto& assignment : assignments) {
                auto& [projectId, taskId, oldHours, hoursPtr] = assignment;
                auto newHours = static_cast<int>(std::round(oldHours * scaleFactor));
                
                if (newHours < 0) newHours = 0;
                if (newHours > capacity) newHours = capacity;
                
                
                *hoursPtr = newHours;
                
                
                std::shared_ptr<Project> projPtr = projects.find(projectId);
                updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
            }
        }
    }
}

void Company::recalculateTaskAllocatedHours() {
    
    fixTaskAssignmentsToCapacity();
    
    
    auto allProjects = getAllProjects();
    for (const auto& project : allProjects) {
        auto mutableProject = const_cast<Project*>(getProject(project.getId()));
        if (mutableProject) {
            
            double currentCosts = mutableProject->getEmployeeCosts();
            if (currentCosts > 0) {
                mutableProject->removeEmployeeCost(currentCosts);
            }
        }
    }
    
    
    
    for (const auto& project : allProjects) {
        auto mutableProject = const_cast<Project*>(getProject(project.getId()));
        if (!mutableProject) continue;
        
        int projectId = project.getId();
        std::vector<Task>& tasks = mutableProject->getTasks();
        double projectTotalCosts = 0.0;
        
        for (auto& task : tasks) {
            int taskId = task.getId();
            
            
            int totalAllocated = 0;
            auto allEmployees = getAllEmployees();
            
            for (const auto& employee : allEmployees) {
                if (employee && employee->isAssignedToProject(projectId)) {
                    
                    std::tuple<int, int, int> key = std::make_tuple(
                        employee->getId(), projectId, taskId);
                    auto assignmentIt = taskAssignments.find(key);
                    if (assignmentIt != taskAssignments.end()) {
                        int hours = assignmentIt->second;
                        totalAllocated += hours;
                        
                        
                        double cost = calculateEmployeeCost(employee->getSalary(), hours);
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
    return roleMatchesSDLCStage(pos, projectPhase);
}

static bool employeeTaskTypeMatches(const std::shared_ptr<Employee>& employee,
                                    const QString& taskType) {
    if (!employee) return false;
    QString employeeType = employee->getEmployeeType();
    return taskTypeMatchesEmployeeType(taskType, employeeType);
}

static int compareEmployeesForSorting(const std::shared_ptr<Employee>& a,
                                      const std::shared_ptr<Employee>& b,
                                      const std::map<int, int>& employeeUsage) {
    if (!a || !b) return 0;

    double hourlyRateA = calculateHourlyRate(a->getSalary());
    double hourlyRateB = calculateHourlyRate(b->getSalary());

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

void Company::autoAssignEmployeesToProject(int projectId) {
    std::shared_ptr<Project> projPtr = projects.find(projectId);
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

    std::vector<std::shared_ptr<Employee>> allEmployees = getAllEmployees();
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
                    calculateHourlyRate(employee->getSalary());
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

            double hourlyRate = calculateHourlyRate(poolEmployee->getSalary());
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
                calculateEmployeeCost(poolEmployee->getSalary(), toAssign);
            if (currentEmployeeCosts + assignmentCost > projPtr->getBudget()) {
                continue;
            }

            poolEmployee->addWeeklyHours(toAssign);
            poolEmployee->addAssignedProject(projectId);
            task.addAllocatedHours(toAssign);
            employeeUsage[employeeId] += toAssign;

            std::tuple<int, int, int> key =
                std::make_tuple(employeeId, projectId, task.getId());
            taskAssignments[key] += toAssign;
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
        std::shared_ptr<Employee> emp = getEmployee(employeeId);
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
    std::tuple<int, int, int> key =
        std::make_tuple(employeeId, projectId, taskId);
    auto it = taskAssignments.find(key);
    if (it != taskAssignments.end()) {
        return it->second;
    }
    return 0;
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
    
    for (const auto& assignment : taskAssignments) {
        auto [empId, projectId, taskId] = assignment.first;
        if (empId == employeeId) {
            int oldHours = assignment.second;
            auto scaledHours = static_cast<int>(std::round(oldHours * scaleFactor));
            
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
            auto& [projectId, taskId, oldHours, scaledHours] = assignment;
            auto adjustedHours = static_cast<int>(std::round(scaledHours * adjustFactor));
            
            if (adjustedHours < 0) adjustedHours = 0;
            if (adjustedHours > capacity) adjustedHours = capacity;
            
            scaledHours = adjustedHours;
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

    
    for (const auto& assignment : assignmentsData) {
        auto [projectId, taskId, oldHours, newHours] = assignment;

        std::tuple<int, int, int> key =
            std::make_tuple(employeeId, projectId, taskId);
        
        
        if (newHours > 0) {
            taskAssignments[key] = newHours;
        } else {
            taskAssignments.erase(key);
        }

        
        std::shared_ptr<Project> projPtr = projects.find(projectId);
        updateTaskAndProjectCosts(projPtr, taskId, oldHours, newHours, employee);
    }

    
    recalculateTaskAllocatedHours();

    
    if (employee->getIsActive()) {
        
        int currentCapacity = employee->getWeeklyHoursCapacity();
        
        
        int totalHours = 0;
        for (const auto& assignment : taskAssignments) {
            auto [empId, projectId, taskId] = assignment.first;
            if (empId == employeeId) {
                totalHours += assignment.second;
            }
        }
        
        
        if (totalHours > currentCapacity) {
            totalHours = currentCapacity;
        }
        
        
        
        if (int currentHours = employee->getCurrentWeeklyHours(); currentHours > 0) {
            try {
                employee->removeWeeklyHours(currentHours);
            } catch (const EmployeeException&) {
                
                    // Ignore exception
            }
        }
        
        
        if (totalHours > 0) {
            try {
                employee->addWeeklyHours(totalHours);
            } catch (const EmployeeException&) {
                
                    // Ignore exception
            }
            
            try {
                    if (totalHours > currentCapacity) {
                        employee->addWeeklyHours(currentCapacity);
                    }
                } catch (const EmployeeException&) {
                    
                    // Ignore exception
                }
            }
        }
    }
}

int Company::getTaskAssignment(int employeeId, int projectId, int taskId) const {
    std::tuple<int, int, int> key = std::make_tuple(employeeId, projectId, taskId);
    if (auto it = taskAssignments.find(key); it != taskAssignments.end()) {
        return it->second;
    }
    return 0;
}

void Company::setTaskAssignment(int employeeId, int projectId, int taskId, int hours) {
    std::tuple<int, int, int> key = std::make_tuple(employeeId, projectId, taskId);
    if (hours > 0) {
        taskAssignments[key] = hours;
    } else {
        taskAssignments.erase(key);
    }
}

void Company::addTaskAssignment(int employeeId, int projectId, int taskId, int hours) {
    std::tuple<int, int, int> key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments[key] += hours;
}

void Company::removeTaskAssignment(int employeeId, int projectId, int taskId) {
    std::tuple<int, int, int> key = std::make_tuple(employeeId, projectId, taskId);
    taskAssignments.erase(key);
}

std::map<std::tuple<int, int, int>, int> Company::getAllTaskAssignments() const {
    return taskAssignments;
}

Project* Company::getMutableProject(int projectId) {
    return const_cast<Project*>(getProject(projectId));
}
