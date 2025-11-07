#pragma once

#include <QTableWidget>

class Company;
class MainWindow;
class Project;
class Task;

class ProjectHelper {
   public:
    static void populateProjectTasksTable(QTableWidget* table, const Project& project,
                                         MainWindow* mainWindow);
    static void clearProjectAllocatedHoursIfNoEmployees(Company* company, int projectId);
    static bool hasAssignedEmployees(const Company* company, int projectId);
};

