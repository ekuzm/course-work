#ifndef DISPLAY_HELPER_H
#define DISPLAY_HELPER_H

#include <QTableWidget>
#include <QTextEdit>
#include <memory>
#include <vector>

#include "company.h"

class DisplayHelper {
   public:
    static void displayEmployees(QTableWidget* employeeTable,
                                 const Company* currentCompany);
    static void displayProjects(QTableWidget* projectTable,
                                const Company* currentCompany);
    static void showCompanyInfo(QTextEdit* companyInfoText,
                                const Company* currentCompany);
    static void showStatistics(QTextEdit* statisticsText,
                               const Company* currentCompany);
};

#endif  // DISPLAY_HELPER_H

