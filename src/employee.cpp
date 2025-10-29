#include "../include/employee.h"

#include <compare>
#include <ostream>
#include <stdexcept>
#include <utility>

EmployeeException::EmployeeException(QString msg) : message(std::move(msg)) {}

const char* EmployeeException::what() const noexcept {
    return message.toLocal8Bit().constData();
}

int Employee::getId() const { return id; }

QString Employee::getName() const { return name; }

QString Employee::getPosition() const { return position; }

double Employee::getSalary() const { return salary; }

QString Employee::getDepartment() const { return department; }

Employee::Employee(int employeeId, QString employeeName, QString employeePosition,
                   double employeeSalary, QString employeeDepartment)
    : id(employeeId),
      name(std::move(employeeName)),
      position(std::move(employeePosition)),
      salary(employeeSalary),
      department(std::move(employeeDepartment)) {
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
    return 0.0;  // Base implementation
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

bool Employee::operator==(const Employee &otherEmployee) const {
    return id == otherEmployee.id && name == otherEmployee.name;
}

std::partial_ordering Employee::operator<=>(const Employee &otherEmployee) const {
    if (auto cmp = salary <=> otherEmployee.salary; cmp != 0) {
        return cmp;
    }
    return id <=> otherEmployee.id;
}

std::ostream &operator<<(std::ostream &outputStream, const Employee &employee) {
    outputStream << employee.id << " - " << employee.name.toStdString() << " ("
       << employee.position.toStdString() << ")";
    return outputStream;
}
