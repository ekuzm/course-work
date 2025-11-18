#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QTableWidget>
#include <memory>
#include <tuple>
#include <vector>

#include "entities/company.h"
#include "entities/project.h"
#include "entities/task.h"
#include "helpers/project_dialog_helper.h"

class MainWindow;

class MainWindowSelectionHelper {
   public:
    static int getSelectedEmployeeId(const MainWindow* window);
    static int getSelectedProjectId(const MainWindow* window);
};

class MainWindowProjectDialogHandler {
   public:
    struct ProjectEditData {
        QString projectName;
        double projectBudget;
        QString selectedPhase;
        QString clientName;
        int estimatedHours;
        QString newName;
        QString newDescription;
        QString newPhase;
        QDate newStartDate;
        QDate newEndDate;
        QString newClientName;
    };

    static void handleEditProjectDialog(
        MainWindow* window, int projectId, QDialog& dialog,
        const ProjectDialogHelper::ProjectDialogFields& fields);
    static void handleAddTaskDialog(MainWindow* window, int projectId,
                                    QDialog& dialog,
                                    const QLineEdit* taskNameEdit,
                                    const QComboBox* taskTypeCombo,
                                    const QLineEdit* taskEstHoursEdit,
                                    const QLineEdit* priorityEdit);
    static void handleAssignEmployeeToTaskDialog(
        MainWindow* window, int projectId, QDialog& dialog,
        const QComboBox* taskCombo, const QComboBox* employeeCombo,
        const QLineEdit* hoursEdit, const std::vector<Task>& tasks);

   private:
    static bool validateProjectEditFields(
        QDialog& dialog, const ProjectDialogHelper::ProjectDialogFields& fields,
        ProjectEditData& data);
    static bool checkProjectChanges(const Project* oldProject,
                                    const ProjectEditData& data);
    static bool validatePhaseTransition(QDialog& dialog,
                                        const QString& currentPhase,
                                        const QString& newPhase);
    static void updateProjectWithChanges(
        MainWindow* window, int projectId, const ProjectEditData& data,
        const ProjectDialogHelper::ProjectDialogFields& fields,
        const Project* oldProject);
    static void showProjectUpdateSuccess(QDialog& dialog,
                                         const ProjectEditData& data);
};

class MainWindowProjectDetailHelper {
   public:
    static void showProjectDetails(MainWindow* window, int projectId);
    static void hideProjectDetails(MainWindow* window);
    static void refreshProjectDetailView(MainWindow* window);
    static void populateProjectTasksTable(MainWindow* window,
                                          const Project& project);
};

class MainWindowTaskAssignmentHelper {
   public:
    static void removeEmployeeFromProjectTasks(const MainWindow* window,
                                               int employeeId, int projectId,
                                               Project* mutableProject,
                                               double employeeHourlyRate);
    static void collectTaskAssignments(
        const MainWindow* window, int projectId,
        const std::vector<Task>& savedTasks,
        std::vector<std::tuple<int, int, int, int>>& savedTaskAssignments);
    static void handleEmployeeActiveAssignments(
        const MainWindow* window, int employeeId,
        const std::shared_ptr<Employee>& employee);
};

class MainWindowValidationHelper {
   public:
    static bool checkCompanyAndHandleError(MainWindow* window,
                                           const QString& actionName);
    static bool checkDuplicateProjectOnEdit(const QString& projectName,
                                            int excludeId,
                                            const Company* currentCompany);
    static void validateAndFixProjectAssignments(MainWindow* window,
                                                 const Company* company);
};

class MainWindowDataOperations {
   public:
    static void refreshAllData(MainWindow* window);
    static void autoSave(MainWindow* window);
    static void autoLoad(MainWindow* window);
    static void selectProjectRowById(MainWindow* window, int projectId);
};

class MainWindowUIHelper {
   public:
    static void setupUI(MainWindow* window);
    static void setupEmployeeTab(MainWindow* window);
    static void setupProjectTab(MainWindow* window);
    static void setupStatisticsTab(MainWindow* window);
    static void drawStatisticsChart(MainWindow* window, QWidget* widget);
    static void clearAllDataFiles(MainWindow* window);
    static void setupTableWidget(QTableWidget* table,
                                 const QStringList& headers,
                                 const QList<int>& columnWidths,
                                 bool stretchLast = true);
    static QWidget* createEmployeeActionButtons(MainWindow* window,
                                                int rowIndex);
    static QWidget* createProjectActionButtons(MainWindow* window,
                                               int rowIndex);
    static void selectProjectRowById(MainWindow* window, int projectId);
};
