#pragma once

#include <QDate>
#include <QString>

#include "exceptions/exceptions.h"

class Task {
   private:
    int id;
    QString name;
    QString type;
    int estimatedHours;
    int allocatedHours{0};
    int priority;
    QString phase{"Planned"};

    void updatePhase();

   public:
    Task(int taskId, const QString& name, const QString& type,
         int estimatedHours, int priority);

    int getId() const;
    QString getName() const;
    QString getType() const;
    int getEstimatedHours() const;
    int getAllocatedHours() const;
    int getPriority() const;
    QString getPhase() const;

    void setEstimatedHours(int hours);
    void setAllocatedHours(int hours);
    void addAllocatedHours(int hours);
    void setPhase(const QString& phaseValue);
};
