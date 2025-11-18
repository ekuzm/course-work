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
#include "exceptions/exception_handler.h"
#include "helpers/display_helper.h"
#include "helpers/employee_dialog_helper.h"
#include "helpers/employee_validator.h"
#include "helpers/project_dialog_helper.h"
#include "helpers/validation_helper.h"
#include "managers/company_manager.h"
#include "managers/file_manager.h"
#include "ui/main_window_helpers.h"
#include "ui/main_window_operations.h"
#include "ui/main_window_ui_components.h"
#include "utils/consts.h"

class EmployeeDialogHelper;

class MainWindow : public QMainWindow {
    Q_OBJECT

    friend class EmployeeOperations;
    friend class ProjectOperations;
    friend class ProjectDetailOperations;
    friend class CompanyOperations;

   public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    void refreshAllData();
    void autoSave();
    void selectProjectRowById(int projectId);

   protected:
    void closeEvent(QCloseEvent* event) override;

    static QString getDataDirectory();

    friend class MainWindowUIBuilder;
    friend class FileHelper;
    friend struct AddEmployeeButtonParams;
    friend struct EditEmployeeButtonParams;
    friend void handleAddProjectButtonClick(
        MainWindow*, QDialog&, const ProjectDialogHelper::ProjectDialogFields&);

   private:
    void autoLoad();

   public:
    void validateAndFixProjectAssignments(const Company* company);

    QTabWidget* mainTabWidget = nullptr;
    EmployeeTabUI employeeUI;
    ProjectTabUI projectUI;
    StatisticsTabUI statisticsUI;
    CompanyUI companyUI;

    std::vector<Company*> companies{};
    Company* currentCompany = nullptr;
    int currentCompanyIndex = -1;
    int nextEmployeeId = 1;
    int nextProjectId = 1;
    int detailedProjectId = -1;
    int pendingTaskSelectionId = -1;
};
