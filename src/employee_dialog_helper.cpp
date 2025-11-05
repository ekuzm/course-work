#include "employee_dialog_helper.h"

#include <QMessageBox>
#include <algorithm>
#include <ranges>

#include "consts.h"
#include "derived_employees.h"

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

void EmployeeDialogHelper::createEmployeeDialog(
    QDialog& dialog, QFormLayout* form, QComboBox*& typeCombo,
    QLineEdit*& nameEdit, QLineEdit*& salaryEdit, QLineEdit*& deptEdit,
    QComboBox*& employmentRateCombo, QComboBox*& managerProject,
    QLineEdit*& devLanguage, QLineEdit*& devExperience,
    QLineEdit*& designerTool, QLineEdit*& designerProjects,
    QLineEdit*& qaTestType, QLineEdit*& qaBugs, QLabel*& managerProjectLabel,
    QLabel*& devLanguageLabel, QLabel*& devExperienceLabel,
    QLabel*& designerToolLabel, QLabel*& designerProjectsLabel,
    QLabel*& qaTestTypeLabel, QLabel*& qaBugsLabel) {
    Q_UNUSED(dialog);

    typeCombo = new QComboBox();
    typeCombo->addItems({"Manager", "Developer", "Designer", "QA"});
    typeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Type:", typeCombo);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., John Doe");
    form->addRow("Name:", nameEdit);

    salaryEdit = new QLineEdit();
    salaryEdit->setPlaceholderText("e.g., 5000");
    form->addRow("Salary ($):", salaryEdit);

    deptEdit = new QLineEdit();
    deptEdit->setPlaceholderText("e.g., Development, Design");
    form->addRow("Department:", deptEdit);

    employmentRateCombo = new QComboBox();
    employmentRateCombo->addItem("Full Time (1.0)", 1.0);
    employmentRateCombo->addItem("Three Quarters (0.75)", 0.75);
    employmentRateCombo->addItem("Half Time (0.5)", 0.5);
    employmentRateCombo->addItem("Quarter Time (0.25)", 0.25);
    employmentRateCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Employment Rate:", employmentRateCombo);

    managerProject = new QComboBox();
    managerProject->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(managerProjectLabel, managerProject);
    managerProjectLabel->setVisible(false);
    managerProject->setVisible(false);

    devLanguage = new QLineEdit();
    devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(devLanguageLabel, devLanguage);
    devLanguageLabel->setVisible(false);
    devLanguage->setVisible(false);

    devExperience = new QLineEdit();
    devExperience->setPlaceholderText("e.g., 3");
    devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(devExperienceLabel, devExperience);
    devExperienceLabel->setVisible(false);
    devExperience->setVisible(false);

    designerTool = new QLineEdit();
    designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    designerToolLabel = new QLabel("Design Tool:");
    form->addRow(designerToolLabel, designerTool);
    designerToolLabel->setVisible(false);
    designerTool->setVisible(false);

    designerProjects = new QLineEdit();
    designerProjects->setPlaceholderText("e.g., 10");
    designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(designerProjectsLabel, designerProjects);
    designerProjectsLabel->setVisible(false);
    designerProjects->setVisible(false);

    qaTestType = new QLineEdit();
    qaTestType->setPlaceholderText("e.g., Manual, Automated");
    qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(qaTestTypeLabel, qaTestType);
    qaTestTypeLabel->setVisible(false);
    qaTestType->setVisible(false);

    qaBugs = new QLineEdit();
    qaBugs->setPlaceholderText("e.g., 25");
    qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(qaBugsLabel, qaBugs);
    qaBugsLabel->setVisible(false);
    qaBugs->setVisible(false);

    auto updateFields = [&](int index) {
        showManagerFields(managerProjectLabel, managerProject, index == 0);
        showDeveloperFields(devLanguageLabel, devLanguage, devExperienceLabel,
                            devExperience, index == 1);
        showDesignerFields(designerToolLabel, designerTool,
                           designerProjectsLabel, designerProjects, index == 2);
        showQaFields(qaTestTypeLabel, qaTestType, qaBugsLabel, qaBugs,
                     index == 3);
    };

    QObject::connect(typeCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     updateFields);
    updateFields(0);
}

