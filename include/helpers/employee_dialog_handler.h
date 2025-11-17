#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>

class Company;

class EmployeeDialogHandler {
   public:
    struct AddEmployeeParams {
        QDialog* dialog;
        Company* company;
        int& nextEmployeeId;
        const QLineEdit* nameEdit;
        const QLineEdit* salaryEdit;
        const QLineEdit* deptEdit;
        QComboBox* typeCombo;
        QComboBox* employmentRateCombo;
        QComboBox* managerProject;
        QLineEdit* devLanguage;
        QLineEdit* devExperience;
        QLineEdit* designerTool;
        QLineEdit* designerProjects;
        QLineEdit* qaTestType;
        QLineEdit* qaBugs;
    };

    struct EditEmployeeParams {
        QDialog* dialog;
        Company* company;
        int employeeId;
        int& nextEmployeeId;
        const QLineEdit* nameEdit;
        const QLineEdit* salaryEdit;
        const QLineEdit* deptEdit;
        QComboBox* employmentRateCombo;
        QComboBox* managerProject;
        QLineEdit* devLanguage;
        QLineEdit* devExperience;
        QLineEdit* designerTool;
        QLineEdit* designerProjects;
        QLineEdit* qaTestType;
        QLineEdit* qaBugs;
        const QString& currentType;
    };

    static bool processAddEmployee(const AddEmployeeParams& params);
    static bool processEditEmployee(const EditEmployeeParams& params);
};
