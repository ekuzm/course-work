#include "company.h"

#include <algorithm>
#include <map>
#include <utility>

#include "consts.h"
#include "container_utils.h"

static double calculateHourlyRate(double monthlySalary) {
    const double HOURS_PER_MONTH = 160.0;
    if (HOURS_PER_MONTH <= 0) return 0.0;
    return monthlySalary / HOURS_PER_MONTH;
}

static double calculateEmployeeCost(double monthlySalary, int hours) {
    return calculateHourlyRate(monthlySalary) * hours;
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
                projPtr->recomputeTotalsFromTasks();
            }
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

void Company::removeProject(int projectId) { projects.remove(projectId); }

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
                                 const QString& projectStatus) {
    if (projectStatus == "Analysis" || projectStatus == "Planning") {
        return employeePosition == "Manager";
    }
    if (projectStatus == "Design") {
        return employeePosition == "Designer";
    }
    if (projectStatus == "Development") {
        return employeePosition == "Developer";
    }
    if (projectStatus == "Testing") {
        return employeePosition == "QA";
    }
    if (projectStatus == "Deployment") {
        return employeePosition == "Manager";
    }
    if (projectStatus == "Maintenance") {
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
    if (!employee->isAvailable(hours)) {
        throw CompanyException(
            QString("Not enough available hours. Available: %1h")
                .arg(employee->getAvailableHours()));
    }

    std::shared_ptr<Project> projPtr = projects.find(projectId);
    if (!projPtr) throw CompanyException("Project not found");

    QString projectStatus = projPtr->getStatus();
    if (projectStatus == "Completed") {
        throw CompanyException("Cannot assign to project with status: " +
                               projectStatus);
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
                roleMatchesSDLCStage(employeePosition, projectStatus);

            if (!roleMatches) {
                throw CompanyException(QString("Employee role '%1' does not "
                                               "match project SDLC stage '%2'")
                                           .arg(employeePosition)
                                           .arg(projectStatus));
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

            int needed = task.getEstimatedHours() - task.getAllocatedHours();
            if (needed <= 0)
                throw CompanyException("Task already fully allocated");
            int toAssign = needed < hours ? needed : hours;

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
                double maxAffordableHourlyRate = averageBudgetPerHour * 0.7;

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

            projPtr->addEmployeeCost(assignmentCost);

            projPtr->recomputeTotalsFromTasks();
            found = true;
            break;
        }
    }
    if (!found) throw CompanyException("Task not found");
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
                                    const QString& projectStatus) {
    if (!employee) return false;
    QString pos = employee->getPosition();
    return roleMatchesSDLCStage(pos, projectStatus);
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
    double hourlyRateA = calculateHourlyRate(a->getSalary());
    double hourlyRateB = calculateHourlyRate(b->getSalary());
    int availA = a->getAvailableHours() - employeeUsage.at(a->getId());
    int availB = b->getAvailableHours() - employeeUsage.at(b->getId());

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

    QString projectStatus = projPtr->getStatus();
    if (projectStatus == "Completed") {
        throw CompanyException("Cannot auto-assign to project with status: " +
                               projectStatus);
    }

    std::vector<Task>& tasks = projPtr->getTasks();
    if (tasks.empty()) throw CompanyException("No tasks in project");

    std::vector<size_t> taskIndices;
    for (size_t i = 0; i < tasks.size(); ++i) {
        taskIndices.push_back(i);
    }

    for (size_t i = 0; i < taskIndices.size() - 1; ++i) {
        for (size_t j = i + 1; j < taskIndices.size(); ++j) {
            const Task& taskA = tasks[taskIndices[i]];
            const Task& taskB = tasks[taskIndices[j]];
            if (compareTaskPriority(taskA, taskB) > 0) {
                size_t temp = taskIndices[i];
                taskIndices[i] = taskIndices[j];
                taskIndices[j] = temp;
            }
        }
    }

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
            if (!employeeRoleMatchesSDLC(employee, projectStatus)) continue;

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

        for (size_t i = 0; i < pool.size() - 1; ++i) {
            for (size_t j = i + 1; j < pool.size(); ++j) {
                if (compareEmployeesForSorting(pool[i], pool[j],
                                               employeeUsage) > 0) {
                    std::shared_ptr<Employee> temp = pool[i];
                    pool[i] = pool[j];
                    pool[j] = temp;
                }
            }
        }

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
            currentEmployeeCosts += assignmentCost;
            remainingBudget -= assignmentCost;
            remaining -= toAssign;
        }
    }

    double totalNewCosts = 0.0;
    for (std::map<int, int>::const_iterator it = employeeUsage.begin();
         it != employeeUsage.end(); ++it) {
        int employeeId = it->first;
        int hours = it->second;
        std::shared_ptr<Employee> emp = getEmployee(employeeId);
        if (emp) {
            totalNewCosts += calculateEmployeeCost(emp->getSalary(), hours);
        }
    }
    projPtr->addEmployeeCost(totalNewCosts);

    projPtr->recomputeTotalsFromTasks();
}
