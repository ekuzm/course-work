#include "task_assignment_helper.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include "company.h"
#include "employee.h"
#include "project.h"
#include "task.h"

QString TaskAssignmentHelper::getExpectedRoleForProjectStatus(const QString& projectStatus) {
    if (projectStatus == "Analysis" || projectStatus == "Planning") {
        return "Manager";
    } else if (projectStatus == "Design") {
        return "Designer";
    } else if (projectStatus == "Development") {
        return "Developer";
    } else if (projectStatus == "Testing") {
        return "QA";
    } else if (projectStatus == "Deployment") {
        return "Manager";
    } else if (projectStatus == "Maintenance") {
        return "any role";
    }
    return "";
}

void TaskAssignmentHelper::populateEmployeeCombo(QComboBox* employeeCombo, const Company* company,
                                                 int projectId, const QString& projectStatus,
                                                 int& matchingCount) {
    if (!employeeCombo || !company) return;

    employeeCombo->clear();
    matchingCount = 0;

    auto employees = company->getAllEmployees();
    auto project = company->getProject(projectId);
    QString projectStatusForRole = project ? project->getStatus() : projectStatus;

    auto roleMatchesSDLC = [](const QString& phase, const QString& role) -> bool {
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
            bool matches = roleMatchesSDLC(projectStatusForRole, emp->getPosition());

            QString info = QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                              .arg(emp->getName())
                              .arg(emp->getPosition())
                              .arg(capacity)
                              .arg(current)
                              .arg(available);

            if (!matches && !projectStatusForRole.isEmpty()) {
                QString expectedRole = getExpectedRoleForProjectStatus(projectStatusForRole);
                info += QString(" [%1 role required for %2 phase]")
                            .arg(expectedRole)
                            .arg(projectStatusForRole);
            }

            employeeCombo->addItem(info, emp->getId());
            if (matches && available > 0) matchingCount++;
        }
    }
}

void TaskAssignmentHelper::setupTaskCombo(QComboBox* taskCombo, const std::vector<Task>& tasks,
                                          int pendingTaskId) {
    if (!taskCombo) return;

    taskCombo->clear();
    for (const auto& task : tasks) {
        if (pendingTaskId > 0 && task.getId() != pendingTaskId) continue;

        int remaining = task.getEstimatedHours() - task.getAllocatedHours();
        QString status = remaining > 0 ? QString("Needs: %1h").arg(remaining) : "Fully allocated";
        taskCombo->addItem(QString("[%1] %2 (%3)").arg(task.getType()).arg(task.getName()).arg(status),
                          task.getId());
    }
}

void TaskAssignmentHelper::setupHoursEdit(QLineEdit* hoursEdit, QComboBox* taskCombo,
                                         const std::vector<Task>& tasks) {
    if (!hoursEdit || !taskCombo) return;

    QObject::connect(taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    [hoursEdit, &tasks](int index) {
                        if (index < 0 || static_cast<size_t>(index) >= tasks.size()) return;
                        const auto& selectedTask = tasks[index];
                        int remaining = selectedTask.getEstimatedHours() - selectedTask.getAllocatedHours();
                        hoursEdit->setText(remaining > 0 ? QString::number(remaining) : "");
                    });

    if (!tasks.empty()) {
        int initialIndex = taskCombo->currentIndex();
        if (initialIndex < 0 || static_cast<size_t>(initialIndex) >= tasks.size()) {
            initialIndex = 0;
        }
        const auto& selectedTask = tasks[static_cast<size_t>(initialIndex)];
        int remaining = selectedTask.getEstimatedHours() - selectedTask.getAllocatedHours();
        if (remaining > 0) {
            hoursEdit->setText(QString::number(remaining));
        }
    }
}

void TaskAssignmentHelper::setupEmployeeComboUpdate(QComboBox* employeeCombo, QComboBox* taskCombo,
                                                    QLabel* taskInfoLabel, const Company* company,
                                                    int projectId, const QString& projectStatus) {
    if (!employeeCombo || !taskCombo || !taskInfoLabel || !company) return;

    auto roleMatchesSDLC = [](const QString& phase, const QString& role) -> bool {
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

    QObject::connect(taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    [employeeCombo, taskInfoLabel, company, projectId, projectStatus, roleMatchesSDLC](int) {
                        int matchingCount = 0;
                        employeeCombo->clear();

                        auto employees = company->getAllEmployees();
                        auto project = company->getProject(projectId);
                        QString projectStatusForRole = project ? project->getStatus() : projectStatus;

                        for (const auto& emp : employees) {
                            if (emp && emp->getIsActive()) {
                                int available = emp->getAvailableHours();
                                int capacity = emp->getWeeklyHoursCapacity();
                                int current = emp->getCurrentWeeklyHours();
                                bool matches = roleMatchesSDLC(projectStatusForRole, emp->getPosition());

                                QString info = QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                                                  .arg(emp->getName())
                                                  .arg(emp->getPosition())
                                                  .arg(capacity)
                                                  .arg(current)
                                                  .arg(available);

                                if (!matches && !projectStatusForRole.isEmpty()) {
                                    QString expectedRole = getExpectedRoleForProjectStatus(projectStatusForRole);
                                    info += QString(" [%1 role required for %2 phase]")
                                                .arg(expectedRole)
                                                .arg(projectStatusForRole);
                                } else if (available > 0) {
                                    matchingCount++;
                                }

                                employeeCombo->addItem(info, emp->getId());
                            }
                        }

                        QString expectedRole = getExpectedRoleForProjectStatus(projectStatusForRole);
                        if (matchingCount > 0) {
                            taskInfoLabel->setText(QString("Phase: %1 | Found %2 %3 employees with available hours")
                                                      .arg(projectStatusForRole.isEmpty() ? "Unknown" : projectStatusForRole)
                                                      .arg(matchingCount)
                                                      .arg(expectedRole));
                            taskInfoLabel->setStyleSheet("color: green;");
                        } else {
                            taskInfoLabel->setText(QString("Phase: %1 | No %2 employees with available hours")
                                                      .arg(projectStatusForRole.isEmpty() ? "Unknown" : projectStatusForRole)
                                                      .arg(expectedRole));
                            taskInfoLabel->setStyleSheet("color: orange;");
                        }
                    });
}

