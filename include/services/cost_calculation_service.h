#pragma once

class CostCalculationService {
   public:
    static double calculateHourlyRate(double monthlySalary);
    static double calculateEmployeeCost(double monthlySalary, int hours);
};




