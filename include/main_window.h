#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
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
#include "company_manager.h"
#include "consts.h"
#include "display_helper.h"
#include "employee_dialog_helper.h"
#include "file_manager.h"

class EmployeeDialogHelper;

class MainWindow : public QMainWindow {
    Q_OBJECT

   private:
   public:
    explicit MainWindow(QWidget* parent = nullptr);
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
    int getSelectedEmployeeId() const;
    int getSelectedProjectId() const;
    void autoSave() const;  // Automatic save without dialog
    static bool checkDuplicateProjectOnEdit(const QString& projectName,
                                             int excludeId,
                                             Company* currentCompany);

    using EmployeeFormWidgets = EmployeeDialogHelper::EmployeeFormWidgets;
    // UI component groups
    struct EmployeeTabUI {
        QWidget* tab = nullptr;
        QTableWidget* table = nullptr;
        QPushButton* addBtn = nullptr;
        QPushButton* editBtn = nullptr;
        QPushButton* deleteBtn = nullptr;
        QPushButton* searchBtn = nullptr;
        QLineEdit* searchEdit = nullptr;
    };

    struct ProjectTabUI {
        QWidget* tab = nullptr;
        QTableWidget* table = nullptr;
        QPushButton* addBtn = nullptr;
        QPushButton* editBtn = nullptr;
        QPushButton* deleteBtn = nullptr;
        QPushButton* searchBtn = nullptr;
        QLineEdit* searchEdit = nullptr;
    };

    struct TabWidgets {
        QTabWidget* mainTabWidget = nullptr;
        QWidget* infoTab = nullptr;
        QTextEdit* companyInfoText = nullptr;
        QWidget* statsTab = nullptr;
        QTextEdit* statisticsText = nullptr;
        QPushButton* refreshStatsBtn = nullptr;
    };

    struct MenuBarUI {
        QMenuBar* menuBar = nullptr;
        QMenu* fileMenu = nullptr;
        QAction* saveAction = nullptr;
        QAction* loadAction = nullptr;
    };

    struct CompanyManagementUI {
        QComboBox* selector = nullptr;
        QPushButton* addBtn = nullptr;
        QPushButton* deleteBtn = nullptr;
    };

    using CompanyData = CompanyManager::CompanyData;

    EmployeeTabUI employeeTabUI;
    ProjectTabUI projectTabUI;
    TabWidgets tabWidgets;
    MenuBarUI menuBarUI;
    CompanyManagementUI companyManagementUI;
    CompanyData companyData;
};

#endif  // MAINWINDOW_H
