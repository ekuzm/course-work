#pragma once

#include <QDialog>
#include <QString>

class ValidationHelper {
   public:
    static bool validateNonEmpty(const QString& value, const QString& fieldName,
                                 QDialog* dialog);
    static bool validateDouble(const QString& text, double& result, double min,
                               double max, const QString& fieldName,
                               QDialog* dialog);
    static bool validateInt(const QString& text, int& result, int min, int max,
                            const QString& fieldName, QDialog* dialog);
    static bool validateDateRange(const QDate& startDate, const QDate& endDate,
                                  QDialog* dialog);
};
