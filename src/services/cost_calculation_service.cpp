#include "services/cost_calculation_service.h"

#include "utils/consts.h"

double CostCalculationService::calculateHourlyRate(double monthlySalary) {
    if (kHoursPerMonth <= 0) return 0.0;
    return monthlySalary / kHoursPerMonth;
}

double CostCalculationService::calculateEmployeeCost(double monthlySalary,
                                                      int hours) {
    return calculateHourlyRate(monthlySalary) * hours;
}

