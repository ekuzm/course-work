#include "task_assignment_helper.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include "company.h"
#include "employee.h"
#include "project.h"
#include "task.h"

QString TaskAssignmentHelper::getExpectedRoleForProjectPhase(
    const QString& projectPhase) {
    if (projectPhase == "Analysis" || projectPhase == "Planning") {
        return "Manager";
    } else if (projectPhase == "Design") {
        return "Designer";
    } else if (projectPhase == "Development") {
        return "Developer";
    } else if (projectPhase == "Testing") {
        return "QA";
    } else if (projectPhase == "Deployment") {
        return "Manager";
    } else if (projectPhase == "Maintenance") {
        return "any role";
    }
    return "";
}

void TaskAssignmentHelper::populateEmployeeCombo(QComboBox* employeeCombo,
                                                 const Company* company,
                                                 int projectId,
                                                 const QString& projectPhase,
                                                 int& matchingCount) {
    if (!employeeCombo || !company) return;

    employeeCombo->clear();
    matchingCount = 0;

    auto employees = company->getAllEmployees();
    auto project = company->getProject(projectId);
    QString projectPhaseForRole =
        project ? project->getPhase() : projectPhase;

    auto roleMatchesSDLC = [](const QString& phase,
                              const QString& role) -> bool {
        if (phase == "Analysis" || phase == "Planning") {
            return role.contains("Manager", Qt::CaseInsensitive);
        } else if (phase == "Design") {
            return role.contains("Designer", Qt::CaseInsensitive);
        } else if (phase == "Development") {
            return role.contains("Developer", Qt::CaseInsensitive);
        } else if (phase == "Testing") {
            return role.contains("QA", Qt::CaseInsensitive) ||
                   role.contains("Tester", Qt::CaseInsensitive);
        } else if (phase == "Deployment") {
            return role.contains("Manager", Qt::CaseInsensitive);
        } else if (phase == "Maintenance") {
            return true;
        }
        return true;
    };

    for (const auto& emp : employees) {
        if (emp && emp->getIsActive()) {
            int available = emp->getAvailableHours();
            int capacity = emp->getWeeklyHoursCapacity();
            int current = emp->getCurrentWeeklyHours();
            bool matches =
                roleMatchesSDLC(projectPhaseForRole, emp->getPosition());

            QString info = QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                               .arg(emp->getName())
                               .arg(emp->getPosition())
                               .arg(capacity)
                               .arg(current)
                               .arg(available);

            if (!matches && !projectPhaseForRole.isEmpty()) {
                QString expectedRole =
                    getExpectedRoleForProjectPhase(projectPhaseForRole);
                info += QString(" [%1 role required for %2 phase]")
                            .arg(expectedRole)
                            .arg(projectPhaseForRole);
            }

            employeeCombo->addItem(info, emp->getId());
            if (matches && available > 0) matchingCount++;
        }
    }
}

void TaskAssignmentHelper::setupTaskCombo(QComboBox* taskCombo,
                                          const std::vector<Task>& tasks,
                                          int pendingTaskId) {
    if (!taskCombo) return;

    taskCombo->clear();
    for (const auto& task : tasks) {
        if (pendingTaskId > 0 && task.getId() != pendingTaskId) continue;

        int remaining = task.getEstimatedHours() - task.getAllocatedHours();
        QString phase = remaining > 0 ? QString("Needs: %1h").arg(remaining)
                                       : "Fully allocated";
        taskCombo->addItem(QString("[%1] %2 (%3)")
                               .arg(task.getType())
                               .arg(task.getName())
                               .arg(phase),
                           task.getId());
    }
}

