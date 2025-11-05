#pragma once

#include <QAction>
#include <QCloseEvent>
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

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

   protected:
    void closeEvent(QCloseEvent* event) override;

   private slots:
    void initializeCompanySetup();

    void addEmployee();
    void editEmployee();
    void deleteEmployee();
    void searchEmployee() const;
    void refreshEmployeeTable() const;

    void addProject();
    void editProject();
    void deleteProject();
    void searchProject() const;
    void refreshProjectTable() const;

    void addProjectTask();
    void assignEmployeeToTask();
    void autoAssignToProject();
    void viewProjectAssignments();
    void viewEmployeeHistory();

    static QString getDataDirectory();
    void saveData();

    void showCompanyInfo() const;
    void showStatistics() const;

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

    int getSelectedEmployeeId() const;
    int getSelectedProjectId() const;
    void autoSave() const;
    void autoLoad();
    static bool checkDuplicateProjectOnEdit(const QString& projectName,
                                            int excludeId,
                                            const Company* currentCompany);

    QWidget* employeeTab = nullptr;
    QTableWidget* employeeTable = nullptr;
    QPushButton* employeeAddBtn = nullptr;
    QPushButton* employeeEditBtn = nullptr;
    QPushButton* employeeDeleteBtn = nullptr;
    QPushButton* employeeHistoryBtn = nullptr;
    QPushButton* employeeRefreshBtn = nullptr;
    QPushButton* employeeSearchBtn = nullptr;
    QLineEdit* employeeSearchEdit = nullptr;

    QWidget* projectTab = nullptr;
    QTableWidget* projectTable = nullptr;
    QPushButton* projectAddBtn = nullptr;
    QPushButton* projectEditBtn = nullptr;
    QPushButton* projectDeleteBtn = nullptr;
    QPushButton* projectAddTaskBtn = nullptr;
    QPushButton* projectAssignBtn = nullptr;
    QPushButton* projectAutoAssignBtn = nullptr;
    QPushButton* projectViewAssignmentsBtn = nullptr;
    QPushButton* projectSearchBtn = nullptr;
    QLineEdit* projectSearchEdit = nullptr;

    QTabWidget* mainTabWidget = nullptr;
    QWidget* infoTab = nullptr;
    QTextEdit* companyInfoText = nullptr;
    QWidget* statsTab = nullptr;
    QTextEdit* statisticsText = nullptr;
    QPushButton* refreshStatsBtn = nullptr;

    QMenuBar* menuBar = nullptr;
    QMenu* fileMenu = nullptr;
    QAction* saveAction = nullptr;

    QComboBox* companySelector = nullptr;
    QPushButton* companyAddBtn = nullptr;
    QPushButton* companyDeleteBtn = nullptr;

    std::vector<Company*> companies{};
    Company* currentCompany = nullptr;
    int currentCompanyIndex = -1;
    int nextEmployeeId = 1;
    int nextProjectId = 1;
};
