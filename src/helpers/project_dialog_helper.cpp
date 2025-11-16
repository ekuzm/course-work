#include "helpers/project_dialog_helper.h"

#include <QDate>
#include <QDateEdit>
#include <QStandardItemModel>

#include "utils/consts.h"
#include "entities/project.h"

void ProjectDialogHelper::createProjectDialogFields(
    QDialog& dialog, QFormLayout* form, ProjectDialogFields& fields) {
    fields.projectTypeCombo = new QComboBox();
    fields.projectTypeCombo->addItems({"Web Development", "Mobile App",
                                       "Software Product", "Consulting",
                                       "Other"});
    fields.projectTypeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Project Type:", fields.projectTypeCombo);

    fields.nameEdit = new QLineEdit();
    fields.nameEdit->setPlaceholderText("e.g., E-commerce Platform");
    form->addRow("Project Name:", fields.nameEdit);

    fields.descEdit = new QTextEdit();
    fields.descEdit->setMaximumHeight(kDescEditMaxHeight);
    fields.descEdit->setPlaceholderText("Brief description of the project...");
    form->addRow("Description:", fields.descEdit);

    fields.phaseCombo = new QComboBox();
    fields.phaseCombo->addItems({"Analysis", "Planning", "Design",
                                 "Development", "Testing", "Deployment",
                                 "Maintenance", "Completed"});
    fields.phaseCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Phase:", fields.phaseCombo);

    fields.startDate = new QDateEdit();
    fields.startDate->setDate(QDate::currentDate());
    fields.startDate->setCalendarPopup(true);
    fields.startDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("Start Date:", fields.startDate);

    fields.endDate = new QDateEdit();
    fields.endDate->setDate(
        QDate::currentDate().addDays(kDefaultProjectEndDateDays));
    fields.endDate->setCalendarPopup(true);
    fields.endDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("End Date:", fields.endDate);

    fields.budgetEdit = new QLineEdit();
    fields.budgetEdit->setPlaceholderText("e.g., 50000");
    form->addRow("Budget ($):", fields.budgetEdit);

    fields.estimatedHoursEdit = new QLineEdit();
    fields.estimatedHoursEdit->setPlaceholderText("e.g., 160 (hours)");
    form->addRow("Estimated Hours:", fields.estimatedHoursEdit);

    fields.clientNameEdit = new QLineEdit();
    fields.clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
    fields.clientNameLabel = new QLabel("Client Name:");
    form->addRow(fields.clientNameLabel, fields.clientNameEdit);

    fields.clientIndustryEdit = new QLineEdit();
    fields.clientIndustryEdit->setPlaceholderText("e.g., Retail, Finance");
    fields.clientIndustryLabel = new QLabel("Client Industry:");
    form->addRow(fields.clientIndustryLabel, fields.clientIndustryEdit);
    fields.clientIndustryLabel->setVisible(false);
    fields.clientIndustryEdit->setVisible(false);

    fields.clientContactEdit = new QLineEdit();
    fields.clientContactEdit->setPlaceholderText("e.g., john@company.com");
    fields.clientContactLabel = new QLabel("Client Contact:");
    form->addRow(fields.clientContactLabel, fields.clientContactEdit);
    fields.clientContactLabel->setVisible(false);
    fields.clientContactEdit->setVisible(false);

    QObject::connect(fields.projectTypeCombo,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     [&fields](int index) {
                         constexpr int CONSULTING_INDEX = 3;
                         constexpr int OTHER_INDEX = 4;
                         bool showDetails = (index == CONSULTING_INDEX ||
                                             index == OTHER_INDEX);
                         fields.clientIndustryLabel->setVisible(showDetails);
                         fields.clientIndustryEdit->setVisible(showDetails);
                         fields.clientContactLabel->setVisible(showDetails);
                         fields.clientContactEdit->setVisible(showDetails);
                     });
}

void ProjectDialogHelper::populateProjectDialogFields(
    const Project* project, ProjectDialogFields& fields) {
    if (!project) return;

    fields.nameEdit->setText(project->getName());
    fields.descEdit->setPlainText(project->getDescription());

    QString currentPhase = project->getPhase();
    int currentPhaseOrder = Project::getPhaseOrder(currentPhase);

    if (currentPhaseOrder < 0) {
        return;
    }
    
    QStandardItemModel* model =
        qobject_cast<QStandardItemModel*>(fields.phaseCombo->model());
    if (!model) {
        return;
    }
    
    for (int i = 0; i < fields.phaseCombo->count(); ++i) {
        QString phaseText = fields.phaseCombo->itemText(i);
        int phaseOrder = Project::getPhaseOrder(phaseText);
        QStandardItem* item = model->item(i);
        if (!item) {
            continue;
        }
        if (phaseOrder >= 0 && phaseOrder < currentPhaseOrder) {
            item->setEnabled(false);
        } else {
            item->setEnabled(true);
        }
    }

    int phaseIndex = fields.phaseCombo->findText(currentPhase);
    if (phaseIndex >= 0) {
        fields.phaseCombo->setCurrentIndex(phaseIndex);
    }

    fields.startDate->setDate(project->getStartDate());
    fields.endDate->setDate(project->getEndDate());
    fields.budgetEdit->setText(QString::number(project->getBudget(), 'f', 2));
    fields.estimatedHoursEdit->setText(
        QString::number(project->getEstimatedHours()));
    fields.clientNameEdit->setText(project->getClientName());
}

void ProjectDialogHelper::setupClientFieldsVisibility(
    ProjectDialogFields& fields) {
    fields.clientIndustryLabel->setVisible(false);
    fields.clientIndustryEdit->setVisible(false);
    fields.clientContactLabel->setVisible(false);
    fields.clientContactEdit->setVisible(false);
}
