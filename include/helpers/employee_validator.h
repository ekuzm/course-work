#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>

class EmployeeValidator {
   public:
    struct ValidateEmployeeTypeFieldsParams {
        const QString& employeeType;
        QDialog* dialog;
        QLineEdit* devLanguage;
        QLineEdit* devExperience;
        QLineEdit* designerTool;
        QLineEdit* designerProjects;
        QLineEdit* qaTestType;
        QLineEdit* qaBugs;
    };
    static bool validateEmployeeTypeFields(
        const ValidateEmployeeTypeFieldsParams& params);
};
