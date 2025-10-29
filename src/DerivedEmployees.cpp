#include "../include/DerivedEmployees.h"

// Manager implementation
Manager::Manager(int id, const QString &name, double salary,
                 const QString &department, int teamSize,
                 const QString &projectManaged)
    : Employee(id, name, "Manager", salary, department), teamSize(teamSize),
      projectManaged(projectManaged) {
  if (teamSize < 0) {
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
  using namespace EmployeeBonusConstants;
  return salary * MANAGER_SALARY_MULTIPLIER + teamSize * MANAGER_TEAM_BONUS;
}

// Developer implementation
Developer::Developer(int id, const QString &name, double salary,
                     const QString &department,
                     const QString &programmingLanguage, int yearsOfExperience)
    : Employee(id, name, "Developer", salary, department),
      programmingLanguage(programmingLanguage),
      yearsOfExperience(yearsOfExperience) {
  if (yearsOfExperience < 0) {
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
  using namespace EmployeeBonusConstants;
  return salary * DEVELOPER_SALARY_MULTIPLIER + yearsOfExperience * DEVELOPER_EXPERIENCE_BONUS;
}

// Designer implementation
Designer::Designer(int id, const QString &name, double salary,
                   const QString &department, const QString &designTool,
                   int numberOfProjects)
    : Employee(id, name, "Designer", salary, department),
      designTool(designTool), numberOfProjects(numberOfProjects) {
  if (numberOfProjects < 0) {
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
  using namespace EmployeeBonusConstants;
  return salary * DESIGNER_SALARY_MULTIPLIER + numberOfProjects * DESIGNER_PROJECT_BONUS;
}

// QA implementation
QA::QA(int id, const QString &name, double salary, const QString &department,
       const QString &testingType, int bugsFound)
    : Employee(id, name, "QA", salary, department), testingType(testingType),
      bugsFound(bugsFound) {
  if (bugsFound < 0) {
    throw EmployeeException("Bugs found cannot be negative");
  }
}

QString QA::getEmployeeType() const { return "QA"; }

QString QA::getDetails() const {
  return Employee::getDetails() + QString(", Testing Type: %1, Bugs Found: %2")
                                      .arg(testingType)
                                      .arg(bugsFound);
}

double QA::calculateBonus() const {
  using namespace EmployeeBonusConstants;
  return salary * QA_SALARY_MULTIPLIER + bugsFound * QA_BUG_BONUS;
}

