#ifndef COMPANY_MANAGER_H
#define COMPANY_MANAGER_H

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <vector>

#include "company.h"

class CompanyManager {
   public:
    struct CompanyData {
        std::vector<Company*> companies;
        Company* currentCompany = nullptr;
        int currentCompanyIndex = -1;
        int nextEmployeeId = 1;
        int nextProjectId = 1;
    };

    static void initializeCompany(CompanyData& companyData,
                                   QComboBox* selector);
    static void addCompany(CompanyData& companyData, QComboBox* selector,
                           QWidget* parent);
    static void switchCompany(CompanyData& companyData, QComboBox* selector,
                              int newIndex);
    static void deleteCompany(CompanyData& companyData, QComboBox* selector,
                              QWidget* parent);
    static void refreshCompanyList(CompanyData& companyData,
                                    QComboBox* selector);
};

#endif  // COMPANY_MANAGER_H

