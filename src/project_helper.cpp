#include "project_helper.h"

#include <QBrush>
#include <QColor>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QWidget>

#include "company.h"
#include "main_window.h"
#include "project.h"
#include "task.h"

void ProjectHelper::populateProjectTasksTable(QTableWidget* table, const Project& project,
                                              MainWindow* mainWindow) {
    if (!table || !mainWindow) return;

    const std::vector<Task>& tasks = project.getTasks();
    table->setRowCount(0);
    table->clearContents();

    if (tasks.empty()) return;

    int rowCount = static_cast<int>(tasks.size());
    table->setRowCount(rowCount);

    for (int row = 0; row < rowCount; ++row) {
        const Task& task = tasks[static_cast<size_t>(row)];

        auto* nameItem = new QTableWidgetItem(task.getName());
        nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        nameItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        table->setItem(row, 0, nameItem);

        auto* typeItem = new QTableWidgetItem(task.getType());
        typeItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        typeItem->setTextAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        table->setItem(row, 1, typeItem);

        auto* priorityItem = new QTableWidgetItem(QString::number(task.getPriority()));
        priorityItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        priorityItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 2, priorityItem);

        auto* estimatedItem = new QTableWidgetItem(QString::number(task.getEstimatedHours()));
        estimatedItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        estimatedItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 3, estimatedItem);

        auto* allocatedItem = new QTableWidgetItem(QString::number(task.getAllocatedHours()));
        allocatedItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        allocatedItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 4, allocatedItem);

        if (task.getAllocatedHours() >= task.getEstimatedHours() && task.getEstimatedHours() > 0) {
            auto* fullyAllocatedItem = new QTableWidgetItem("Fully Allocated");
            fullyAllocatedItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            fullyAllocatedItem->setTextAlignment(Qt::AlignCenter);
            fullyAllocatedItem->setForeground(QBrush(QColor(100, 100, 100)));
            table->setItem(row, 5, fullyAllocatedItem);
        } else {
            QWidget* assignContainer = new QWidget(table);
            auto* assignLayout = new QHBoxLayout(assignContainer);
            assignLayout->setContentsMargins(0, 0, 0, 0);
            assignLayout->setAlignment(Qt::AlignCenter);

            auto* assignButton = new QPushButton("Assign", assignContainer);
            assignButton->setMinimumWidth(150);
            assignButton->setFixedHeight(39);
            assignButton->setProperty("projectId", project.getId());
            assignButton->setProperty("taskId", task.getId());
            QObject::connect(assignButton, &QPushButton::clicked, [mainWindow, projectId = project.getId(), taskId = task.getId()]() {
                if (mainWindow) {

                    QMetaObject::invokeMethod(mainWindow, "assignTaskFromDetails", Qt::QueuedConnection,
                                              Q_ARG(int, projectId),
                                              Q_ARG(int, taskId));
                }
            });

            assignLayout->addWidget(assignButton);
            table->setCellWidget(row, 5, assignContainer);
        }
        table->setRowHeight(row, 62);
    }
}

void ProjectHelper::clearProjectAllocatedHoursIfNoEmployees(Company* company, int projectId) {
    if (!company) return;

    const Project* project = company->getProject(projectId);
    if (!project || project->getAllocatedHours() == 0) return;

    if (!hasAssignedEmployees(company, projectId)) {
        Project* projPtr = const_cast<Project*>(project);
        std::vector<Task>& projectTasks = projPtr->getTasks();
        for (auto& t : projectTasks) {
            if (t.getAllocatedHours() > 0) {
                t.setAllocatedHours(0);
            }
        }
        projPtr->recomputeTotalsFromTasks();
    }
}

bool ProjectHelper::hasAssignedEmployees(const Company* company, int projectId) {
    if (!company) return false;

    auto employees = company->getAllEmployees();
    for (const auto& emp : employees) {
        if (emp && emp->getIsActive() && emp->isAssignedToProject(projectId)) {
            return true;
        }
    }
    return false;
}

