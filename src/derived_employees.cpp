#include "derived_employees.h"

#include <utility>

Manager::Manager(int employeeId, QString employeeName, double employeeSalary,
                 QString employeeDepartment, int managedProjectId,
                 double employmentRate)
    : Employee(employeeId, std::move(employeeName), "Manager", employeeSalary,
               std::move(employeeDepartment), employmentRate),
      managedProjectId(managedProjectId) {}

QString Manager::getEmployeeType() const { return "Manager"; }

QString Manager::getDetails() const {
    return Employee::getDetails() +
           QString(", Managed Project ID: %1").arg(managedProjectId);
}

double Manager::calculateBonus() const {
    return getSalary() * kManagerSalaryMultiplier;
}

int Manager::getManagedProjectId() const { return managedProjectId; }

void Manager::setManagedProjectId(int projectId) {
    managedProjectId = projectId;
}

Developer::Developer(int employeeId, QString employeeName,
                     double employeeSalary, QString employeeDepartment,
                     QString developerProgrammingLanguage,
                     double developerYearsOfExperience, double employmentRate)
    : Employee(employeeId, std::move(employeeName), "Developer", employeeSalary,
               std::move(employeeDepartment), employmentRate),
      programmingLanguage(std::move(developerProgrammingLanguage)),
      yearsOfExperience(developerYearsOfExperience) {
    if (developerYearsOfExperience < 0.0 || developerYearsOfExperience > 50.0) {
        throw EmployeeException(
            "Years of experience must be between 0.0 and 50.0");
    }
}

QString Developer::getEmployeeType() const { return "Developer"; }

QString Developer::getDetails() const {
    return Employee::getDetails() +
           QString(", Language: %1, Experience: %2 years")
               .arg(programmingLanguage)
               .arg(yearsOfExperience, 0, 'f', 1);
}

double Developer::calculateBonus() const {
    return (getSalary() * kDeveloperSalaryMultiplier) +
           (yearsOfExperience * kDeveloperExperienceBonus);
}

Designer::Designer(int employeeId, QString employeeName, double employeeSalary,
                   QString employeeDepartment, QString designerTool,
                   int designerNumberOfProjects, double employmentRate)
    : Employee(employeeId, std::move(employeeName), "Designer", employeeSalary,
               std::move(employeeDepartment), employmentRate),
      designTool(std::move(designerTool)),
      numberOfProjects(designerNumberOfProjects) {
    if (designerNumberOfProjects < 0) {
        throw EmployeeException("Number of projects cannot be negative");
    }
}

QString Designer::getEmployeeType() const { return "Designer"; }

QString Designer::getDetails() const {
    return Employee::getDetails() + QString(", Tool: %1, Projects: %2")
                                        .arg(designTool)
                                        .arg(numberOfProjects);
}

double Designer::calculateBonus() const {
    return (getSalary() * kDesignerSalaryMultiplier) +
           (numberOfProjects * kDesignerProjectBonus);
}

QA::QA(int employeeId, QString employeeName, double employeeSalary,
       QString employeeDepartment, QString qaTestingType, int qaBugsFound,
       double employmentRate)
    : Employee(employeeId, std::move(employeeName), "QA", employeeSalary,
               std::move(employeeDepartment), employmentRate),
      testingType(std::move(qaTestingType)),
      bugsFound(qaBugsFound) {
    if (qaBugsFound < 0) {
        throw EmployeeException("Bugs found cannot be negative");
    }
}

QString QA::getEmployeeType() const { return "QA"; }

QString QA::getDetails() const {
    return Employee::getDetails() +
           QString(", Testing Type: %1, Bugs Found: %2")
               .arg(testingType)
               .arg(bugsFound);
}

double QA::calculateBonus() const {
    return (getSalary() * kQaSalaryMultiplier) + (bugsFound * kQaBugBonus);
}

QString Developer::getProgrammingLanguage() const {
    return programmingLanguage;
}

double Developer::getYearsOfExperience() const { return yearsOfExperience; }

QString Designer::getDesignTool() const { return designTool; }

int Designer::getNumberOfProjects() const { return numberOfProjects; }

QString QA::getTestingType() const { return testingType; }

int QA::getBugsFound() const { return bugsFound; }
