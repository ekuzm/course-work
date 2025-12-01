#include "helpers/action_button_helper.h"

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QWidget>
#include <functional>

#include "ui/main_window.h"
#include "ui/main_window_operations.h"

QWidget* createActionContainer(QTableWidget* table) {
    auto* actionContainer = new QWidget(table);
    auto* actionLayout = new QHBoxLayout(actionContainer);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setAlignment(Qt::AlignCenter);
    return actionContainer;
}

QPushButton* createActionButton(QWidget* parent) {
    auto* actionButton = new QPushButton("Actions", parent);
    actionButton->setFixedHeight(38);
    actionButton->setMinimumWidth(150);
    return actionButton;
}

void connectAction(const QAction* action, QTableWidget* table,
                   const MainWindow* mainWindow, int rowIndex,
                   const std::function<void()>& operation) {
    QObject::connect(action, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex, operation]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             operation();
                         }
                     });
}

QWidget* ActionButtonHelper::createEmployeeActionButtons(
    QTableWidget* table, int rowIndex, MainWindow* mainWindow) {
    if (!table || !mainWindow) return nullptr;

    auto* actionContainer = createActionContainer(table);
    auto* actionButton = createActionButton(actionContainer);
    auto* actionMenu = new QMenu(actionButton);

    const auto* editAction = actionMenu->addAction("🖉 Edit");
    const auto* fireAction = actionMenu->addAction("❌ Fire");
    const auto* deleteAction = actionMenu->addAction("🗑️ Delete");
    const auto* historyAction = actionMenu->addAction("📋 History");

    actionButton->setMenu(actionMenu);
    qobject_cast<QHBoxLayout*>(actionContainer->layout())
        ->addWidget(actionButton);

    connectAction(editAction, table, mainWindow, rowIndex, [mainWindow]() {
        EmployeeOperations::editEmployee(mainWindow);
    });
    connectAction(fireAction, table, mainWindow, rowIndex, [mainWindow]() {
        EmployeeOperations::fireEmployee(mainWindow);
    });
    connectAction(deleteAction, table, mainWindow, rowIndex, [mainWindow]() {
        EmployeeOperations::deleteEmployee(mainWindow);
    });
    connectAction(historyAction, table, mainWindow, rowIndex, [mainWindow]() {
        ProjectOperations::viewEmployeeHistory(mainWindow);
    });

    return actionContainer;
}

QWidget* ActionButtonHelper::createProjectActionButtons(QTableWidget* table,
                                                        int rowIndex,
                                                        MainWindow* mainWindow,
                                                        bool includeAddTask) {
    if (!table || !mainWindow) return nullptr;

    auto* actionContainer = createActionContainer(table);
    auto* actionButton = createActionButton(actionContainer);
    auto* actionMenu = new QMenu(actionButton);

    const auto* editAction = actionMenu->addAction("🖉 Edit");
    const auto* deleteAction = actionMenu->addAction("🗑️ Delete");

    if (includeAddTask) {
        actionMenu->addSeparator();
        const auto* addTaskAction = actionMenu->addAction("➕ Add Task");
        connectAction(
            addTaskAction, table, mainWindow, rowIndex,
            [mainWindow]() { ProjectOperations::addProjectTask(mainWindow); });
    }

    actionMenu->addSeparator();
    const auto* moreAction = actionMenu->addAction("📋 More");

    QObject::connect(
        actionMenu, &QMenu::aboutToShow, actionMenu,
        [actionButton, actionMenu]() {
            const int buttonWidth = actionButton->width();
            actionMenu->setMinimumWidth(buttonWidth);
            const QPoint pos =
                actionButton->mapToGlobal(QPoint(0, actionButton->height()));
            QTimer::singleShot(0, actionMenu,
                               [actionMenu, pos]() { actionMenu->move(pos); });
            QTimer::singleShot(10, actionMenu, [actionMenu, pos]() {
                if (actionMenu->isVisible()) {
                    actionMenu->move(pos);
                }
            });
            QTimer::singleShot(30, actionMenu, [actionMenu, pos]() {
                if (actionMenu->isVisible()) {
                    actionMenu->move(pos);
                }
            });
        });

    actionButton->setMenu(actionMenu);

    qobject_cast<QHBoxLayout*>(actionContainer->layout())
        ->addWidget(actionButton);

    connectAction(editAction, table, mainWindow, rowIndex, [mainWindow]() {
        ProjectOperations::editProject(mainWindow);
    });
    connectAction(deleteAction, table, mainWindow, rowIndex, [mainWindow]() {
        ProjectOperations::deleteProject(mainWindow);
    });
    connectAction(moreAction, table, mainWindow, rowIndex, [mainWindow]() {
        ProjectOperations::openProjectDetails(mainWindow);
    });

    return actionContainer;
}
