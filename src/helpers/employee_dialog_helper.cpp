#include "helpers/employee_dialog_helper.h"

#include <QFormLayout>
#include <QMessageBox>
#include <algorithm>
#include <ranges>

#include "entities/derived_employees.h"
#include "utils/consts.h"

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

QComboBox* EmployeeDialogHelper::createEmploymentRateCombo() {
    auto* combo = new QComboBox();
    combo->addItem("Full Time (1.0)", 1.0);
    combo->addItem("Three Quarters (0.75)", 0.75);
    combo->addItem("Half Time (0.5)", 0.5);
    combo->addItem("Quarter Time (0.25)", 0.25);
    combo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    return combo;
}

void EmployeeDialogHelper::showManagerFields(QLabel* managerProjectLabel,
                                             QComboBox* managerProject,
                                             bool show) {
    managerProjectLabel->setVisible(show);
    managerProject->setVisible(show);
}

void EmployeeDialogHelper::showDeveloperFields(QLabel* devLanguageLabel,
                                               QLineEdit* devLanguage,
                                               QLabel* devExperienceLabel,
                                               QLineEdit* devExperience,
                                               bool show) {
    devLanguageLabel->setVisible(show);
    devLanguage->setVisible(show);
    devExperienceLabel->setVisible(show);
    devExperience->setVisible(show);
}

void EmployeeDialogHelper::showDesignerFields(QLabel* designerToolLabel,
                                              QLineEdit* designerTool,
                                              QLabel* designerProjectsLabel,
                                              QLineEdit* designerProjects,
                                              bool show) {
    designerToolLabel->setVisible(show);
    designerTool->setVisible(show);
    designerProjectsLabel->setVisible(show);
    designerProjects->setVisible(show);
}

void EmployeeDialogHelper::showQaFields(QLabel* qaTestTypeLabel,
                                        QLineEdit* qaTestType,
                                        QLabel* qaBugsLabel, QLineEdit* qaBugs,
                                        bool show) {
    qaTestTypeLabel->setVisible(show);
    qaTestType->setVisible(show);
    qaBugsLabel->setVisible(show);
    qaBugs->setVisible(show);
}

void EmployeeDialogHelper::setAllFieldsVisible(
    const SetAllFieldsVisibleParams& params) {
    if (params.managerProjectLabel)
        params.managerProjectLabel->setVisible(true);
    if (params.managerProject) params.managerProject->setVisible(true);
    if (params.devLanguageLabel) params.devLanguageLabel->setVisible(true);
    if (params.devLanguage) params.devLanguage->setVisible(true);
    if (params.devExperienceLabel) params.devExperienceLabel->setVisible(true);
    if (params.devExperience) params.devExperience->setVisible(true);
    if (params.designerToolLabel) params.designerToolLabel->setVisible(true);
    if (params.designerTool) params.designerTool->setVisible(true);
    if (params.designerProjectsLabel)
        params.designerProjectsLabel->setVisible(true);
    if (params.designerProjects) params.designerProjects->setVisible(true);
    if (params.qaTestTypeLabel) params.qaTestTypeLabel->setVisible(true);
    if (params.qaTestType) params.qaTestType->setVisible(true);
    if (params.qaBugsLabel) params.qaBugsLabel->setVisible(true);
    if (params.qaBugs) params.qaBugs->setVisible(true);
}