void TaskAssignmentHelper::setupHoursEdit(QLineEdit* hoursEdit,
                                          QComboBox* taskCombo,
                                          QComboBox* employeeCombo,
                                          const std::vector<Task>& tasks,
                                          const Company* company) {
    if (!hoursEdit || !taskCombo || !employeeCombo || !company) return;

    auto updateHoursEdit = [hoursEdit, taskCombo, employeeCombo, &tasks, company]() {
        int taskIndex = taskCombo->currentIndex();
        int employeeIndex = employeeCombo->currentIndex();
        
        if (taskIndex < 0 || static_cast<size_t>(taskIndex) >= tasks.size()) return;
        if (employeeIndex < 0) return;
        
        const auto& selectedTask = tasks[static_cast<size_t>(taskIndex)];
        int employeeId = employeeCombo->itemData(employeeIndex).toInt();
        auto employee = company->getEmployee(employeeId);
        
        if (!employee) return;
        
        int taskRemaining = selectedTask.getEstimatedHours() -
                            selectedTask.getAllocatedHours();
        int employeeAvailable = employee->getAvailableHours();
        int maxHours = std::min(taskRemaining, employeeAvailable);
        
        if (maxHours > 0) {
            hoursEdit->setText(QString::number(maxHours));
            hoursEdit->setPlaceholderText(QString("Max: %1h (task needs: %2h, employee has: %3h available)")
                                              .arg(maxHours)
                                              .arg(taskRemaining)
                                              .arg(employeeAvailable));
        } else {
            hoursEdit->setText("");
            if (taskRemaining <= 0) {
                hoursEdit->setPlaceholderText("Task is fully allocated");
            } else {
                hoursEdit->setPlaceholderText(QString("Employee has only %1h available (task needs: %2h)")
                                                  .arg(employeeAvailable)
                                                  .arg(taskRemaining));
            }
        }
    };

    QObject::connect(
        taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [updateHoursEdit](int) { updateHoursEdit(); });

    QObject::connect(
        employeeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [updateHoursEdit](int) { updateHoursEdit(); });

    if (!tasks.empty() && employeeCombo->count() > 0) {
        updateHoursEdit();
    }
}

void TaskAssignmentHelper::setupEmployeeComboUpdate(
    QComboBox* employeeCombo, QComboBox* taskCombo, QLabel* taskInfoLabel,
    const Company* company, int projectId, const QString& projectPhase) {
    if (!employeeCombo || !taskCombo || !taskInfoLabel || !company) return;

    auto roleMatchesSDLC = [](const QString& phase,
                              const QString& role) -> bool {
        if (phase == "Analysis" || phase == "Planning") {
            return role.contains("Manager", Qt::CaseInsensitive);
        } else if (phase == "Design") {
            return role.contains("Designer", Qt::CaseInsensitive);
        } else if (phase == "Development") {
            return role.contains("Developer", Qt::CaseInsensitive);
        } else if (phase == "Testing") {
            return role.contains("QA", Qt::CaseInsensitive) ||
                   role.contains("Tester", Qt::CaseInsensitive);
        } else if (phase == "Deployment") {
            return role.contains("Manager", Qt::CaseInsensitive);
        } else if (phase == "Maintenance") {
            return true;
        }
        return true;
    };

    QObject::connect(
        taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [employeeCombo, taskInfoLabel, company, projectId, projectPhase,
         roleMatchesSDLC](int) {
            int matchingCount = 0;
            employeeCombo->clear();

            auto employees = company->getAllEmployees();
            auto project = company->getProject(projectId);
            QString projectPhaseForRole =
                project ? project->getPhase() : projectPhase;

            for (const auto& emp : employees) {
                if (emp && emp->getIsActive()) {
                    int available = emp->getAvailableHours();
                    int capacity = emp->getWeeklyHoursCapacity();
                    int current = emp->getCurrentWeeklyHours();
                    bool matches = roleMatchesSDLC(projectPhaseForRole,
                                                   emp->getPosition());

                    QString info =
                        QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                            .arg(emp->getName())
                            .arg(emp->getPosition())
                            .arg(capacity)
                            .arg(current)
                            .arg(available);

                    if (!matches && !projectPhaseForRole.isEmpty()) {
                        QString expectedRole = getExpectedRoleForProjectPhase(
                            projectPhaseForRole);
                        info += QString(" [%1 role required for %2 phase]")
                                    .arg(expectedRole)
                                    .arg(projectPhaseForRole);
                    } else if (available > 0) {
                        matchingCount++;
                    }

                    employeeCombo->addItem(info, emp->getId());
                }
            }

            QString expectedRole =
                getExpectedRoleForProjectPhase(projectPhaseForRole);
            if (matchingCount > 0) {
                taskInfoLabel->setText(QString("Phase: %1 | Found %2 %3 "
                                               "employees with available hours")
                                           .arg(projectPhaseForRole.isEmpty()
                                                    ? "Unknown"
                                                    : projectPhaseForRole)
                                           .arg(matchingCount)
                                           .arg(expectedRole));
                taskInfoLabel->setStyleSheet("color: green;");
            } else {
                taskInfoLabel->setText(
                    QString("Phase: %1 | No %2 employees with available hours")
                        .arg(projectPhaseForRole.isEmpty()
                                 ? "Unknown"
                                 : projectPhaseForRole)
                        .arg(expectedRole));
                taskInfoLabel->setStyleSheet("color: orange;");
            }
        });
}
