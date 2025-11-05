#pragma once

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

    static QString formatProjectInfo(
        const std::shared_ptr<const Employee>& employee,
        const Company* currentCompany);
};
