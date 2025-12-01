#include "helpers/task_assignment_helper.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include "entities/company.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"

static bool roleMatchesSDLC(const QString& phase, const QString& role) {
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
}

static QString formatEmployeeInfo(const std::shared_ptr<Employee>& emp,
                                  const QString& projectPhaseForRole) {
    int available = emp->getAvailableHours();
    int capacity = emp->getWeeklyHoursCapacity();
    int current = emp->getCurrentWeeklyHours();
    bool matches = roleMatchesSDLC(projectPhaseForRole, emp->getPosition());

    QString info = QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                       .arg(emp->getName())
                       .arg(emp->getPosition())
                       .arg(capacity)
                       .arg(current)
                       .arg(available);

    if (!matches && !projectPhaseForRole.isEmpty()) {
        QString expectedRole =
            TaskAssignmentHelper::getExpectedRoleForProjectPhase(
                projectPhaseForRole);
        info += QString(" [%1 role required for %2 phase]")
                    .arg(expectedRole)
                    .arg(projectPhaseForRole);
    }

    return info;
}

static void updateTaskInfoLabel(QLabel* taskInfoLabel, int matchingCount,
                                const QString& projectPhaseForRole) {
    QString expectedRole = TaskAssignmentHelper::getExpectedRoleForProjectPhase(
        projectPhaseForRole);
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
                .arg(projectPhaseForRole.isEmpty() ? "Unknown"
                                                   : projectPhaseForRole)
                .arg(expectedRole));
        taskInfoLabel->setStyleSheet("color: orange;");
    }
}

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
    QString projectPhaseForRole = project ? project->getPhase() : projectPhase;

    for (const auto& emp : employees) {
        if (emp && emp->getIsActive()) {
            bool matches =
                roleMatchesSDLC(projectPhaseForRole, emp->getPosition());
            QString info = formatEmployeeInfo(emp, projectPhaseForRole);

            employeeCombo->addItem(info, emp->getId());
            if (matches && emp->getAvailableHours() > 0) matchingCount++;
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
        QString phase;
        if (task.getAllocatedHours() > 0 && remaining <= 0) {
            phase = "Fully allocated";
        } else if (remaining > 0) {
            phase = QString("Needs: %1h").arg(remaining);
        }
        QString taskText =
            QString("[%1] %2").arg(task.getType()).arg(task.getName());
        if (!phase.isEmpty()) {
            taskText += QString(" (%1)").arg(phase);
        }
        taskCombo->addItem(taskText, task.getId());
    }
}

static void updateHoursEditImpl(QLineEdit* hoursEdit,
                                const QComboBox* taskCombo,
                                const QComboBox* employeeCombo,
                                const std::vector<Task>& tasks,
                                const Company* company) {
    int taskIndex = taskCombo->currentIndex();
    int employeeIndex = employeeCombo->currentIndex();

    if (taskIndex < 0 || static_cast<size_t>(taskIndex) >= tasks.size()) return;
    if (employeeIndex < 0) return;

    const auto& selectedTask = tasks[static_cast<size_t>(taskIndex)];
    int employeeId = employeeCombo->itemData(employeeIndex).toInt();
    auto employee = company->getEmployee(employeeId);

    if (!employee) return;

    int taskRemaining =
        selectedTask.getEstimatedHours() - selectedTask.getAllocatedHours();
    int employeeAvailable = employee->getAvailableHours();
    int maxHours = std::min(taskRemaining, employeeAvailable);

    if (maxHours > 0) {
        hoursEdit->setText(QString::number(maxHours));
        hoursEdit->setPlaceholderText(
            QString("Max: %1h (task needs: %2h, employee has: %3h available)")
                .arg(maxHours)
                .arg(taskRemaining)
                .arg(employeeAvailable));
    } else {
        hoursEdit->setText("");
        if (taskRemaining <= 0 && selectedTask.getAllocatedHours() > 0) {
            hoursEdit->setPlaceholderText("Task is fully allocated");
        } else {
            hoursEdit->setPlaceholderText(
                QString("Employee has only %1h available (task needs: %2h)")
                    .arg(employeeAvailable)
                    .arg(taskRemaining));
        }
    }
}

void TaskAssignmentHelper::setupHoursEdit(QLineEdit* hoursEdit,
                                          const QComboBox* taskCombo,
                                          const QComboBox* employeeCombo,
                                          const std::vector<Task>& tasks,
                                          const Company* company) {
    if (!hoursEdit || !taskCombo || !employeeCombo || !company) return;

    auto updateHoursEdit = [hoursEdit, taskCombo, employeeCombo, &tasks,
                            company]() {
        updateHoursEditImpl(hoursEdit, taskCombo, employeeCombo, tasks,
                            company);
    };

    QObject::connect(taskCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [updateHoursEdit](int) { updateHoursEdit(); });

    QObject::connect(employeeCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [updateHoursEdit](int) { updateHoursEdit(); });

    if (!tasks.empty() && employeeCombo->count() > 0) {
        updateHoursEdit();
    }
}

void TaskAssignmentHelper::setupEmployeeComboUpdate(
    QComboBox* employeeCombo, const QComboBox* taskCombo, QLabel* taskInfoLabel,
    const Company* company, int projectId, const QString& projectPhase) {
    if (!employeeCombo || !taskCombo || !taskInfoLabel || !company) return;

    QObject::connect(
        taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [employeeCombo, taskInfoLabel, company, projectId, projectPhase](int) {
            int matchingCount = 0;
            employeeCombo->clear();

            auto employees = company->getAllEmployees();
            auto project = company->getProject(projectId);
            QString projectPhaseForRole =
                project ? project->getPhase() : projectPhase;

            for (const auto& emp : employees) {
                if (!emp || !emp->getIsActive()) continue;

                QString info = formatEmployeeInfo(emp, projectPhaseForRole);
                if (bool matches = roleMatchesSDLC(projectPhaseForRole,
                                                   emp->getPosition());
                    matches && emp->getAvailableHours() > 0) {
                    matchingCount++;
                }

                employeeCombo->addItem(info, emp->getId());
            }

            updateTaskInfoLabel(taskInfoLabel, matchingCount,
                                projectPhaseForRole);
        });
}