void EmployeeDialogHelper::createEditEmployeeDialog(
    QDialog& dialog, QFormLayout* form, std::shared_ptr<Employee> employee,
    QLineEdit*& nameEdit, QLineEdit*& salaryEdit, QLineEdit*& deptEdit,
    QComboBox*& employmentRateCombo, QComboBox*& managerProject,
    QLineEdit*& devLanguage, QLineEdit*& devExperience,
    QLineEdit*& designerTool, QLineEdit*& designerProjects,
    QLineEdit*& qaTestType, QLineEdit*& qaBugs, QLabel*& managerProjectLabel,
    QLabel*& devLanguageLabel, QLabel*& devExperienceLabel,
    QLabel*& designerToolLabel, QLabel*& designerProjectsLabel,
    QLabel*& qaTestTypeLabel, QLabel*& qaBugsLabel) {
    Q_UNUSED(dialog);

    auto* typeLabel = new QLabel("Type:");
    auto* typeDisplay = new QLineEdit();
    typeDisplay->setText(employee->getEmployeeType());
    typeDisplay->setReadOnly(true);
    typeDisplay->setStyleSheet("QLineEdit { background-color: #f5f5f5; }");
    form->addRow(typeLabel, typeDisplay);

    nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., John Doe");
    nameEdit->setText(employee->getName());
    form->addRow("Name:", nameEdit);

    salaryEdit = new QLineEdit();
    salaryEdit->setPlaceholderText("e.g., 5000");
    salaryEdit->setText(QString::number(employee->getSalary(), 'f', 2));
    form->addRow("Salary ($):", salaryEdit);

    deptEdit = new QLineEdit();
    deptEdit->setPlaceholderText("e.g., Development, Design");
    deptEdit->setText(employee->getDepartment());
    form->addRow("Department:", deptEdit);

    employmentRateCombo = new QComboBox();
    employmentRateCombo->addItem("Full Time (1.0)", 1.0);
    employmentRateCombo->addItem("Three Quarters (0.75)", 0.75);
    employmentRateCombo->addItem("Half Time (0.5)", 0.5);
    employmentRateCombo->addItem("Quarter Time (0.25)", 0.25);
    employmentRateCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Employment Rate:", employmentRateCombo);

    managerProject = new QComboBox();
    managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(managerProjectLabel, managerProject);

    devLanguage = new QLineEdit();
    devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(devLanguageLabel, devLanguage);

    devExperience = new QLineEdit();
    devExperience->setPlaceholderText("e.g., 3");
    devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(devExperienceLabel, devExperience);

    designerTool = new QLineEdit();
    designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    designerToolLabel = new QLabel("Design Tool:");
    form->addRow(designerToolLabel, designerTool);

    designerProjects = new QLineEdit();
    designerProjects->setPlaceholderText("e.g., 10");
    designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(designerProjectsLabel, designerProjects);

    qaTestType = new QLineEdit();
    qaTestType->setPlaceholderText("e.g., Manual, Automated");
    qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(qaTestTypeLabel, qaTestType);

    qaBugs = new QLineEdit();
    qaBugs->setPlaceholderText("e.g., 25");
    qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(qaBugsLabel, qaBugs);

    populateEmployeeFields(
        employmentRateCombo, managerProject, devLanguage, devExperience,
        designerTool, designerProjects, qaTestType, qaBugs, managerProjectLabel,
        devLanguageLabel, devExperienceLabel, designerToolLabel,
        designerProjectsLabel, qaTestTypeLabel, qaBugsLabel, employee);
}

