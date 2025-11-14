#include "helpers/action_button_helper.h"

#include <QAction>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

#include "ui/main_window.h"

QWidget* ActionButtonHelper::createEmployeeActionButtons(
    QTableWidget* table, int rowIndex, MainWindow* mainWindow) {
    if (!table || !mainWindow) return nullptr;

    QWidget* actionContainer = new QWidget(table);
    QHBoxLayout* actionLayout = new QHBoxLayout(actionContainer);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setAlignment(Qt::AlignCenter);

    QPushButton* actionButton = new QPushButton("Actions", actionContainer);
    actionButton->setFixedHeight(38);
    actionButton->setMinimumWidth(150);
    QMenu* actionMenu = new QMenu(actionButton);

    QAction* editAction = actionMenu->addAction("✎ Edit");
    QAction* fireAction = actionMenu->addAction("❌ Fire");
    QAction* deleteAction = actionMenu->addAction("🗑️ Delete");
    QAction* historyAction = actionMenu->addAction("📋 History");

    actionButton->setMenu(actionMenu);
    actionLayout->addWidget(actionButton);

    QObject::connect(editAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "editEmployee",
                                                       Qt::QueuedConnection);
                         }
                     });

    QObject::connect(fireAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "fireEmployee",
                                                       Qt::QueuedConnection);
                         }
                     });

    QObject::connect(deleteAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "deleteEmployee",
                                                       Qt::QueuedConnection);
                         }
                     });

    QObject::connect(historyAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "viewEmployeeHistory",
                                                       Qt::QueuedConnection);
                         }
                     });

    return actionContainer;
}

QWidget* ActionButtonHelper::createProjectActionButtons(QTableWidget* table,
                                                        int rowIndex,
                                                        MainWindow* mainWindow,
                                                        bool includeAddTask) {
    if (!table || !mainWindow) return nullptr;

    QWidget* actionContainer = new QWidget(table);
    QHBoxLayout* actionLayout = new QHBoxLayout(actionContainer);
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->setAlignment(Qt::AlignCenter);

    QPushButton* actionButton = new QPushButton("Actions", actionContainer);
    actionButton->setFixedHeight(38);
    actionButton->setMinimumWidth(150);
    QMenu* actionMenu = new QMenu(actionButton);

    QAction* editAction = actionMenu->addAction("✎ Edit Project");
    QAction* deleteAction = actionMenu->addAction("🗑️ Delete Project");

    if (includeAddTask) {
        actionMenu->addSeparator();
        QAction* addTaskAction = actionMenu->addAction("➕ Add Task");
        QObject::connect(addTaskAction, &QAction::triggered, table,
                         [table, mainWindow, row = rowIndex]() {
                             table->setCurrentCell(row, 0);
                             if (mainWindow) {
                                 QMetaObject::invokeMethod(
                                     mainWindow, "addProjectTask",
                                     Qt::QueuedConnection);
                             }
                         });
    }

    actionMenu->addSeparator();
    QAction* moreAction = actionMenu->addAction("More");

    actionButton->setMenu(actionMenu);
    actionLayout->addWidget(actionButton);

    QObject::connect(editAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "editProject",
                                                       Qt::QueuedConnection);
                         }
                     });

    QObject::connect(deleteAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "deleteProject",
                                                       Qt::QueuedConnection);
                         }
                     });

    QObject::connect(moreAction, &QAction::triggered, table,
                     [table, mainWindow, row = rowIndex]() {
                         table->setCurrentCell(row, 0);
                         if (mainWindow) {
                             QMetaObject::invokeMethod(mainWindow,
                                                       "openProjectDetails",
                                                       Qt::QueuedConnection);
                         }
                     });

    return actionContainer;
}
