#pragma once

#include <memory>
#include <vector>

#include "entities/company.h"
#include "entities/employee.h"

class EmployeeService {
   private:
    Company* company;

   public:
    explicit EmployeeService(Company* company);

    void recalculateEmployeeHours() const;
    int calculateTotalAssignedHours(int employeeId) const;
};




