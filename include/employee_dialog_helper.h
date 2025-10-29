#ifndef EMPLOYEE_DIALOG_HELPER_H
#define EMPLOYEE_DIALOG_HELPER_H

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <memory>

#include "company.h"
#include "employee.h"

class EmployeeDialogHelper {
   public:
    struct EmployeeFormWidgets {
        QComboBox* typeCombo = nullptr;
        QLineEdit* nameEdit = nullptr;
        QLineEdit* salaryEdit = nullptr;
        QLineEdit* deptEdit = nullptr;
        QLineEdit* managerProject = nullptr;
        QLineEdit* managerTeamSize = nullptr;
        QLineEdit* devLanguage = nullptr;
        QLineEdit* devExperience = nullptr;
        QLineEdit* designerTool = nullptr;
        QLineEdit* designerProjects = nullptr;
        QLineEdit* qaTestType = nullptr;
        QLineEdit* qaBugs = nullptr;
        QLabel* managerProjectLabel = nullptr;
        QLabel* managerTeamSizeLabel = nullptr;
        QLabel* devLanguageLabel = nullptr;
        QLabel* devExperienceLabel = nullptr;
        QLabel* designerToolLabel = nullptr;
        QLabel* designerProjectsLabel = nullptr;
        QLabel* qaTestTypeLabel = nullptr;
        QLabel* qaBugsLabel = nullptr;
    };

    static EmployeeFormWidgets createEmployeeDialog(QDialog& /* dialog */,
                                                      QFormLayout* form);
    static EmployeeFormWidgets createEditEmployeeDialog(
        QDialog& /* dialog */, QFormLayout* form, std::shared_ptr<Employee> employee);
    static void populateEmployeeFields(const EmployeeFormWidgets& widgets,
                                       std::shared_ptr<Employee> employee);
    static bool validateEmployeeInput(const QString& name, double salary,
                                      const QString& department);
    static bool checkDuplicateEmployee(const QString& name,
                                       Company* currentCompany);
    static bool checkDuplicateEmployeeOnEdit(const QString& name, int excludeId,
                                             Company* currentCompany);
    static std::shared_ptr<Employee> createEmployeeFromType(
        const QString& employeeType, int employeeId, const QString& name,
        double salary, const QString& department,
        const EmployeeFormWidgets& widgets);

   private:
    static void showManagerFields(const EmployeeFormWidgets& widgets, bool show);
    static void showDeveloperFields(const EmployeeFormWidgets& widgets, bool show);
    static void showDesignerFields(const EmployeeFormWidgets& widgets, bool show);
    static void showQaFields(const EmployeeFormWidgets& widgets, bool show);
};

#endif  // EMPLOYEE_DIALOG_HELPER_H

