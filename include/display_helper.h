#pragma once

#include <QTableWidget>
#include <QTextEdit>
#include <memory>
#include <vector>

#include "company.h"

class MainWindow;

class DisplayHelper {
   public:
    static void displayEmployees(QTableWidget* employeeTable,
                                 const Company* currentCompany,
                                 MainWindow* mainWindow);
    static void displayProjects(QTableWidget* projectTable,
                                const Company* currentCompany,
                                MainWindow* mainWindow);
    static void showCompanyInfo(QTextEdit* companyInfoText,
                                const Company* currentCompany);
    static void showStatistics(QTextEdit* statisticsText,
                               const Company* currentCompany);

    static QString formatProjectInfo(
        const std::shared_ptr<const Employee>& employee,
        const Company* currentCompany);

    static QString formatTaskInfo(
        const std::shared_ptr<const Employee>& employee,
        const Company* currentCompany);
    static QString formatEmploymentRate(double rate);
};
