#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <memory>
#include <QString>

// Custom exception class
class EmployeeException : public std::exception {
private:
    QString message;
public:
    EmployeeException(const QString& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.toLocal8Bit().constData();
    }
};

// Abstract base class with virtual functions
class Employee {
protected:
    int id;
    QString name;
    QString position;
    double salary;
    QString department;

public:
    Employee(int id, const QString& name, const QString& position, double salary, const QString& department);
    virtual ~Employee() = default;

    // Virtual functions - polymorphism
    virtual QString getEmployeeType() const = 0;
    virtual QString getDetails() const;
    virtual double calculateBonus() const = 0;

    // Getters
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getPosition() const { return position; }
    double getSalary() const { return salary; }
    QString getDepartment() const { return department; }

    // Setters with exception handling
    void setSalary(double newSalary);
    void setDepartment(const QString& newDepartment);

    // Operator overloading (for friend function demonstration)
    bool operator==(const Employee& other) const;
    bool operator<(const Employee& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Employee& emp);
};

#endif // EMPLOYEE_H


