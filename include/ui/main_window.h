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
#include <QMessageBox>
#include <QPushButton>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>
#include <vector>

#include "entities/company.h"
#include "managers/company_manager.h"
#include "utils/consts.h"
#include "helpers/display_helper.h"
#include "helpers/employee_dialog_helper.h"
#include "helpers/employee_validator.h"
#include "exceptions/exception_handler.h"
#include "managers/file_manager.h"
#include "helpers/project_dialog_helper.h"
#include "helpers/validation_helper.h"

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
    void fireEmployee();
    void searchEmployee();
    void refreshEmployeeTable() const;

    void addProject();
    void editProject();
    void deleteProject();
    void refreshProjectTable() const;
    void openProjectDetails();
    void closeProjectDetails();
    void autoAssignDetailedProject();
    void assignTaskFromDetails(int projectId = -1, int taskId = -1);

    void addProjectTask();
    void assignEmployeeToTask();
    void autoAssignToProject(int projectId = -1);
    void viewProjectAssignments();
    void viewEmployeeHistory();
    void showStatistics() const;
    void refreshAllData() const;

    void addCompany();
    void switchCompany();
    void deleteCompany();
    void refreshCompanyList();

    static QString getDataDirectory();

    friend class MainWindowUIBuilder;
    friend class FileHelper;

   private:
    void setupUI();
    void setupEmployeeTab();
    void setupProjectTab();
    void setupStatisticsTab();
    void drawStatisticsChart(QWidget* widget);

    int getSelectedEmployeeId() const;
    int getSelectedProjectId() const;
    void autoSave() const;
    void autoLoad();

   public:
    void validateAndFixProjectAssignments(Company* company);

   private:
    bool checkCompanyAndHandleError(const QString& actionName);
    void clearAllDataFiles();
    static bool checkDuplicateProjectOnEdit(const QString& projectName,
                                            int excludeId,
                                            const Company* currentCompany);
    void showProjectDetails(int projectId);
    void hideProjectDetails();
    void refreshProjectDetailView();
    void populateProjectTasksTable(const Project& project);
    void selectProjectRowById(int projectId);
    
    void handleEditProjectDialog(int projectId, QDialog& dialog, const ProjectDialogHelper::ProjectDialogFields& fields);
    void handleAddTaskDialog(int projectId, QDialog& dialog, QLineEdit* taskNameEdit, QComboBox* taskTypeCombo, QLineEdit* taskEstHoursEdit, QLineEdit* priorityEdit);
    void handleAssignEmployeeToTaskDialog(int projectId, QDialog& dialog, QComboBox* taskCombo, QComboBox* employeeCombo, QLineEdit* hoursEdit, const std::vector<Task>& tasks);

    static void setupTableWidget(QTableWidget* table,
                                 const QStringList& headers,
                                 const QList<int>& columnWidths,
                                 bool stretchLast = true);
    QWidget* createEmployeeActionButtons(int rowIndex);
    QWidget* createProjectActionButtons(int rowIndex);

    QWidget* companyWidget = nullptr;
    QWidget* employeeTab = nullptr;
    QTableWidget* employeeTable = nullptr;
    QPushButton* employeeAddBtn = nullptr;
    QLineEdit* employeeSearchEdit = nullptr;

    QWidget* projectTab = nullptr;
    QWidget* projectListContainer = nullptr;
    QWidget* projectDetailHeaderContainer = nullptr;
    QWidget* projectDetailContainer = nullptr;
    QTableWidget* projectTable = nullptr;
    QTableWidget* projectTasksTable = nullptr;
    QPushButton* projectAddBtn = nullptr;
    QPushButton* projectDetailCloseBtn = nullptr;
    QPushButton* projectDetailAutoAssignBtn = nullptr;
    QLabel* projectDetailTitle = nullptr;
    QTextEdit* projectDetailInfoText = nullptr;

    QTabWidget* mainTabWidget = nullptr;
    QWidget* statsTab = nullptr;
    QWidget* statisticsChartWidget = nullptr;
    QWidget* statisticsChartInnerWidget = nullptr;
    QTextEdit* statisticsText = nullptr;

    QComboBox* companySelector = nullptr;
    QPushButton* companyAddBtn = nullptr;
    QPushButton* companyDeleteBtn = nullptr;

    std::vector<Company*> companies{};
    Company* currentCompany = nullptr;
    int currentCompanyIndex = -1;
    int nextEmployeeId = 1;
    int nextProjectId = 1;
    int detailedProjectId = -1;
    int pendingTaskSelectionId = -1;
};
