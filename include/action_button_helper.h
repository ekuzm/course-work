#pragma once

#include <QWidget>

class QTableWidget;
class MainWindow;

class ActionButtonHelper {
   public:
    static QWidget* createEmployeeActionButtons(QTableWidget* table, int rowIndex, MainWindow* mainWindow);
    static QWidget* createProjectActionButtons(QTableWidget* table, int rowIndex, MainWindow* mainWindow, bool includeAddTask = true);
};

