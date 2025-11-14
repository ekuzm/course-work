#include "services/employee_service.h"

#include "exceptions/exceptions.h"

EmployeeService::EmployeeService(Company* company) : company(company) {
    if (!company) {
        throw CompanyException("Company cannot be null");
    }
}

void EmployeeService::recalculateEmployeeHours() {
    
    auto employees = company->getAllEmployees();
    for (const auto& emp : employees) {
        if (emp) {
            
            int currentHours = emp->getCurrentWeeklyHours();
            if (currentHours > 0) {
                try {
                    emp->removeWeeklyHours(currentHours);
                } catch (const EmployeeException&) {
                    
                    // Ignore exception
                }
            }
        }
    }

    
    auto allAssignments = company->getAllTaskAssignments();
    for (const auto& assignment : allAssignments) {
        auto [employeeId, projectId, taskId] = assignment.first;
        int hours = assignment.second;

        std::shared_ptr<Employee> employee = company->getEmployee(employeeId);
        if (employee && employee->getIsActive() && hours > 0) {
            try {
                employee->addWeeklyHours(hours);
            } catch (const EmployeeException&) {
                
                    // Ignore exception
            }
        }
    }
}

int EmployeeService::calculateTotalAssignedHours(int employeeId) const {
    int totalHours = 0;
    auto allAssignments = company->getAllTaskAssignments();
    
    for (const auto& assignment : allAssignments) {
        int empId = std::get<0>(assignment.first);
        if (empId == employeeId) {
            totalHours += assignment.second;
        }
    }
    
    return totalHours;
}




