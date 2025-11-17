#pragma once

#include <QString>
#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <ranges>
#include <tuple>
#include <vector>

#include "entities/employee.h"
#include "utils/consts.h"

inline double calculateHourlyRate(double monthlySalary) {
    if (kHoursPerMonth <= 0) return 0.0;
    return monthlySalary / kHoursPerMonth;
}

inline double calculateEmployeeCost(double monthlySalary, int hours) {
    return calculateHourlyRate(monthlySalary) * hours;
}

inline bool roleMatchesSDLCStage(const QString& employeePosition,
                                 const QString& projectPhase) {
    if (projectPhase == "Analysis" || projectPhase == "Planning") {
        return employeePosition == "Manager";
    }
    if (projectPhase == "Design") {
        return employeePosition == "Designer";
    }
    if (projectPhase == "Development") {
        return employeePosition == "Developer";
    }
    if (projectPhase == "Testing") {
        return employeePosition == "QA";
    }
    return true;
}

inline bool taskTypeMatchesEmployeeType(const QString& taskType,
                                        const QString& employeeType) {
    static const std::map<QString, QString> typeMapping = {
        {"Management", "Manager"},
        {"Development", "Developer"},
        {"Design", "Designer"},
        {"QA", "QA"}};
    if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
        return it->second == employeeType;
    }
    return false;
}

inline QString getRequiredEmployeeType(const QString& taskType) {
    static const std::map<QString, QString> typeMapping = {
        {"Management", "Manager"},
        {"Development", "Developer"},
        {"Design", "Designer"},
        {"QA", "QA"}};
    if (const auto& it = typeMapping.find(taskType); it != typeMapping.end()) {
        return it->second;
    }
    return "Unknown";
}

inline int compareEmployeesForSorting(const std::shared_ptr<Employee>& a,
                                      const std::shared_ptr<Employee>& b,
                                      const std::map<int, int>& employeeUsage) {
    if (!a || !b) return 0;

    double hourlyRateA = calculateHourlyRate(a->getSalary());
    double hourlyRateB = calculateHourlyRate(b->getSalary());

    int usedA = 0;
    if (auto itA = employeeUsage.find(a->getId()); itA != employeeUsage.end()) {
        usedA = itA->second;
    }
    int usedB = 0;
    if (auto itB = employeeUsage.find(b->getId()); itB != employeeUsage.end()) {
        usedB = itB->second;
    }

    int availA = a->getAvailableHours() - usedA;
    int availB = b->getAvailableHours() - usedB;

    double rateDiff = hourlyRateA - hourlyRateB;
    if (rateDiff < -0.01) {
        return -1;
    }
    if (rateDiff > 0.01) {
        return 1;
    }
    if (availA > availB) {
        return -1;
    }
    if (availA < availB) {
        return 1;
    }
    return 0;
}

inline void reduceExcessHours(
    std::vector<std::tuple<int, int, int, int>>& assignmentsData, int& excess,
    int& totalScaledHours) {
    std::ranges::sort(assignmentsData, [](const auto& a, const auto& b) {
        const auto& [projectIdA, taskIdA, oldHoursA, adjustedHoursA] = a;
        const auto& [projectIdB, taskIdB, oldHoursB, adjustedHoursB] = b;
        return adjustedHoursA > adjustedHoursB;
    });

    for (auto& assignment : assignmentsData) {
        if (excess <= 0) break;
        auto& [projectId, taskId, oldHours, adjustedHours] = assignment;
        if (adjustedHours <= 0) {
            continue;
        }

        auto reduction = std::min(excess, adjustedHours);
        adjustedHours = adjustedHours - reduction;
        totalScaledHours -= reduction;
        excess -= reduction;
    }
}

inline void adjustAssignmentsToCapacity(
    std::vector<std::tuple<int, int, int, int>>& assignmentsData, int capacity,
    int& totalScaledHours) {
    if (totalScaledHours <= capacity) {
        return;
    }

    const auto adjustFactor = static_cast<double>(capacity) / totalScaledHours;
    totalScaledHours = 0;

    for (auto& assignment : assignmentsData) {
        auto& [projectId, taskId, oldHours, scaledHours] = assignment;
        auto adjustedHours =
            static_cast<int>(std::round(scaledHours * adjustFactor));

        if (adjustedHours < 0) adjustedHours = 0;
        if (adjustedHours > capacity) adjustedHours = capacity;

        scaledHours = adjustedHours;
        totalScaledHours += adjustedHours;
    }

    if (totalScaledHours > capacity) {
        int excess = totalScaledHours - capacity;
        reduceExcessHours(assignmentsData, excess, totalScaledHours);
    }
}
