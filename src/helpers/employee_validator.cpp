#include "helpers/employee_validator.h"

#include <QDialog>

#include "helpers/validation_helper.h"
#include "utils/consts.h"

bool EmployeeValidator::validateEmployeeTypeFields(
    const ValidateEmployeeTypeFieldsParams& params) {
    if (params.employeeType == "Developer") {
        if (!ValidationHelper::validateNonEmpty(
                params.devLanguage->text().trimmed(), "Programming language",
                params.dialog)) {
            return false;
        }
        double years = 0.0;
        if (!ValidationHelper::validateDouble(
                params.devExperience->text().trimmed(), years, 0.0, 50.0,
                "Years of experience", params.dialog)) {
            return false;
        }
    } else if (params.employeeType == "Designer") {
        if (!ValidationHelper::validateNonEmpty(
                params.designerTool->text().trimmed(), "Design tool",
                params.dialog)) {
            return false;
        }
        int projects = 0;
        if (!ValidationHelper::validateInt(
                params.designerProjects->text().trimmed(), projects, 0,
                kMaxNumberOfProjects, "Number of projects", params.dialog)) {
            return false;
        }
    } else if (params.employeeType == "QA") {
        if (!ValidationHelper::validateNonEmpty(
                params.qaTestType->text().trimmed(), "Testing type",
                params.dialog)) {
            return false;
        }
        int bugs = 0;
        if (!ValidationHelper::validateInt(params.qaBugs->text().trimmed(),
                                           bugs, 0, kMaxBugsFound, "Bugs found",
                                           params.dialog)) {
            return false;
        }
    }
    return true;
}
