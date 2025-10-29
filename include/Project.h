#ifndef PROJECT_H
#define PROJECT_H

#include <QString>
#include <QDate>
#include <memory>

// Exception class for projects
class ProjectException : public std::exception {
private:
    QString message;
public:
    ProjectException(const QString& msg) : message(msg) {}
    const char* what() const noexcept override {
        return message.toLocal8Bit().constData();
    }
};

class Project {
private:
    int id;
    QString name;
    QString description;
    QString status;
    QDate startDate;
    QDate endDate;
    double budget;
    QString clientName;

public:
    Project(int id, const QString& name, const QString& description, 
            const QString& status, const QDate& startDate, const QDate& endDate,
            double budget, const QString& clientName);

    // Getters
    int getId() const { return id; }
    QString getName() const { return name; }
    QString getDescription() const { return description; }
    QString getStatus() const { return status; }
    QDate getStartDate() const { return startDate; }
    QDate getEndDate() const { return endDate; }
    double getBudget() const { return budget; }
    QString getClientName() const { return clientName; }

    // Setters
    void setStatus(const QString& newStatus);
    void setBudget(double newBudget);

    // Operator overloading
    bool operator==(const Project& other) const;
    bool operator<(const Project& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Project& proj);

    // Calculate days left/duration
    int getDaysDuration() const;
    bool isActive() const;
};

#endif // PROJECT_H


