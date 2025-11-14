#pragma once

#include <QDialog>
#include <QFormLayout>
#include <QString>

class QLineEdit;
class QComboBox;
class Company;

class TaskDialogHelper {
   public:
    static void createAddTaskDialog(QDialog* dialog, QFormLayout* form,
                                    QLineEdit*& taskNameEdit,
                                    QComboBox*& taskTypeCombo,
                                    QLineEdit*& taskEstHoursEdit,
                                    QLineEdit*& priorityEdit);
    static bool validateAndAddTask(const QString& taskName,
                                   const QString& taskType, int taskEst,
                                   int priority, int projectId,
                                   const Company* company, QDialog* dialog);
};
