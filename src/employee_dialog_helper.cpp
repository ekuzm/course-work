#include "../include/employee_dialog_helper.h"

#include <QMessageBox>
#include <algorithm>
#include <ranges>

#include "../include/consts.h"
#include "../include/derived_employees.h"

void EmployeeDialogHelper::showManagerFields(const EmployeeFormWidgets& widgets,
                                              bool show) {
    widgets.managerProjectLabel->setVisible(show);
    widgets.managerProject->setVisible(show);
    widgets.managerTeamSizeLabel->setVisible(show);
    widgets.managerTeamSize->setVisible(show);
}

void EmployeeDialogHelper::showDeveloperFields(const EmployeeFormWidgets& widgets,
                                                bool show) {
    widgets.devLanguageLabel->setVisible(show);
    widgets.devLanguage->setVisible(show);
    widgets.devExperienceLabel->setVisible(show);
    widgets.devExperience->setVisible(show);
}

void EmployeeDialogHelper::showDesignerFields(const EmployeeFormWidgets& widgets,
                                              bool show) {
    widgets.designerToolLabel->setVisible(show);
    widgets.designerTool->setVisible(show);
    widgets.designerProjectsLabel->setVisible(show);
    widgets.designerProjects->setVisible(show);
}

void EmployeeDialogHelper::showQaFields(const EmployeeFormWidgets& widgets,
                                         bool show) {
    widgets.qaTestTypeLabel->setVisible(show);
    widgets.qaTestType->setVisible(show);
    widgets.qaBugsLabel->setVisible(show);
    widgets.qaBugs->setVisible(show);
}

EmployeeDialogHelper::EmployeeFormWidgets
EmployeeDialogHelper::createEmployeeDialog(QDialog& /* dialog */, QFormLayout* form) {
    EmployeeFormWidgets widgets;

    widgets.typeCombo = new QComboBox();
    widgets.typeCombo->addItems({"Manager", "Developer", "Designer", "QA"});
    widgets.typeCombo->setStyleSheet("background-color: white;");
    form->addRow("Type:", widgets.typeCombo);

    widgets.nameEdit = new QLineEdit();
    widgets.nameEdit->setPlaceholderText("e.g., John Doe");
    form->addRow("Name:", widgets.nameEdit);

    widgets.salaryEdit = new QLineEdit();
    widgets.salaryEdit->setPlaceholderText("e.g., 5000");
    form->addRow("Salary ($):", widgets.salaryEdit);

    widgets.deptEdit = new QLineEdit();
    widgets.deptEdit->setPlaceholderText("e.g., Development, Design");
    form->addRow("Department:", widgets.deptEdit);

    // Manager fields
    widgets.managerProject = new QLineEdit();
    widgets.managerProject->setPlaceholderText("e.g., Mobile App Project");
    widgets.managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(widgets.managerProjectLabel, widgets.managerProject);
    widgets.managerProjectLabel->setVisible(false);
    widgets.managerProject->setVisible(false);

    widgets.managerTeamSize = new QLineEdit();
    widgets.managerTeamSize->setPlaceholderText("e.g., 5");
    widgets.managerTeamSizeLabel = new QLabel("Team Size:");
    form->addRow(widgets.managerTeamSizeLabel, widgets.managerTeamSize);
    widgets.managerTeamSizeLabel->setVisible(false);
    widgets.managerTeamSize->setVisible(false);

    // Developer fields
    widgets.devLanguage = new QLineEdit();
    widgets.devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    widgets.devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(widgets.devLanguageLabel, widgets.devLanguage);
    widgets.devLanguageLabel->setVisible(false);
    widgets.devLanguage->setVisible(false);

    widgets.devExperience = new QLineEdit();
    widgets.devExperience->setPlaceholderText("e.g., 3");
    widgets.devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(widgets.devExperienceLabel, widgets.devExperience);
    widgets.devExperienceLabel->setVisible(false);
    widgets.devExperience->setVisible(false);

    // Designer fields
    widgets.designerTool = new QLineEdit();
    widgets.designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    widgets.designerToolLabel = new QLabel("Design Tool:");
    form->addRow(widgets.designerToolLabel, widgets.designerTool);
    widgets.designerToolLabel->setVisible(false);
    widgets.designerTool->setVisible(false);

    widgets.designerProjects = new QLineEdit();
    widgets.designerProjects->setPlaceholderText("e.g., 10");
    widgets.designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(widgets.designerProjectsLabel, widgets.designerProjects);
    widgets.designerProjectsLabel->setVisible(false);
    widgets.designerProjects->setVisible(false);

    // QA fields
    widgets.qaTestType = new QLineEdit();
    widgets.qaTestType->setPlaceholderText("e.g., Manual, Automated");
    widgets.qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(widgets.qaTestTypeLabel, widgets.qaTestType);
    widgets.qaTestTypeLabel->setVisible(false);
    widgets.qaTestType->setVisible(false);

    widgets.qaBugs = new QLineEdit();
    widgets.qaBugs->setPlaceholderText("e.g., 25");
    widgets.qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(widgets.qaBugsLabel, widgets.qaBugs);
    widgets.qaBugsLabel->setVisible(false);
    widgets.qaBugs->setVisible(false);

    // Setup field visibility handler
    auto updateFields = [widgets](int index) {
        showManagerFields(widgets, index == 0);
        showDeveloperFields(widgets, index == 1);
        showDesignerFields(widgets, index == 2);
        showQaFields(widgets, index == 3);
    };

    QObject::connect(widgets.typeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged), updateFields);
    updateFields(0);

    return widgets;
}

