#ifndef PROJECT_H
#define PROJECT_H

#include <QDate>
#include <QString>
#include <memory>
#include <ostream>

// Exception class for projects
class ProjectException : public std::exception {
   private:
    QString message;

   public:
    explicit ProjectException(QString msg);
    const char* what() const noexcept override;
};

struct ProjectParams {
    QString name;
    QString description;
    QString status;
    QDate startDate;
    QDate endDate;
    double budget;
    QString clientName;
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
    Project(int projectId, const ProjectParams& params);

    // Getters
    int getId() const;
    QString getName() const;
    QString getDescription() const;
    QString getStatus() const;
    QDate getStartDate() const;
    QDate getEndDate() const;
    double getBudget() const;
    QString getClientName() const;

    // Setters
    void setStatus(const QString& newStatus);
    void setBudget(double newBudget);

    // Operator overloading
    bool operator==(const Project& otherProject) const;
    bool operator<(const Project& otherProject) const;

    // Hidden friend
    friend std::ostream& operator<<(std::ostream& outputStream,
                                    const Project& project) {
        outputStream << project.id << " - " << project.name.toStdString() << " ("
                     << project.status.toStdString() << ")";
        return outputStream;
    }

    // Calculate days left/duration
    int getDaysDuration() const;
    bool isActive() const;
};

#endif  // PROJECT_H
