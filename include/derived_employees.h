#ifndef DERIVED_EMPLOYEES_H
#define DERIVED_EMPLOYEES_H

#include "employee.h"
#include "consts.h"

class Manager : public Employee {
   private:
    int teamSize;
    QString projectManaged;

   public:
    Manager(int employeeId, QString employeeName, double employeeSalary,
            QString employeeDepartment, int managerTeamSize,
            QString managedProject);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    int getTeamSize() const;
    QString getProjectManaged() const;
};

class Developer : public Employee {
   private:
    QString programmingLanguage;
    int yearsOfExperience;

   public:
    Developer(int employeeId, QString employeeName, double employeeSalary,
              QString employeeDepartment, QString developerProgrammingLanguage,
              int developerYearsOfExperience);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getProgrammingLanguage() const;
    int getYearsOfExperience() const;
};

class Designer : public Employee {
   private:
    QString designTool;
    int numberOfProjects;

   public:
    Designer(int employeeId, QString employeeName, double employeeSalary,
             QString employeeDepartment, QString designerTool,
             int designerNumberOfProjects);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getDesignTool() const;
    int getNumberOfProjects() const;
};

class QA : public Employee {
   private:
    QString testingType;
    int bugsFound;

   public:
    QA(int employeeId, QString employeeName, double employeeSalary,
       QString employeeDepartment, QString qaTestingType, int qaBugsFound);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getTestingType() const;
    int getBugsFound() const;
};

#endif  // DERIVED_EMPLOYEES_H
