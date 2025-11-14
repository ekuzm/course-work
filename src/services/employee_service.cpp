#include "services/employee_service.h"

#include <QLoggingCategory>

#include "exceptions/exceptions.h"

Q_LOGGING_CATEGORY(employeeService, "employee.service")

EmployeeService::EmployeeService(Company* company) : company(company) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
}

void EmployeeService::recalculateEmployeeHours() const {
    
    auto employees = company->getAllEmployees();
    for (const auto& emp : employees) {
        if (emp) {
            // Remove existing hours
            if (int currentHours = emp->getCurrentWeeklyHours(); currentHours > 0) {
                try {
                    emp->removeWeeklyHours(currentHours);
                } catch (const EmployeeException& e) {
                    qCWarning(employeeService) << "Failed to remove weekly hours:" << e.what();
                }
            }
        }
    }

    
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& [key, hours] : allAssignments) {
        const auto [employeeId, projectId, taskId] = key;

        std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
        // Add hours for active employees
        if (employee && employee->getIsActive() && hours > 0) {
            try {
                employee->addWeeklyHours(hours);
            } catch (const EmployeeException& e) {
                qCWarning(employeeService) << "Failed to add weekly hours:" << e.what();
            }
        }
    }
}

int EmployeeService::calculateTotalAssignedHours(int employeeId) const {
    int totalHours = 0;
    auto allAssignments = company->getAllTaskAssignments();
    
    for (const auto& [key, hours] : allAssignments) {
        const auto [empId, projectId, taskId] = key;
        if (empId == employeeId) {
            totalHours += hours;
        }
    }
    
    return totalHours;
}




