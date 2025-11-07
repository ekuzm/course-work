#pragma once

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
    static void createEmployeeDialog(
        QDialog& dialog, QFormLayout* form, QComboBox*& typeCombo,
        QLineEdit*& nameEdit, QLineEdit*& salaryEdit, QLineEdit*& deptEdit,
        QComboBox*& employmentRateCombo, QComboBox*& managerProject,
        QLineEdit*& devLanguage, QLineEdit*& devExperience,
        QLineEdit*& designerTool, QLineEdit*& designerProjects,
        QLineEdit*& qaTestType, QLineEdit*& qaBugs,
        QLabel*& managerProjectLabel, QLabel*& devLanguageLabel,
        QLabel*& devExperienceLabel, QLabel*& designerToolLabel,
        QLabel*& designerProjectsLabel, QLabel*& qaTestTypeLabel,
        QLabel*& qaBugsLabel);
    static void createEditEmployeeDialog(
        QDialog& dialog, QFormLayout* form, std::shared_ptr<Employee> employee,
        QLineEdit*& nameEdit, QLineEdit*& salaryEdit, QLineEdit*& deptEdit,
        QComboBox*& employmentRateCombo, QComboBox*& managerProject,
        QLineEdit*& devLanguage, QLineEdit*& devExperience,
        QLineEdit*& designerTool, QLineEdit*& designerProjects,
        QLineEdit*& qaTestType, QLineEdit*& qaBugs,
        QLabel*& managerProjectLabel, QLabel*& devLanguageLabel,
        QLabel*& devExperienceLabel, QLabel*& designerToolLabel,
        QLabel*& designerProjectsLabel, QLabel*& qaTestTypeLabel,
        QLabel*& qaBugsLabel);
    static void populateEmployeeFields(
        QComboBox* employmentRateCombo, QComboBox* managerProject,
        QLineEdit* devLanguage, QLineEdit* devExperience,
        QLineEdit* designerTool, QLineEdit* designerProjects,
        QLineEdit* qaTestType, QLineEdit* qaBugs, QLabel* managerProjectLabel,
        QLabel* devLanguageLabel, QLabel* devExperienceLabel,
        QLabel* designerToolLabel, QLabel* designerProjectsLabel,
        QLabel* qaTestTypeLabel, QLabel* qaBugsLabel,
        std::shared_ptr<Employee> employee);
    static bool checkDuplicateEmployee(const QString& name,
                                       const Company* currentCompany);
    static bool checkDuplicateEmployeeOnEdit(const QString& name, int excludeId,
                                             const Company* currentCompany);
    static std::shared_ptr<Employee> createEmployeeFromType(
        const QString& employeeType, int employeeId, const QString& name,
        double salary, const QString& department,
        QComboBox* employmentRateCombo, QComboBox* managerProject,
        QLineEdit* devLanguage, QLineEdit* devExperience,
        QLineEdit* designerTool, QLineEdit* designerProjects,
        QLineEdit* qaTestType, QLineEdit* qaBugs);

   private:
    static QComboBox* createEmploymentRateCombo();
    static void showManagerFields(QLabel* managerProjectLabel,
                                  QComboBox* managerProject, bool show);
    static void showDeveloperFields(QLabel* devLanguageLabel,
                                    QLineEdit* devLanguage,
                                    QLabel* devExperienceLabel,
                                    QLineEdit* devExperience, bool show);
    static void showDesignerFields(QLabel* designerToolLabel,
                                   QLineEdit* designerTool,
                                   QLabel* designerProjectsLabel,
                                   QLineEdit* designerProjects, bool show);
    static void showQaFields(QLabel* qaTestTypeLabel, QLineEdit* qaTestType,
                             QLabel* qaBugsLabel, QLineEdit* qaBugs, bool show);
};
