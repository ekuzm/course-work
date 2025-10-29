#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>
#include <memory>
#include <vector>

#include "company.h"

// File manager for saving/loading company data
class FileManager {
   public:
    static void saveToFile(Company& company, const QString& fileName);
    static Company loadFromFile(const QString& fileName);

    // Multi-company support
    static void saveCompanies(const std::vector<Company*>& companies,
                              const QString& fileName);
    static std::vector<Company*> loadCompanies(const QString& fileName);

   private:
    static void saveEmployees(Company& company, std::ofstream& file);
    static void saveProjects(Company& company, std::ofstream& file);
    static void loadEmployees(Company& company, std::ifstream& file);
    static void loadProjects(Company& company, std::ifstream& file);

    static void saveSingleCompany(Company& company, std::ofstream& file);
    static Company loadSingleCompany(std::ifstream& file);
};

#endif  // FILEMANAGER_H
