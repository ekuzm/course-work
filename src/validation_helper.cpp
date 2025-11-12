#include "validation_helper.h"

#include <QDate>
#include <QDateEdit>
#include <QMessageBox>

bool ValidationHelper::validateNonEmpty(const QString& value,
                                        const QString& fieldName,
                                        QDialog* dialog) {
    if (value.isEmpty()) {
        QMessageBox::warning(
            dialog, "Validation Error",
            QString("%1 cannot be empty!\n\nPlease enter a valid %2.")
                .arg(fieldName, fieldName.toLower()));
        return false;
    }
    return true;
}

bool ValidationHelper::validateDouble(const QString& text, double& result,
                                      double min, double max,
                                      const QString& fieldName,
                                      QDialog* dialog) {
    bool conversionSuccess = false;
    double value = text.toDouble(&conversionSuccess);

    if (!conversionSuccess) {
        QMessageBox::warning(
            dialog, "Validation Error",
            QString("Invalid %1 format!\n\nPlease enter a valid number.\n"
                    "Current value: \"%2\"\nValid range: $%3 to $%4")
                .arg(fieldName.toLower(), text, QString::number(min, 'f', 0),
                     QString::number(max, 'f', 0)));
        return false;
    }

    if (value < min || value > max) {
        QMessageBox::warning(dialog, "Validation Error",
                             QString("%1 out of valid range!\n\nCurrent value: "
                                     "$%2\nValid range: $%3 to $%4")
                                 .arg(fieldName, QString::number(value, 'f', 2),
                                      QString::number(min, 'f', 0),
                                      QString::number(max, 'f', 0)));
        return false;
    }

    result = value;
    return true;
}

bool ValidationHelper::validateInt(const QString& text, int& result, int min,
                                   int max, const QString& fieldName,
                                   QDialog* dialog) {
    bool conversionSuccess = false;
    int value = text.toInt(&conversionSuccess);

    if (!conversionSuccess || text.isEmpty()) {
        QMessageBox::warning(
            dialog, "Validation Error",
            QString("Invalid %1 format!\n\nPlease enter a valid number.\n"
                    "Current value: \"%2\"\nValid range: %3 to %4")
                .arg(fieldName.toLower(), text, QString::number(min),
                     QString::number(max)));
        return false;
    }

    if (value < min || value > max) {
        QString errorMessage;
        if (max < min) {
            errorMessage =
                QString(
                    "%1 cannot be assigned!\n\n"
                    "Current value: %2\n"
                    "Maximum allowed: %3\n\n"
                    "The employee does not have enough available hours.")
                    .arg(fieldName, QString::number(value),
                         QString::number(max));
        } else {
            errorMessage = QString(
                               "%1 out of valid range!\n\n"
                               "Current value: %2\n"
                               "Valid range: %3 to %4")
                               .arg(fieldName, QString::number(value),
                                    QString::number(min), QString::number(max));
        }
        QMessageBox::warning(dialog, "Validation Error", errorMessage);
        return false;
    }

    result = value;
    return true;
}

bool ValidationHelper::validateDateRange(const QDate& startDate,
                                         const QDate& endDate,
                                         QDialog* dialog) {
    if (endDate < startDate) {
        QMessageBox::warning(dialog, "Validation Error",
                             QString("End date cannot be before start date!\n\n"
                                     "Start date: %1\n"
                                     "End date: %2\n\n"
                                     "Please adjust the dates.")
                                 .arg(startDate.toString("yyyy-MM-dd"),
                                      endDate.toString("yyyy-MM-dd")));
        return false;
    }
    return true;
}
