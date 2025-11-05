#pragma once

#include <QString>
#include <exception>
#include <memory>
#include <vector>

#include "company.h"
#include "exceptions.h"

class FileManager {
   public:
    static void saveCompany(const Company& company, const QString& fileName);

    static void saveEmployees(const Company& company, const QString& fileName);

    static void saveProjects(const Company& company, const QString& fileName);

    static Company loadCompany(const QString& fileName);

    static void loadEmployees(Company& company, const QString& fileName);

    static void loadProjects(Company& company, const QString& fileName);

    static Company loadFromFile(const QString& fileName);

   private:
    static void saveEmployeeToStream(std::shared_ptr<Employee> employee,
                                     std::ofstream& file);
    static std::shared_ptr<Employee> loadEmployeeFromStream(
        std::ifstream& file);

    static void saveProjectToStream(const Project& project,
                                    std::ofstream& file);
    static Project loadProjectFromStream(std::ifstream& file);

    static void saveEmployeesToStream(const Company& company,
                                      std::ofstream& file);
    static void saveProjectsToStream(const Company& company,
                                     std::ofstream& file);
    static void loadEmployeesFromStream(Company& company, std::ifstream& file);
    static void loadProjectsFromStream(Company& company, std::ifstream& file);
    static void saveSingleCompany(const Company& company, std::ofstream& file);
    static Company loadSingleCompany(std::ifstream& file);
};