EmployeeDialogHelper::EmployeeFormWidgets
EmployeeDialogHelper::createEditEmployeeDialog(
    QDialog& /* dialog */, QFormLayout* form, std::shared_ptr<Employee> employee) {
    EmployeeFormWidgets widgets;

    // Type is read-only
    auto* typeLabel = new QLabel("Type:");
    auto* typeDisplay = new QLineEdit();
    typeDisplay->setText(employee->getEmployeeType());
    typeDisplay->setReadOnly(true);
    typeDisplay->setStyleSheet("QLineEdit { background-color: #f5f5f5; }");
    form->addRow(typeLabel, typeDisplay);

    widgets.nameEdit = new QLineEdit();
    widgets.nameEdit->setPlaceholderText("e.g., John Doe");
    widgets.nameEdit->setText(employee->getName());
    form->addRow("Name:", widgets.nameEdit);

    widgets.salaryEdit = new QLineEdit();
    widgets.salaryEdit->setPlaceholderText("e.g., 5000");
    widgets.salaryEdit->setText(QString::number(employee->getSalary(), 'f', 2));
    form->addRow("Salary ($):", widgets.salaryEdit);

    widgets.deptEdit = new QLineEdit();
    widgets.deptEdit->setPlaceholderText("e.g., Development, Design");
    widgets.deptEdit->setText(employee->getDepartment());
    form->addRow("Department:", widgets.deptEdit);

    // Create all type-specific fields
    widgets.managerProject = new QLineEdit();
    widgets.managerProject->setPlaceholderText("e.g., Mobile App Project");
    widgets.managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(widgets.managerProjectLabel, widgets.managerProject);

    widgets.managerTeamSize = new QLineEdit();
    widgets.managerTeamSize->setPlaceholderText("e.g., 5");
    widgets.managerTeamSizeLabel = new QLabel("Team Size:");
    form->addRow(widgets.managerTeamSizeLabel, widgets.managerTeamSize);

    widgets.devLanguage = new QLineEdit();
    widgets.devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    widgets.devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(widgets.devLanguageLabel, widgets.devLanguage);

    widgets.devExperience = new QLineEdit();
    widgets.devExperience->setPlaceholderText("e.g., 3");
    widgets.devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(widgets.devExperienceLabel, widgets.devExperience);

    widgets.designerTool = new QLineEdit();
    widgets.designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    widgets.designerToolLabel = new QLabel("Design Tool:");
    form->addRow(widgets.designerToolLabel, widgets.designerTool);

    widgets.designerProjects = new QLineEdit();
    widgets.designerProjects->setPlaceholderText("e.g., 10");
    widgets.designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(widgets.designerProjectsLabel, widgets.designerProjects);

    widgets.qaTestType = new QLineEdit();
    widgets.qaTestType->setPlaceholderText("e.g., Manual, Automated");
    widgets.qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(widgets.qaTestTypeLabel, widgets.qaTestType);

    widgets.qaBugs = new QLineEdit();
    widgets.qaBugs->setPlaceholderText("e.g., 25");
    widgets.qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(widgets.qaBugsLabel, widgets.qaBugs);

    populateEmployeeFields(widgets, employee);

    return widgets;
}

