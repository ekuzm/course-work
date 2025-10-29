#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <compare>
#include <QString>
#include <memory>
#include <ostream>

class EmployeeException : public std::exception {
   private:
    QString message;

   public:
    explicit EmployeeException(QString msg);
    const char* what() const noexcept override;
};

class Employee {
   private:
    int id;
    QString name;
    QString position;
    double salary;
    QString department;

   public:
    Employee(int employeeId, QString name, QString position, double salary,
             QString department);
    virtual ~Employee() = default;

    virtual QString getEmployeeType() const = 0;
    virtual QString getDetails() const;
    virtual double calculateBonus() const = 0;

    // Getters
    int getId() const;
    QString getName() const;
    QString getPosition() const;
    double getSalary() const;
    QString getDepartment() const;

    // Setters with exception handling
    void setSalary(double newSalary);
    void setDepartment(const QString& newDepartment);

    // Operator overloading
    bool operator==(const Employee& otherEmployee) const;
    std::partial_ordering operator <=> (const Employee& otherEmployee) const;

    // Hidden friend
    friend std::ostream& operator<<(std::ostream& outputStream,
                                    const Employee& employee) {
        outputStream << employee.id << " - " << employee.name.toStdString() << " ("
                     << employee.position.toStdString() << ")";
        return outputStream;
    }
};

#endif  // EMPLOYEE_
