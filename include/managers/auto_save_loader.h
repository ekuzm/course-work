#pragma once

#include <QString>
#include <vector>

class Company;

class AutoSaveLoader {
   public:
    static QString getDataDirectory();
    static void autoSave(const std::vector<Company*>& companies,
                         class MainWindow* mainWindow);
    static void autoLoad(std::vector<Company*>& companies,
                         Company*& currentCompany, int& currentCompanyIndex,
                         class MainWindow* mainWindow);
    static void clearDataFiles(const QString& dataDirPath);
};
