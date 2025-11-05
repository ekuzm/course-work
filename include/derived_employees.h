#pragma once

#include "consts.h"
#include "employee.h"

class Manager : public Employee {
   private:
    int managedProjectId;

   public:
    Manager(int employeeId, QString employeeName, double employeeSalary,
            QString employeeDepartment, int managedProjectId = -1,
            double employmentRate = 1.0);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    int getManagedProjectId() const;
    void setManagedProjectId(int projectId);
};

class Developer : public Employee {
   private:
    QString programmingLanguage;
    int yearsOfExperience;

   public:
    Developer(int employeeId, QString employeeName, double employeeSalary,
              QString employeeDepartment, QString developerProgrammingLanguage,
              int developerYearsOfExperience, double employmentRate = 1.0);

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
             int designerNumberOfProjects, double employmentRate = 1.0);

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
       QString employeeDepartment, QString qaTestingType, int qaBugsFound,
       double employmentRate = 1.0);

    QString getEmployeeType() const override;
    QString getDetails() const override;
    double calculateBonus() const override;

    QString getTestingType() const;
    int getBugsFound() const;
};
