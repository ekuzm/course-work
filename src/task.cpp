#include "task.h"

#include <utility>

Task::Task(int taskId, const QString& name, const QString& type,
           int estimatedHours, int priority)
    : id(taskId),
      name(name),
      type(type),
      estimatedHours(estimatedHours),
      priority(priority),
      phase("Planned") {
    if (name.isEmpty()) throw TaskException("Task name cannot be empty");
    if (estimatedHours < 0)
        throw TaskException("Estimated hours cannot be negative");
}

int Task::getId() const { return id; }

QString Task::getName() const { return name; }

QString Task::getType() const { return type; }

int Task::getEstimatedHours() const { return estimatedHours; }

int Task::getAllocatedHours() const { return allocatedHours; }

int Task::getPriority() const { return priority; }

QString Task::getPhase() const { return phase; }

void Task::setEstimatedHours(int hours) {
    if (hours < 0) throw TaskException("Estimated hours cannot be negative");
    estimatedHours = hours;
    updatePhase();
}

void Task::setAllocatedHours(int hours) {
    if (hours < 0) throw TaskException("Allocated hours cannot be negative");
    allocatedHours = hours;
    updatePhase();
}

void Task::addAllocatedHours(int hours) {
    if (hours < 0) throw TaskException("Cannot add negative hours");
    allocatedHours += hours;
    updatePhase();
}

void Task::setPhase(const QString& phaseValue) {
    if (phaseValue.isEmpty()) throw TaskException("Phase cannot be empty");
    phase = phaseValue;
}

void Task::updatePhase() {
    if (allocatedHours == 0) {
        phase = "Planned";
    } else if (allocatedHours >= estimatedHours) {
        phase = "Completed";
    } else {
        phase = "In Progress";
    }
}
