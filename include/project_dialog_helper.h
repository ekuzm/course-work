#pragma once

#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>

class Project;

class ProjectDialogHelper {
   public:
    struct ProjectDialogFields {
        QLineEdit* nameEdit = nullptr;
        QTextEdit* descEdit = nullptr;
        QComboBox* statusCombo = nullptr;
        QDateEdit* startDate = nullptr;
        QDateEdit* endDate = nullptr;
        QLineEdit* budgetEdit = nullptr;
        QLineEdit* estimatedHoursEdit = nullptr;
        QLineEdit* clientNameEdit = nullptr;
        QLabel* clientNameLabel = nullptr;
        QLineEdit* clientIndustryEdit = nullptr;
        QLabel* clientIndustryLabel = nullptr;
        QLineEdit* clientContactEdit = nullptr;
        QLabel* clientContactLabel = nullptr;
        QComboBox* projectTypeCombo = nullptr;
    };

    static void createProjectDialogFields(QDialog& dialog, QFormLayout* form, ProjectDialogFields& fields);
    static void populateProjectDialogFields(const Project* project, ProjectDialogFields& fields);
    static void setupClientFieldsVisibility(ProjectDialogFields& fields);
};


