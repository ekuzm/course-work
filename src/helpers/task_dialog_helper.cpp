#include "helpers/task_dialog_helper.h"

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "entities/company.h"
#include "entities/task.h"
#include "exceptions/exceptions.h"
#include "helpers/validation_helper.h"
#include "utils/consts.h"

void TaskDialogHelper::createAddTaskDialog(QDialog* dialog, QFormLayout* form,
                                           QLineEdit*& taskNameEdit,
                                           QComboBox*& taskTypeCombo,
                                           QLineEdit*& taskEstHoursEdit,
                                           QLineEdit*& priorityEdit) {
    if (!dialog || !form) return;

    dialog->setWindowTitle("Add Task to Project");
    dialog->setMinimumWidth(400);
    dialog->setStyleSheet(
        "QDialog { background-color: white; } "
        "QComboBox { background-color: white; color: black; } "
        "QLineEdit { background-color: white; color: black; } "
        "QLabel { color: black; }");

    taskNameEdit = new QLineEdit();
    taskNameEdit->setPlaceholderText("e.g., API Development");
    form->addRow("Task Name:", taskNameEdit);

    taskTypeCombo = new QComboBox();
    taskTypeCombo->addItems({"Development", "QA", "Design", "Management"});
    taskTypeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Task Type:", taskTypeCombo);

    taskEstHoursEdit = new QLineEdit();
    taskEstHoursEdit->setPlaceholderText("e.g., 80");
    form->addRow("Estimated Hours:", taskEstHoursEdit);

    priorityEdit = new QLineEdit();
    priorityEdit->setPlaceholderText("e.g., 5 (higher = more important)");
    form->addRow("Priority:", priorityEdit);
}

bool TaskDialogHelper::validateAndAddTask(const QString& taskName,
                                          const QString& taskType, int taskEst,
                                          int priority, int projectId,
                                          Company* company, QDialog* dialog) {
    if (!company || !dialog) return false;

    auto tasks = company->getProjectTasks(projectId);
    for (const auto& existingTask : tasks) {
        if (existingTask.getName().toLower() == taskName.toLower()) {
            QWidget* parent = dialog ? dialog->parentWidget() : nullptr;
            if (!parent && dialog) {
                parent = dialog;
            }
            QMessageBox::warning(
                parent, "Duplicate Error",
                QString("A task with this name already exists!\n\n"
                        "Task name: \"%1\"\n"
                        "Project ID: %2\n"
                        "Please choose a different name for the task.")
                    .arg(taskName, QString::number(projectId)));
            return false;
        }
    }

    const auto* proj = company->getProject(projectId);
    if (!proj) {
        QWidget* parent = dialog ? dialog->parentWidget() : nullptr;
        if (!parent && dialog) {
            parent = dialog;
        }
        QMessageBox::warning(parent, "Error", "Project not found!");
        return false;
    }

    int nextId = proj->getNextTaskId();
    Task task(nextId, taskName, taskType, taskEst, priority);
    company->addTaskToProject(projectId, task);
    return true;
}
