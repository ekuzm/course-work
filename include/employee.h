#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include <QString>
#include <memory>

class EmployeeException : public std::exception {
   private:
    QString message;

   public:
    EmployeeException(QString msg);
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

    // Operator overloading (for friend function demonstration)
    bool operator==(const Employee& otherEmployee) const;
    bool operator<(const Employee& otherEmployee) const;
    friend std::ostream& operator<<(std::ostream& outputStream,
                                    const Employee& employee);
};

#endif  // EMPLOYEE_
