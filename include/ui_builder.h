#pragma once

#include <QWidget>

class QTableWidget;
class QTabWidget;
class QComboBox;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QLabel;
class MainWindow;

class MainWindowUIBuilder {
   public:
    static void setupMainUI(MainWindow* window);
    static void setupEmployeeTab(MainWindow* window, QTabWidget* tabWidget);
    static void setupProjectTab(MainWindow* window, QTabWidget* tabWidget);
    static void setupStatisticsTab(MainWindow* window, QTabWidget* tabWidget);
    static QWidget* createCompanyWidget(MainWindow* window);
};