void EmployeeDialogHelper::populateEmployeeFields(
    QComboBox* employmentRateCombo, QComboBox* managerProject,
    QLineEdit* devLanguage, QLineEdit* devExperience, QLineEdit* designerTool,
    QLineEdit* designerProjects, QLineEdit* qaTestType, QLineEdit* qaBugs,
    QLabel* managerProjectLabel, QLabel* devLanguageLabel,
    QLabel* devExperienceLabel, QLabel* designerToolLabel,
    QLabel* designerProjectsLabel, QLabel* qaTestTypeLabel, QLabel* qaBugsLabel,
    std::shared_ptr<Employee> employee) {
    QString currentType = employee->getEmployeeType();

    double employmentRate = employee->getEmploymentRate();
    int index = employmentRateCombo->findData(employmentRate);
    if (index != -1) {
        employmentRateCombo->setCurrentIndex(index);
    }

    showManagerFields(managerProjectLabel, managerProject, false);
    showDeveloperFields(devLanguageLabel, devLanguage, devExperienceLabel,
                        devExperience, false);
    showDesignerFields(designerToolLabel, designerTool, designerProjectsLabel,
                       designerProjects, false);
    showQaFields(qaTestTypeLabel, qaTestType, qaBugsLabel, qaBugs, false);

    if (currentType == "Manager") {
        showManagerFields(managerProjectLabel, managerProject, true);
    } else if (currentType == "Developer") {
        if (const auto* developer =
                dynamic_cast<const Developer*>(employee.get());
            developer != nullptr) {
            devLanguage->setText(developer->getProgrammingLanguage());
            devExperience->setText(
                QString::number(developer->getYearsOfExperience()));
        }
        showDeveloperFields(devLanguageLabel, devLanguage, devExperienceLabel,
                            devExperience, true);
    } else if (currentType == "Designer") {
        if (const auto* designer =
                dynamic_cast<const Designer*>(employee.get());
            designer != nullptr) {
            designerTool->setText(designer->getDesignTool());
            designerProjects->setText(
                QString::number(designer->getNumberOfProjects()));
        }
        showDesignerFields(designerToolLabel, designerTool,
                           designerProjectsLabel, designerProjects, true);
    } else if (currentType == "QA") {
        if (const auto* qaEmployee = dynamic_cast<const QA*>(employee.get());
            qaEmployee != nullptr) {
            qaTestType->setText(qaEmployee->getTestingType());
            qaBugs->setText(QString::number(qaEmployee->getBugsFound()));
        }
        showQaFields(qaTestTypeLabel, qaTestType, qaBugsLabel, qaBugs, true);
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
    const QString& employeeType, int employeeId, const QString& name,
    double salary, const QString& department, QComboBox* employmentRateCombo,
    QComboBox* managerProject, QLineEdit* devLanguage, QLineEdit* devExperience,
    QLineEdit* designerTool, QLineEdit* designerProjects, QLineEdit* qaTestType,
    QLineEdit* qaBugs) {
    double employmentRate = employmentRateCombo->currentData().toDouble();

    if (employeeType == "Manager") {
        int projectId = managerProject->currentData().toInt();

        return std::make_shared<Manager>(employeeId, name, salary, department,
                                         projectId, employmentRate);
    }

    if (employeeType == "Developer") {
        QString language = devLanguage->text().trimmed();
        if (language.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int years = devExperience->text().toInt(&conversionSuccess);
        if (!conversionSuccess || years < 0 || years > kMaxYearsOfExperience) {
            return nullptr;
        }

        return std::make_shared<Developer>(employeeId, name, salary, department,
                                           language, years, employmentRate);
    }

    if (employeeType == "Designer") {
        QString tool = designerTool->text().trimmed();
        if (tool.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int projects = designerProjects->text().toInt(&conversionSuccess);
        if (!conversionSuccess || projects < 0 ||
            projects > kMaxNumberOfProjects) {
            return nullptr;
        }

        return std::make_shared<Designer>(employeeId, name, salary, department,
                                          tool, projects, employmentRate);
    }

    if (employeeType == "QA") {
        QString testType = qaTestType->text().trimmed();
        if (testType.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int bugs = qaBugs->text().toInt(&conversionSuccess);
        if (!conversionSuccess || bugs < 0 || bugs > kMaxBugsFound) {
            return nullptr;
        }

        return std::make_shared<QA>(employeeId, name, salary, department,
                                    testType, bugs, employmentRate);
    }

    return nullptr;
}
