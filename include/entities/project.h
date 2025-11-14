#pragma once

#include <QDate>
#include <QString>
#include <memory>
#include <vector>

#include "exceptions/exceptions.h"
#include "entities/task.h"

class Project {
   private:
    int id;
    QString name;
    QString description;
    QString phase;
    QDate startDate;
    QDate endDate;
    double budget;
    QString clientName;
    int initialEstimatedHours;
    int allocatedHours{0};
    double employeeCosts{0.0};
    std::vector<Task> tasks;

   public:
    Project(int projectId, const QString& name, const QString& description,
            const QString& phase, const QDate& startDate, const QDate& endDate,
            double budget, const QString& clientName, int estimatedHours = 0);

    int getId() const;
    QString getName() const;
    QString getDescription() const;
    QString getPhase() const;
    QDate getStartDate() const;
    QDate getEndDate() const;
    double getBudget() const;
    QString getClientName() const;
    int getEstimatedHours() const;
    int getInitialEstimatedHours() const;
    int getAllocatedHours() const;
    double getEmployeeCosts() const;
    const std::vector<Task>& getTasks() const;
    std::vector<Task>& getTasks();
    void addTask(const Task& task);
    void clearTasks();
    int getTasksEstimatedTotal() const;
    int getTasksAllocatedTotal() const;
    int getNextTaskId() const;

    static int getPhaseOrder(const QString& phaseName);
    void setPhase(const QString& newPhase);
    void setBudget(double newBudget);
    void setEstimatedHours(int hours);
    void setAllocatedHours(int hours);
    void addEmployeeCost(double cost);
    void removeEmployeeCost(double cost);
    void recomputeTotalsFromTasks();

    bool isActive() const;
};
