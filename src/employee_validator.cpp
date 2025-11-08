#include "employee_validator.h"

#include <QDialog>

#include "consts.h"
#include "validation_helper.h"

bool EmployeeValidator::validateEmployeeTypeFields(const QString& employeeType, QDialog* dialog,
                                                   QLineEdit* devLanguage, QLineEdit* devExperience,
                                                   QLineEdit* designerTool, QLineEdit* designerProjects,
                                                   QLineEdit* qaTestType, QLineEdit* qaBugs) {
    if (employeeType == "Developer") {
        if (!ValidationHelper::validateNonEmpty(devLanguage->text().trimmed(), "Programming language", dialog)) {
            return false;
        }
        int years = 0;
        if (!ValidationHelper::validateInt(devExperience->text().trimmed(), years, 0, kMaxYearsOfExperience,
                                          "Years of experience", dialog)) {
            return false;
        }
    } else if (employeeType == "Designer") {
        if (!ValidationHelper::validateNonEmpty(designerTool->text().trimmed(), "Design tool", dialog)) {
            return false;
        }
        int projects = 0;
        if (!ValidationHelper::validateInt(designerProjects->text().trimmed(), projects, 0, kMaxNumberOfProjects,
                                          "Number of projects", dialog)) {
            return false;
        }
    } else if (employeeType == "QA") {
        if (!ValidationHelper::validateNonEmpty(qaTestType->text().trimmed(), "Testing type", dialog)) {
            return false;
        }
        int bugs = 0;
        if (!ValidationHelper::validateInt(qaBugs->text().trimmed(), bugs, 0, kMaxBugsFound,
                                          "Bugs found", dialog)) {
            return false;
        }
    }
    return true;
}





