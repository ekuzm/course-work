#include "../include/derived_employees.h"

#include <utility>

// Manager implementation
Manager::Manager(int employeeId, QString employeeName, double employeeSalary,
                 QString employeeDepartment, int managerTeamSize,
                 QString managedProject)
    : Employee(employeeId, std::move(employeeName), "Manager", employeeSalary,
               std::move(employeeDepartment)),
      teamSize(managerTeamSize),
      projectManaged(std::move(managedProject)) {
    if (managerTeamSize < 0) {
        throw EmployeeException("Team size cannot be negative");
    }
}

QString Manager::getEmployeeType() const { return "Manager"; }

QString Manager::getDetails() const {
    return Employee::getDetails() + QString(", Team Size: %1, Project: %2")
                                        .arg(teamSize)
                                        .arg(projectManaged);
}

double Manager::calculateBonus() const {
    return getSalary() * kManagerSalaryMultiplier + teamSize * kManagerTeamBonus;
}

// Developer implementation
Developer::Developer(int employeeId, QString employeeName, double employeeSalary,
                     QString employeeDepartment,
                     QString developerProgrammingLanguage,
                     int developerYearsOfExperience)
    : Employee(employeeId, std::move(employeeName), "Developer", employeeSalary,
               std::move(employeeDepartment)),
      programmingLanguage(std::move(developerProgrammingLanguage)),
      yearsOfExperience(developerYearsOfExperience) {
    if (developerYearsOfExperience < 0) {
        throw EmployeeException("Years of experience cannot be negative");
    }
}

QString Developer::getEmployeeType() const { return "Developer"; }

QString Developer::getDetails() const {
    return Employee::getDetails() +
           QString(", Language: %1, Experience: %2 years")
               .arg(programmingLanguage)
               .arg(yearsOfExperience);
}

double Developer::calculateBonus() const {
    return getSalary() * kDeveloperSalaryMultiplier +
           yearsOfExperience * kDeveloperExperienceBonus;
}

// Designer implementation
Designer::Designer(int employeeId, QString employeeName, double employeeSalary,
                   QString employeeDepartment, QString designerTool,
                   int designerNumberOfProjects)
    : Employee(employeeId, std::move(employeeName), "Designer", employeeSalary,
               std::move(employeeDepartment)),
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
    return getSalary() * kDesignerSalaryMultiplier +
           numberOfProjects * kDesignerProjectBonus;
}

// QA implementation
QA::QA(int employeeId, QString employeeName, double employeeSalary,
       QString employeeDepartment, QString qaTestingType, int qaBugsFound)
    : Employee(employeeId, std::move(employeeName), "QA", employeeSalary,
               std::move(employeeDepartment)),
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
    return getSalary() * kQaSalaryMultiplier + bugsFound * kQaBugBonus;
}

int Manager::getTeamSize() const { return teamSize; }

QString Manager::getProjectManaged() const { return projectManaged; }

QString Developer::getProgrammingLanguage() const {
    return programmingLanguage;
}

int Developer::getYearsOfExperience() const { return yearsOfExperience; }

QString Designer::getDesignTool() const { return designTool; }

int Designer::getNumberOfProjects() const { return numberOfProjects; }

QString QA::getTestingType() const { return testingType; }

int QA::getBugsFound() const { return bugsFound; }
