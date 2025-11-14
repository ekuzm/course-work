#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <vector>

#include "entities/company.h"

class CompanyManager {
   public:
    static void initializeCompany(std::vector<Company*>& companies,
                                  Company*& currentCompany,
                                  int& currentCompanyIndex, int& nextEmployeeId,
                                  int& nextProjectId, QComboBox* selector);
    static void addCompany(std::vector<Company*>& companies,
                           Company*& currentCompany, int& currentCompanyIndex,
                           QComboBox* selector, QWidget* parent);
    static void switchCompany(std::vector<Company*>& companies,
                              Company*& currentCompany,
                              int& currentCompanyIndex, QComboBox* selector,
                              int newIndex);
    static void deleteCompany(std::vector<Company*>& companies,
                              Company*& currentCompany,
                              int& currentCompanyIndex, QComboBox* selector,
                              QWidget* parent);
    static void refreshCompanyList(const std::vector<Company*>& companies,
                                   QComboBox* selector);
};
