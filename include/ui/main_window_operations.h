#pragma once

#include <QDialog>
#include <QString>
#include <memory>
#include <tuple>
#include <vector>

#include "entities/company.h"
#include "entities/employee.h"
#include "entities/project.h"
#include "entities/task.h"
#include "helpers/project_dialog_helper.h"

class MainWindow;

class EmployeeOperations {
   public:
    static void addEmployee(MainWindow* window);
    static void editEmployee(MainWindow* window);
    static void deleteEmployee(MainWindow* window);
    static void fireEmployee(MainWindow* window);
    static void searchEmployee(MainWindow* window);
    static void refreshEmployeeTable(MainWindow* window);
};

class ProjectOperations {
   public:
    static void addProject(MainWindow* window);
    static void editProject(MainWindow* window);
    static void deleteProject(MainWindow* window);
    static void refreshProjectTable(MainWindow* window);
    static void openProjectDetails(MainWindow* window);
    static void closeProjectDetails(MainWindow* window);
    static void autoAssignDetailedProject(MainWindow* window);
    static void assignTaskFromDetails(MainWindow* window, int projectId = -1,
                                      int taskId = -1);
    static void addProjectTask(MainWindow* window);
    static void assignEmployeeToTask(MainWindow* window);
    static void autoAssignToProject(MainWindow* window, int projectId = -1);
    static void viewProjectAssignments(MainWindow* window);
    static void viewEmployeeHistory(MainWindow* window);
    static void showStatistics(MainWindow* window);
};

class ProjectDetailOperations {
   public:
    static void showProjectDetails(MainWindow* window, int projectId);
    static void hideProjectDetails(MainWindow* window);
    static void refreshProjectDetailView(MainWindow* window);
    static void populateProjectTasksTable(MainWindow* window,
                                          const Project& project);
};

class CompanyOperations {
   public:
    static void addCompany(MainWindow* window);
    static void switchCompany(MainWindow* window);
    static void deleteCompany(MainWindow* window);
    static void refreshCompanyList(MainWindow* window);
    static void initializeCompanySetup(MainWindow* window);
};
