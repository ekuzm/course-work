#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>
#include <vector>

#include "company.h"
#include "consts.h"
#include "file_manager.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

   private:

   public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

   private slots:
    // Company setup
    void initializeCompanySetup();

    // Employee slots
    void addEmployee();
    void editEmployee();
    void deleteEmployee();
    void searchEmployee();
    void refreshEmployeeTable();

    // Project slots
    void addProject();
    void editProject();
    void deleteProject();
    void searchProject();
    void refreshProjectTable();

    // File operations
    void saveData();
    void loadData();

    // View tabs
    void showCompanyInfo();
    void showStatistics();

    // Company management
    void addCompany();
    void switchCompany();
    void deleteCompany();
    void refreshCompanyList();

   private:
    void setupUI();
    void setupEmployeeTab();
    void setupProjectTab();
    void setupCompanyInfoTab();
    void setupStatisticsTab();

    // Helper methods
    void displayEmployees();
    void displayProjects();
    int getSelectedEmployeeId();
    int getSelectedProjectId();
    void autoSave();  // Automatic save without dialog

    // UI components
    QTabWidget* tabWidget = nullptr;

    // Employee tab
    QWidget* employeeTab = nullptr;
    QTableWidget* employeeTable = nullptr;
    QPushButton* addEmployeeBtn = nullptr;
    QPushButton* editEmployeeBtn = nullptr;
    QPushButton* deleteEmployeeBtn = nullptr;
    QPushButton* searchEmployeeBtn = nullptr;
    QLineEdit* searchEmployeeEdit = nullptr;

    // Project tab
    QWidget* projectTab = nullptr;
    QTableWidget* projectTable = nullptr;
    QPushButton* addProjectBtn = nullptr;
    QPushButton* editProjectBtn = nullptr;
    QPushButton* deleteProjectBtn = nullptr;
    QPushButton* searchProjectBtn = nullptr;
    QLineEdit* searchProjectEdit = nullptr;

    // Company info tab
    QWidget* infoTab = nullptr;
    QTextEdit* companyInfoText = nullptr;

    // Statistics tab
    QWidget* statsTab = nullptr;
    QTextEdit* statisticsText = nullptr;
    QPushButton* refreshStatsBtn = nullptr;

    // Menu bar
    QMenuBar* menuBar = nullptr;
    QMenu* fileMenu = nullptr;
    QAction* saveAction = nullptr;
    QAction* loadAction = nullptr;

    // Data
    std::vector<Company*> companies;
    Company* currentCompany = nullptr;
    int currentCompanyIndex = -1;
    int nextEmployeeId = 1;
    int nextProjectId = 1;

    // Company management UI
    QComboBox* companySelector = nullptr;
    QPushButton* addCompanyBtn = nullptr;
    QPushButton* deleteCompanyBtn = nullptr;
};

#endif  // MAINWINDOW_H
