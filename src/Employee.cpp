#include "../include/Employee.h"
#include <ostream>
#include <stdexcept>

Employee::Employee(int id, const QString &name, const QString &position,
                   double salary, const QString &department)
    : id(id), name(name), position(position), salary(salary),
      department(department) {
  if (name.isEmpty()) {
    throw EmployeeException("Employee name cannot be empty");
  }
  if (salary < 0) {
    throw EmployeeException("Salary cannot be negative");
  }
}

QString Employee::getDetails() const {
  return QString("ID: %1, Name: %2, Position: %3, Salary: %4, Department: %5")
      .arg(id)
      .arg(name)
      .arg(position)
      .arg(salary)
      .arg(department);
}

double Employee::calculateBonus() const {
  return 0.0; // Base implementation
}

void Employee::setSalary(double newSalary) {
  if (newSalary < 0) {
    throw EmployeeException("Salary cannot be negative");
  }
  salary = newSalary;
}

void Employee::setDepartment(const QString &newDepartment) {
  if (newDepartment.isEmpty()) {
    throw EmployeeException("Department cannot be empty");
  }
  department = newDepartment;
}

bool Employee::operator==(const Employee &other) const {
  return id == other.id && name == other.name;
}

bool Employee::operator<(const Employee &other) const {
  if (salary != other.salary) {
    return salary < other.salary;
  }
  return id < other.id;
}

std::ostream &operator<<(std::ostream &os, const Employee &emp) {
  os << emp.id << " - " << emp.name.toStdString() << " ("
     << emp.position.toStdString() << ")";
  return os;
}
