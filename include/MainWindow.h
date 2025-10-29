#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QMessageBox>
#include <QDateEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "Company.h"
#include "FileManager.h"
#include <memory>
#include <vector>

class MainWindow : public QMainWindow {
    Q_OBJECT

private:
    // Validation constants
    static constexpr int MIN_SALARY = 100;
    static constexpr int MAX_SALARY = 500000;
    static constexpr int MIN_YEAR = 1800;
    static constexpr int DESC_EDIT_MAX_HEIGHT = 100;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

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
    void autoSave(); // Automatic save without dialog

    // UI components
    QTabWidget* tabWidget;
    
    // Employee tab
    QWidget* employeeTab;
    QTableWidget* employeeTable;
    QPushButton* addEmployeeBtn;
    QPushButton* editEmployeeBtn;
    QPushButton* deleteEmployeeBtn;
    QPushButton* searchEmployeeBtn;
    QLineEdit* searchEmployeeEdit;
    
    // Project tab
    QWidget* projectTab;
    QTableWidget* projectTable;
    QPushButton* addProjectBtn;
    QPushButton* editProjectBtn;
    QPushButton* deleteProjectBtn;
    QPushButton* searchProjectBtn;
    QLineEdit* searchProjectEdit;
    
    // Company info tab
    QWidget* infoTab;
    QTextEdit* companyInfoText;
    
    // Statistics tab
    QWidget* statsTab;
    QTextEdit* statisticsText;
    QPushButton* refreshStatsBtn;
    
    // Menu bar
    QMenuBar* menuBar;
    QMenu* fileMenu;
    QAction* saveAction;
    QAction* loadAction;
    
    // Data
    std::vector<Company*> companies;
    Company* currentCompany;
    int currentCompanyIndex;
    int nextEmployeeId;
    int nextProjectId;
    
    // Company management UI
    QComboBox* companySelector;
    QPushButton* addCompanyBtn;
    QPushButton* deleteCompanyBtn;
};

#endif // MAINWINDOW_H

