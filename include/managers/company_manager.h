#pragma once

#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QString>
#include <span>
#include <vector>

#include "entities/company.h"

class CompanyManager {
   public:
    static void initializeCompany(std::span<Company* const> companies,
                                  [[maybe_unused]] Company*& currentCompany,
                                  const int& currentCompanyIndex,
                                  [[maybe_unused]] int& nextEmployeeId,
                                  [[maybe_unused]] int& nextProjectId,
                                  QComboBox* selector);
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
    static void refreshCompanyList(std::span<Company* const> companies,
                                   QComboBox* selector);
};
