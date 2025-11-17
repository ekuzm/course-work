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
    static QString getExpectedRoleForProjectPhase(const QString& projectPhase);
    static void populateEmployeeCombo(QComboBox* employeeCombo,
                                      const Company* company, int projectId,
                                      const QString& projectPhase,
                                      int& matchingCount);
    static void setupTaskCombo(QComboBox* taskCombo,
                               const std::vector<class Task>& tasks,
                               int pendingTaskId);
    static void setupHoursEdit(QLineEdit* hoursEdit, const QComboBox* taskCombo,
                               const QComboBox* employeeCombo,
                               const std::vector<class Task>& tasks,
                               const Company* company);
    static void setupEmployeeComboUpdate(QComboBox* employeeCombo,
                                         const QComboBox* taskCombo,
                                         QLabel* taskInfoLabel,
                                         const Company* company, int projectId,
                                         const QString& projectPhase);
};
