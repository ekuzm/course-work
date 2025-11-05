#include "task.h"

#include <utility>

Task::Task(int taskId, const QString& name, const QString& type,
           int estimatedHours, int priority)
    : id(taskId),
      name(name),
      type(type),
      estimatedHours(estimatedHours),
      priority(priority),
      status("Planned") {
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

QString Task::getStatus() const { return status; }

void Task::setEstimatedHours(int hours) {
    if (hours < 0) throw TaskException("Estimated hours cannot be negative");
    estimatedHours = hours;
    updateStatus();
}

void Task::setAllocatedHours(int hours) {
    if (hours < 0) throw TaskException("Allocated hours cannot be negative");
    allocatedHours = hours;
    updateStatus();
}

void Task::addAllocatedHours(int hours) {
    if (hours < 0) throw TaskException("Cannot add negative hours");
    allocatedHours += hours;
    updateStatus();
}

void Task::setStatus(const QString& statusValue) {
    if (statusValue.isEmpty()) throw TaskException("Status cannot be empty");
    status = statusValue;
}

void Task::updateStatus() {
    if (allocatedHours == 0) {
        status = "Planned";
    } else if (allocatedHours >= estimatedHours) {
        status = "Completed";
    } else {
        status = "In Progress";
    }
}