void EmployeeDialogHelper::createEmployeeDialog(
    const QDialog& dialog, QFormLayout* form,
    CreateEmployeeDialogFields& fields) {
    (void)dialog;

    fields.typeCombo = new QComboBox();
    fields.typeCombo->addItems({"Manager", "Developer", "Designer", "QA"});
    fields.typeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Type:", fields.typeCombo);

    fields.nameEdit = new QLineEdit();
    fields.nameEdit->setPlaceholderText("e.g., John Doe");
    form->addRow("Name:", fields.nameEdit);

    fields.salaryEdit = new QLineEdit();
    fields.salaryEdit->setPlaceholderText("e.g., 5000");
    form->addRow("Salary ($):", fields.salaryEdit);

    fields.deptEdit = new QLineEdit();
    fields.deptEdit->setPlaceholderText("e.g., Development, Design");
    form->addRow("Department:", fields.deptEdit);

    fields.employmentRateCombo = createEmploymentRateCombo();
    form->addRow("Employment Rate:", fields.employmentRateCombo);

    fields.managerProject = new QComboBox();
    fields.managerProject->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    fields.managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(fields.managerProjectLabel, fields.managerProject);
    fields.managerProjectLabel->setVisible(false);
    fields.managerProject->setVisible(false);

    fields.devLanguage = new QLineEdit();
    fields.devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    fields.devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(fields.devLanguageLabel, fields.devLanguage);
    fields.devLanguageLabel->setVisible(false);
    fields.devLanguage->setVisible(false);

    fields.devExperience = new QLineEdit();
    fields.devExperience->setPlaceholderText("e.g., 3.5 (0.0-50.0)");
    fields.devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(fields.devExperienceLabel, fields.devExperience);
    fields.devExperienceLabel->setVisible(false);
    fields.devExperience->setVisible(false);

    fields.designerTool = new QLineEdit();
    fields.designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    fields.designerToolLabel = new QLabel("Design Tool:");
    form->addRow(fields.designerToolLabel, fields.designerTool);
    fields.designerToolLabel->setVisible(false);
    fields.designerTool->setVisible(false);

    fields.designerProjects = new QLineEdit();
    fields.designerProjects->setPlaceholderText("e.g., 10");
    fields.designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(fields.designerProjectsLabel, fields.designerProjects);
    fields.designerProjectsLabel->setVisible(false);
    fields.designerProjects->setVisible(false);

    fields.qaTestType = new QLineEdit();
    fields.qaTestType->setPlaceholderText("e.g., Manual, Automated");
    fields.qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(fields.qaTestTypeLabel, fields.qaTestType);
    fields.qaTestTypeLabel->setVisible(false);
    fields.qaTestType->setVisible(false);

    fields.qaBugs = new QLineEdit();
    fields.qaBugs->setPlaceholderText("e.g., 25");
    fields.qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(fields.qaBugsLabel, fields.qaBugs);
    fields.qaBugsLabel->setVisible(false);
    fields.qaBugs->setVisible(false);

    auto updateFields = [&fields](int index) {
        showManagerFields(fields.managerProjectLabel, fields.managerProject,
                          index == 0);
        showDeveloperFields(fields.devLanguageLabel, fields.devLanguage,
                            fields.devExperienceLabel, fields.devExperience,
                            index == 1);
        showDesignerFields(fields.designerToolLabel, fields.designerTool,
                           fields.designerProjectsLabel,
                           fields.designerProjects, index == 2);
        showQaFields(fields.qaTestTypeLabel, fields.qaTestType,
                     fields.qaBugsLabel, fields.qaBugs, index == 3);
    };

    QObject::connect(fields.typeCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     updateFields);
    updateFields(0);
}

void EmployeeDialogHelper::createEditEmployeeDialog(
    const QDialog& dialog, QFormLayout* form,
    std::shared_ptr<Employee> employee,
    CreateEditEmployeeDialogFields& fields) {
    (void)dialog;

    auto* typeLabel = new QLabel("Type:");
    auto* typeDisplay = new QLineEdit();
    typeDisplay->setText(employee->getEmployeeType());
    typeDisplay->setReadOnly(true);
    typeDisplay->setStyleSheet("QLineEdit { background-color: #f5f5f5; }");
    form->addRow(typeLabel, typeDisplay);

    fields.nameEdit = new QLineEdit();
    fields.nameEdit->setPlaceholderText("e.g., John Doe");
    fields.nameEdit->setText(employee->getName());
    form->addRow("Name:", fields.nameEdit);

    fields.salaryEdit = new QLineEdit();
    fields.salaryEdit->setPlaceholderText("e.g., 5000");
    fields.salaryEdit->setText(QString::number(employee->getSalary(), 'f', 2));
    form->addRow("Salary ($):", fields.salaryEdit);

    fields.deptEdit = new QLineEdit();
    fields.deptEdit->setPlaceholderText("e.g., Development, Design");
    fields.deptEdit->setText(employee->getDepartment());
    form->addRow("Department:", fields.deptEdit);

    fields.employmentRateCombo = createEmploymentRateCombo();
    form->addRow("Employment Rate:", fields.employmentRateCombo);

    fields.managerProject = new QComboBox();
    fields.managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(fields.managerProjectLabel, fields.managerProject);

    fields.devLanguage = new QLineEdit();
    fields.devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    fields.devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(fields.devLanguageLabel, fields.devLanguage);

    fields.devExperience = new QLineEdit();
    fields.devExperience->setPlaceholderText("e.g., 3.5 (0.0-50.0)");
    fields.devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(fields.devExperienceLabel, fields.devExperience);

    fields.designerTool = new QLineEdit();
    fields.designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    fields.designerToolLabel = new QLabel("Design Tool:");
    form->addRow(fields.designerToolLabel, fields.designerTool);

    fields.designerProjects = new QLineEdit();
    fields.designerProjects->setPlaceholderText("e.g., 10");
    fields.designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(fields.designerProjectsLabel, fields.designerProjects);

    fields.qaTestType = new QLineEdit();
    fields.qaTestType->setPlaceholderText("e.g., Manual, Automated");
    fields.qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(fields.qaTestTypeLabel, fields.qaTestType);

    fields.qaBugs = new QLineEdit();
    fields.qaBugs->setPlaceholderText("e.g., 25");
    fields.qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(fields.qaBugsLabel, fields.qaBugs);

    EmployeeDialogHelper::PopulateEmployeeFieldsParams populateParams{
        fields.employmentRateCombo,
        fields.managerProject,
        fields.devLanguage,
        fields.devExperience,
        fields.designerTool,
        fields.designerProjects,
        fields.qaTestType,
        fields.qaBugs,
        fields.managerProjectLabel,
        fields.devLanguageLabel,
        fields.devExperienceLabel,
        fields.designerToolLabel,
        fields.designerProjectsLabel,
        fields.qaTestTypeLabel,
        fields.qaBugsLabel,
        employee};
    populateEmployeeFields(populateParams);
}

