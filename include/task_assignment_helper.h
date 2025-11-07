#pragma once

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QString>
#include <vector>

class Company;
class Project;

class TaskAssignmentHelper {
   public:
    static QString getExpectedRoleForProjectStatus(const QString& projectStatus);
    static void populateEmployeeCombo(QComboBox* employeeCombo, const Company* company, 
                                     int projectId, const QString& projectStatus,
                                     int& matchingCount);
    static void setupTaskCombo(QComboBox* taskCombo, const std::vector<class Task>& tasks, 
                               int pendingTaskId);
    static void setupHoursEdit(QLineEdit* hoursEdit, QComboBox* taskCombo,
                               const std::vector<class Task>& tasks);
    static void setupEmployeeComboUpdate(QComboBox* employeeCombo, QComboBox* taskCombo,
                                        QLabel* taskInfoLabel, const Company* company,
                                        int projectId, const QString& projectStatus);
};

