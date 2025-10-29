#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "Company.h"
#include <QString>
#include <memory>
#include <vector>

// File manager for saving/loading company data
class FileManager {
public:
    static void saveToFile(Company& company, const QString& filename);
    static Company loadFromFile(const QString& filename);
    
    // Multi-company support
    static void saveCompanies(const std::vector<Company*>& companies, const QString& filename);
    static std::vector<Company*> loadCompanies(const QString& filename);
    
private:
    static void saveEmployees(Company& company, std::ofstream& file);
    static void saveProjects(Company& company, std::ofstream& file);
    static void loadEmployees(Company& company, std::ifstream& file);
    static void loadProjects(Company& company, std::ifstream& file);
    
    static void saveSingleCompany(Company& company, std::ofstream& file);
    static Company loadSingleCompany(std::ifstream& file);
};

#endif // FILEMANAGER_H


