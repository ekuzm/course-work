#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QString>

class EmployeeValidator {
   public:
    static bool validateEmployeeTypeFields(const QString& employeeType, QDialog* dialog,
                                          QLineEdit* devLanguage, QLineEdit* devExperience,
                                          QLineEdit* designerTool, QLineEdit* designerProjects,
                                          QLineEdit* qaTestType, QLineEdit* qaBugs);
};