void EmployeeDialogHelper::populateEmployeeFields(
    const PopulateEmployeeFieldsParams& params) {
    QString currentType = params.employee->getEmployeeType();

    double employmentRate = params.employee->getEmploymentRate();
    if (int index = params.employmentRateCombo->findData(employmentRate);
        index != -1) {
        params.employmentRateCombo->setCurrentIndex(index);
    }

    showManagerFields(params.managerProjectLabel, params.managerProject, false);
    showDeveloperFields(params.devLanguageLabel, params.devLanguage,
                        params.devExperienceLabel, params.devExperience, false);
    showDesignerFields(params.designerToolLabel, params.designerTool,
                       params.designerProjectsLabel, params.designerProjects,
                       false);
    showQaFields(params.qaTestTypeLabel, params.qaTestType, params.qaBugsLabel,
                 params.qaBugs, false);

    if (currentType == "Manager") {
        showManagerFields(params.managerProjectLabel, params.managerProject,
                          true);
    } else if (currentType == "Developer") {
        if (const auto* developer =
                dynamic_cast<const Developer*>(params.employee.get());
            developer != nullptr) {
            params.devLanguage->setText(developer->getProgrammingLanguage());
            params.devExperience->setText(
                QString::number(developer->getYearsOfExperience(), 'f', 1));
        }
        showDeveloperFields(params.devLanguageLabel, params.devLanguage,
                            params.devExperienceLabel, params.devExperience,
                            true);
    } else if (currentType == "Designer") {
        if (const auto* designer =
                dynamic_cast<const Designer*>(params.employee.get());
            designer != nullptr) {
            params.designerTool->setText(designer->getDesignTool());
            params.designerProjects->setText(
                QString::number(designer->getNumberOfProjects()));
        }
        showDesignerFields(params.designerToolLabel, params.designerTool,
                           params.designerProjectsLabel,
                           params.designerProjects, true);
    } else if (currentType == "QA") {
        if (const auto* qaEmployee =
                dynamic_cast<const QA*>(params.employee.get());
            qaEmployee != nullptr) {
            params.qaTestType->setText(qaEmployee->getTestingType());
            params.qaBugs->setText(QString::number(qaEmployee->getBugsFound()));
        }
        showQaFields(params.qaTestTypeLabel, params.qaTestType,
                     params.qaBugsLabel, params.qaBugs, true);
    }
}

bool EmployeeDialogHelper::checkDuplicateEmployee(
    const QString& name, const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingEmployees = currentCompany->getAllEmployees();
    auto duplicateFound =
        std::ranges::any_of(existingEmployees, [&name](const auto& employee) {
            return employee != nullptr &&
                   employee->getName().toLower() == name.toLower();
        });
    return !duplicateFound;
}

bool EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(
    const QString& name, int excludeId, const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingEmployees = currentCompany->getAllEmployees();
    auto duplicateFound = std::ranges::any_of(
        existingEmployees, [&name, excludeId](const auto& employee) {
            return employee != nullptr && employee->getId() != excludeId &&
                   employee->getName().toLower() == name.toLower();
        });
    return !duplicateFound;
}

std::shared_ptr<Employee> EmployeeDialogHelper::createEmployeeFromType(
    const CreateEmployeeFromTypeParams& params) {
    double employmentRate =
        params.employmentRateCombo->currentData().toDouble();

    if (params.employeeType == "Manager") {
        int projectId = params.managerProject->currentData().toInt();

        return std::make_shared<Manager>(params.employeeId, params.name,
                                         params.salary, params.department,
                                         projectId, employmentRate);
    }

    if (params.employeeType == "Developer") {
        QString language = params.devLanguage->text().trimmed();
        if (language.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        double years =
            params.devExperience->text().toDouble(&conversionSuccess);
        if (!conversionSuccess || years < 0.0 || years > 50.0) {
            return nullptr;
        }

        return std::make_shared<Developer>(params.employeeId, params.name,
                                           params.salary, params.department,
                                           language, years, employmentRate);
    }

    if (params.employeeType == "Designer") {
        QString tool = params.designerTool->text().trimmed();
        if (tool.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int projects =
            params.designerProjects->text().toInt(&conversionSuccess);
        if (!conversionSuccess || projects < 0 ||
            projects > kMaxNumberOfProjects) {
            return nullptr;
        }

        return std::make_shared<Designer>(params.employeeId, params.name,
                                          params.salary, params.department,
                                          tool, projects, employmentRate);
    }

    if (params.employeeType == "QA") {
        QString testType = params.qaTestType->text().trimmed();
        if (testType.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int bugs = params.qaBugs->text().toInt(&conversionSuccess);
        if (!conversionSuccess || bugs < 0 || bugs > kMaxBugsFound) {
            return nullptr;
        }

        return std::make_shared<QA>(params.employeeId, params.name,
                                    params.salary, params.department, testType,
                                    bugs, employmentRate);
    }

    return nullptr;
}
