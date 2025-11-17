#pragma once

#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <memory>

#include "entities/company.h"
#include "entities/employee.h"

class EmployeeDialogHelper {
   public:
    struct CreateEmployeeDialogFields {
        QComboBox*& typeCombo;
        QLineEdit*& nameEdit;
        QLineEdit*& salaryEdit;
        QLineEdit*& deptEdit;
        QComboBox*& employmentRateCombo;
        QComboBox*& managerProject;
        QLineEdit*& devLanguage;
        QLineEdit*& devExperience;
        QLineEdit*& designerTool;
        QLineEdit*& designerProjects;
        QLineEdit*& qaTestType;
        QLineEdit*& qaBugs;
        QLabel*& managerProjectLabel;
        QLabel*& devLanguageLabel;
        QLabel*& devExperienceLabel;
        QLabel*& designerToolLabel;
        QLabel*& designerProjectsLabel;
        QLabel*& qaTestTypeLabel;
        QLabel*& qaBugsLabel;
    };
    static void createEmployeeDialog(const QDialog& dialog, QFormLayout* form,
                                     CreateEmployeeDialogFields& fields);
    struct CreateEditEmployeeDialogFields {
        QLineEdit*& nameEdit;
        QLineEdit*& salaryEdit;
        QLineEdit*& deptEdit;
        QComboBox*& employmentRateCombo;
        QComboBox*& managerProject;
        QLineEdit*& devLanguage;
        QLineEdit*& devExperience;
        QLineEdit*& designerTool;
        QLineEdit*& designerProjects;
        QLineEdit*& qaTestType;
        QLineEdit*& qaBugs;
        QLabel*& managerProjectLabel;
        QLabel*& devLanguageLabel;
        QLabel*& devExperienceLabel;
        QLabel*& designerToolLabel;
        QLabel*& designerProjectsLabel;
        QLabel*& qaTestTypeLabel;
        QLabel*& qaBugsLabel;
    };
    static void createEditEmployeeDialog(
        const QDialog& dialog, QFormLayout* form,
        std::shared_ptr<Employee> employee,
        CreateEditEmployeeDialogFields& fields);
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
    struct SetAllFieldsVisibleParams {
        QLabel* managerProjectLabel;
        QComboBox* managerProject;
        QLabel* devLanguageLabel;
        QLineEdit* devLanguage;
        QLabel* devExperienceLabel;
        QLineEdit* devExperience;
        QLabel* designerToolLabel;
        QLineEdit* designerTool;
        QLabel* designerProjectsLabel;
        QLineEdit* designerProjects;
        QLabel* qaTestTypeLabel;
        QLineEdit* qaTestType;
        QLabel* qaBugsLabel;
        QLineEdit* qaBugs;
    };
    static void setAllFieldsVisible(const SetAllFieldsVisibleParams& params);

    struct PopulateEmployeeFieldsParams {
        QComboBox* employmentRateCombo;
        QComboBox* managerProject;
        QLineEdit* devLanguage;
        QLineEdit* devExperience;
        QLineEdit* designerTool;
        QLineEdit* designerProjects;
        QLineEdit* qaTestType;
        QLineEdit* qaBugs;
        QLabel* managerProjectLabel;
        QLabel* devLanguageLabel;
        QLabel* devExperienceLabel;
        QLabel* designerToolLabel;
        QLabel* designerProjectsLabel;
        QLabel* qaTestTypeLabel;
        QLabel* qaBugsLabel;
        std::shared_ptr<Employee> employee;
    };
    static void populateEmployeeFields(
        const PopulateEmployeeFieldsParams& params);
    static bool checkDuplicateEmployee(const QString& name,
                                       const Company* currentCompany);
    static bool checkDuplicateEmployeeOnEdit(const QString& name, int excludeId,
                                             const Company* currentCompany);
    struct CreateEmployeeFromTypeParams {
        const QString& employeeType;
        int employeeId;
        const QString& name;
        double salary;
        const QString& department;
        QComboBox* employmentRateCombo;
        QComboBox* managerProject;
        QLineEdit* devLanguage;
        QLineEdit* devExperience;
        QLineEdit* designerTool;
        QLineEdit* designerProjects;
        QLineEdit* qaTestType;
        QLineEdit* qaBugs;
    };
    static std::shared_ptr<Employee> createEmployeeFromType(
        const CreateEmployeeFromTypeParams& params);

   private:
    static QComboBox* createEmploymentRateCombo();
};
