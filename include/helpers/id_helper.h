#pragma once

#include <memory>
#include <vector>

class Employee;
class Project;

class IdHelper {
   public:
    static int findMaxEmployeeId(
        const std::vector<std::shared_ptr<Employee>>& employees);

    static int findMaxProjectId(const std::vector<Project>& projects);

    static int calculateNextId(int maxId);
};

