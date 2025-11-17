#pragma once

#include <QString>
#include <exception>
#include <map>
#include <memory>
#include <vector>

#include "entities/company.h"
#include "exceptions/exceptions.h"

class FileManager {
   public:
    static void saveCompany(const Company& company, const QString& fileName);

    static void saveEmployees(const Company& company, const QString& fileName);

    static void saveProjects(const Company& company, const QString& fileName);

    static void saveTasks(const Company& company, const QString& fileName);

    static void saveTaskAssignments(const Company& company,
                                    const QString& fileName);

    static Company loadCompany(const QString& fileName);

    static void loadEmployees(Company& company, const QString& fileName);

    static void loadProjects(Company& company, const QString& fileName);

    static void loadTasks(Company& company, const QString& fileName);

    static void loadTaskAssignments(Company& company, const QString& fileName);

    static Company loadFromFile(const QString& fileName);

    static std::map<int, bool> employeeStatusesFromFile;

   private:
    static int parseIntFromStream(std::ifstream& fileStream,
                                  const QString& fieldName);
    static double parseDoubleFromStream(std::ifstream& fileStream,
                                        const QString& fieldName);
    static QString parseStringFromStream(std::ifstream& fileStream);
    static double parseEmploymentRate(std::ifstream& fileStream);

    static void saveEmployeeBaseData(std::shared_ptr<Employee> employee,
                                     std::ofstream& fileStream);
    static void saveEmployeeTypeSpecificData(std::shared_ptr<Employee> employee,
                                             std::ofstream& fileStream);

    struct EmployeeBaseData {
        int id;
        QString name;
        double salary;
        QString department;
        double employmentRate;
        bool isActive{true};
    };
    static EmployeeBaseData loadEmployeeBaseData(std::ifstream& fileStream);

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