void EmployeeDialogHelper::populateEmployeeFields(
    const EmployeeFormWidgets& widgets, std::shared_ptr<Employee> employee) {
    QString currentType = employee->getEmployeeType();

    // Hide all fields first
    showManagerFields(widgets, false);
    showDeveloperFields(widgets, false);
    showDesignerFields(widgets, false);
    showQaFields(widgets, false);

    // Show and populate fields based on type
    if (currentType == "Manager") {
        if (const auto* manager = dynamic_cast<const Manager*>(employee.get());
            manager != nullptr) {
            widgets.managerProject->setText(manager->getProjectManaged());
            widgets.managerTeamSize->setText(QString::number(manager->getTeamSize()));
        }
        showManagerFields(widgets, true);
    } else if (currentType == "Developer") {
        if (const auto* developer = dynamic_cast<const Developer*>(employee.get());
            developer != nullptr) {
            widgets.devLanguage->setText(developer->getProgrammingLanguage());
            widgets.devExperience->setText(
                QString::number(developer->getYearsOfExperience()));
        }
        showDeveloperFields(widgets, true);
    } else if (currentType == "Designer") {
        if (const auto* designer = dynamic_cast<const Designer*>(employee.get());
            designer != nullptr) {
            widgets.designerTool->setText(designer->getDesignTool());
            widgets.designerProjects->setText(
                QString::number(designer->getNumberOfProjects()));
        }
        showDesignerFields(widgets, true);
    } else if (currentType == "QA") {
        if (const auto* qaEmployee = dynamic_cast<const QA*>(employee.get());
            qaEmployee != nullptr) {
            widgets.qaTestType->setText(qaEmployee->getTestingType());
            widgets.qaBugs->setText(QString::number(qaEmployee->getBugsFound()));
        }
        showQaFields(widgets, true);
    }
}

bool EmployeeDialogHelper::validateEmployeeInput(const QString& name,
                                                  double salary,
                                                  const QString& department) {
    if (name.isEmpty()) {
        return false;
    }

    if (salary < kMinSalary || salary > kMaxSalary) {
        return false;
    }

    if (department.isEmpty()) {
        return false;
    }

    return true;
}

bool EmployeeDialogHelper::checkDuplicateEmployee(const QString& name,
                                                   const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingEmployees = currentCompany->getAllEmployees();
    auto duplicateFound = std::ranges::any_of(
        existingEmployees,
        [&name](const auto& employee) {
            return employee != nullptr &&
                   employee->getName().toLower() == name.toLower();
        });
    return !duplicateFound;
}

bool EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(const QString& name,
                                                         int excludeId,
                                                         const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingEmployees = currentCompany->getAllEmployees();
    auto duplicateFound = std::ranges::any_of(
        existingEmployees,
        [&name, excludeId](const auto& employee) {
            return employee != nullptr && employee->getId() != excludeId &&
                   employee->getName().toLower() == name.toLower();
        });
    return !duplicateFound;
}

std::shared_ptr<Employee> EmployeeDialogHelper::createEmployeeFromType(
    const QString& employeeType, int employeeId, const QString& name,
    double salary, const QString& department,
    const EmployeeFormWidgets& widgets) {
    if (employeeType == "Manager") {
        QString projectName = widgets.managerProject->text().trimmed();
        if (projectName.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int teamSize = widgets.managerTeamSize->text().toInt(&conversionSuccess);
        if (!conversionSuccess || teamSize < 0) {
            return nullptr;
        }

        return std::make_shared<Manager>(employeeId, name, salary, department,
                                         teamSize, projectName);
    }

    if (employeeType == "Developer") {
        QString language = widgets.devLanguage->text().trimmed();
        if (language.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int years = widgets.devExperience->text().toInt(&conversionSuccess);
        if (!conversionSuccess || years < 0) {
            return nullptr;
        }

        return std::make_shared<Developer>(employeeId, name, salary, department,
                                           language, years);
    }

    if (employeeType == "Designer") {
        QString tool = widgets.designerTool->text().trimmed();
        if (tool.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int projects = widgets.designerProjects->text().toInt(&conversionSuccess);
        if (!conversionSuccess || projects < 0) {
            return nullptr;
        }

        return std::make_shared<Designer>(employeeId, name, salary, department,
                                          tool, projects);
    }

    if (employeeType == "QA") {
        QString testType = widgets.qaTestType->text().trimmed();
        if (testType.isEmpty()) {
            return nullptr;
        }

        bool conversionSuccess = false;
        int bugs = widgets.qaBugs->text().toInt(&conversionSuccess);
        if (!conversionSuccess || bugs < 0) {
            return nullptr;
        }

        return std::make_shared<QA>(employeeId, name, salary, department,
                                    testType, bugs);
    }

    return nullptr;
}

