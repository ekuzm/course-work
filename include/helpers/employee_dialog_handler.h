#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>

class Company;

class EmployeeDialogHandler {
   public:
    static bool processAddEmployee(
        QDialog* dialog, Company* company, int& nextEmployeeId,
        QLineEdit* nameEdit, QLineEdit* salaryEdit, QLineEdit* deptEdit,
        QComboBox* typeCombo, QComboBox* employmentRateCombo,
        QComboBox* managerProject, QLineEdit* devLanguage,
        QLineEdit* devExperience, QLineEdit* designerTool,
        QLineEdit* designerProjects, QLineEdit* qaTestType, QLineEdit* qaBugs);
    static bool processEditEmployee(
        QDialog* dialog, Company* company, int employeeId, int& nextEmployeeId,
        QLineEdit* nameEdit, QLineEdit* salaryEdit, QLineEdit* deptEdit,
        QComboBox* employmentRateCombo, QComboBox* managerProject,
        QLineEdit* devLanguage, QLineEdit* devExperience,
        QLineEdit* designerTool, QLineEdit* designerProjects,
        QLineEdit* qaTestType, QLineEdit* qaBugs, const QString& currentType);

   private:
    static bool validateEmployeeFields(QDialog* dialog,
                                       QLineEdit* nameEdit,
                                       QLineEdit* salaryEdit,
                                       QLineEdit* deptEdit,
                                       QString& name,
                                       double& salary,
                                       QString& department);
};
