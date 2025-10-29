#ifndef DERIVED_EMPLOYEES_H
#define DERIVED_EMPLOYEES_H

#include "Employee.h"

// Bonus calculation constants
namespace EmployeeBonusConstants {
    constexpr double MANAGER_SALARY_MULTIPLIER = 0.25;
    constexpr int MANAGER_TEAM_BONUS = 1000;
    constexpr double DEVELOPER_SALARY_MULTIPLIER = 0.20;
    constexpr int DEVELOPER_EXPERIENCE_BONUS = 500;
    constexpr double DESIGNER_SALARY_MULTIPLIER = 0.15;
    constexpr int DESIGNER_PROJECT_BONUS = 800;
    constexpr double QA_SALARY_MULTIPLIER = 0.10;
    constexpr int QA_BUG_BONUS = 100;
}

// Derived class - Manager
class Manager : public Employee {
private:
    int teamSize;
    QString projectManaged;

public:
    Manager(int id, const QString& name, double salary, const QString& department, 
            int teamSize, const QString& projectManaged);
    
    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    int getTeamSize() const { return teamSize; }
    QString getProjectManaged() const { return projectManaged; }
};

// Derived class - Developer
class Developer : public Employee {
private:
    QString programmingLanguage;
    int yearsOfExperience;

public:
    Developer(int id, const QString& name, double salary, const QString& department,
              const QString& programmingLanguage, int yearsOfExperience);
    
    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getProgrammingLanguage() const { return programmingLanguage; }
    int getYearsOfExperience() const { return yearsOfExperience; }
};

// Derived class - Designer
class Designer : public Employee {
private:
    QString designTool;
    int numberOfProjects;

public:
    Designer(int id, const QString& name, double salary, const QString& department,
             const QString& designTool, int numberOfProjects);
    
    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getDesignTool() const { return designTool; }
    int getNumberOfProjects() const { return numberOfProjects; }
};

// Derived class - QA (Quality Assurance)
class QA : public Employee {
private:
    QString testingType;
    int bugsFound;

public:
    QA(int id, const QString& name, double salary, const QString& department,
       const QString& testingType, int bugsFound);
    
    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getTestingType() const { return testingType; }
    int getBugsFound() const { return bugsFound; }
};

#endif // DERIVED_EMPLOYEES_H


