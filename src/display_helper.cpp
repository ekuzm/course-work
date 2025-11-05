#include "display_helper.h"

#include <QStringList>
#include <QTableWidgetItem>
#include <algorithm>
#include <map>

#include "derived_employees.h"

QString DisplayHelper::formatProjectInfo(
    const std::shared_ptr<const Employee>& employee,
    const Company* currentCompany) {
    QStringList projectNames;

    if (auto manager = std::dynamic_pointer_cast<const Manager>(employee)) {
        int projectId = manager->getManagedProjectId();
        if (projectId >= 0) {
            const auto* project = currentCompany->getProject(projectId);
            if (project) {
                projectNames.append(project->getName());
            }
        }
    }

    for (int pid : employee->getAssignedProjects()) {
        const auto* project = currentCompany->getProject(pid);
        if (project) {
            QString projectName = project->getName();
            if (!projectNames.contains(projectName)) {
                projectNames.append(projectName);
            }
        }
    }

    return projectNames.isEmpty() ? "-" : projectNames.join(", ");
}

void DisplayHelper::displayEmployees(QTableWidget* employeeTable,
                                     const Company* currentCompany) {
    if (currentCompany == nullptr || employeeTable == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(employees.size());

    for (size_t index = 0; index < employees.size(); ++index) {
        const auto& employee = employees[index];
        employeeTable->setItem(
            index, 0, new QTableWidgetItem(QString::number(employee->getId())));
        employeeTable->setItem(index, 1,
                               new QTableWidgetItem(employee->getName()));
        employeeTable->setItem(index, 2,
                               new QTableWidgetItem(employee->getDepartment()));
        employeeTable->setItem(index, 3,
                               new QTableWidgetItem(QString::number(
                                   employee->getSalary(), 'f', 2)));
        double rate = employee->getEmploymentRate();
        QString rateStr = QString::number(rate, 'f', 2);
        if (rate == 1.0) {
            rateStr = "1.0 (Full)";
        } else if (rate == 0.75) {
            rateStr = "0.75 (3/4)";
        } else if (rate == 0.5) {
            rateStr = "0.5 (Half)";
        } else if (rate == 0.25) {
            rateStr = "0.25 (1/4)";
        }
        employeeTable->setItem(index, 4, new QTableWidgetItem(rateStr));
        employeeTable->setItem(
            index, 5, new QTableWidgetItem(employee->getEmployeeType()));

        QString projectInfo =
            DisplayHelper::formatProjectInfo(employee, currentCompany);
        employeeTable->setItem(index, 6, new QTableWidgetItem(projectInfo));
    }
}

void DisplayHelper::displayProjects(QTableWidget* projectTable,
                                    const Company* currentCompany) {
    if (currentCompany == nullptr || projectTable == nullptr) return;

    auto projects = currentCompany->getAllProjects();
    projectTable->setRowCount(projects.size());

    for (size_t index = 0; index < projects.size(); ++index) {
        const auto& project = projects[index];
        projectTable->setItem(
            index, 0, new QTableWidgetItem(QString::number(project.getId())));
        projectTable->setItem(index, 1,
                              new QTableWidgetItem(project.getName()));
        projectTable->setItem(index, 2,
                              new QTableWidgetItem(project.getStatus()));
        projectTable->setItem(
            index, 3,
            new QTableWidgetItem(QString::number(project.getBudget(), 'f', 2)));
        projectTable->setItem(
            index, 4,
            new QTableWidgetItem(QString::number(project.getEstimatedHours())));
        projectTable->setItem(
            index, 5,
            new QTableWidgetItem(QString::number(project.getAllocatedHours())));
        projectTable->setItem(index, 6,
                              new QTableWidgetItem(project.getClientName()));
    }
}

void DisplayHelper::showCompanyInfo(QTextEdit* companyInfoText,
                                    const Company* currentCompany) {
    if (currentCompany == nullptr || companyInfoText == nullptr) return;

    QString html = R"(
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Arial, sans-serif;
                    margin: 0;
                    padding: 20px;
                    background: linear-gradient(135deg, #f5f7fa 0%, #e8ecf1 100%);
                    color: #333333;
                }
                .container {
                    max-width: 1200px;
                    margin: 0 auto;
                }
                .header-section {
                    margin-bottom: 30px;
                }
                .company-name {
                    font-size: 42px;
                    font-weight: 800;
                    background: linear-gradient(135deg, #1565c0 0%, #2196f3 100%);
                    -webkit-background-clip: text;
                    -webkit-text-fill-color: transparent;
                    background-clip: text;
                    margin-bottom: 10px;
                    padding-bottom: 20px;
                    border-bottom: 3px solid #2196f3;
                }
                .content-grid {
                    display: grid;
                    grid-template-columns: 1fr;
                    gap: 25px;
                    margin-top: 20px;
                }
                .info-card {
                    background: #ffffff;
                    padding: 30px;
                    border-radius: 16px;
                    box-shadow: 0 4px 20px rgba(25,118,210,0.12);
                    border: 2px solid #e3f2fd;
                    transition: transform 0.3s, box-shadow 0.3s;
                }
                .info-card:hover {
                    transform: translateY(-2px);
                    box-shadow: 0 8px 30px rgba(25,118,210,0.18);
                }
                .info-icon {
                    font-size: 24px;
                    margin-right: 12px;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    width: 40px;
                    height: 40px;
                    background: linear-gradient(135deg, #e3f2fd 0%, #bbdefb 100%);
                    border-radius: 10px;
                    box-shadow: 0 2px 8px rgba(33,150,243,0.15);
                }
                .info-label {
                    font-size: 16px;
                    color: #1565c0;
                    font-weight: 700;
                    min-width: 120px;
                }
                .separator {
                    color: #90caf9;
                    font-size: 18px;
                    font-weight: 600;
                    margin: 0 12px;
                }
                .info-value {
                    font-size: 18px;
                    color: #000000;
                    font-weight: 600;
                    flex: 1;
                }
                .info-row {
                    display: flex;
                    align-items: center;
                    padding: 20px;
                    margin: 10px 0;
                    border-radius: 12px;
                    background: linear-gradient(135deg, #ffffff 0%, #f8fbff 100%);
                    border: 2px solid #e3f2fd;
                    transition: all 0.3s ease;
                    box-shadow: 0 2px 8px rgba(33,150,243,0.08);
                }
                .info-row:hover {
                    transform: translateX(5px);
                    box-shadow: 0 4px 16px rgba(33,150,243,0.2);
                    border-color: #90caf9;
                    background: linear-gradient(135deg, #ffffff 0%, #e3f2fd 100%);
                }
            </style>
        </head>
        <body>
            <div class="container">
    )";

    html += QString(R"(
        <div class="header-section">
            <div class="company-name">%1</div>
        </div>
        <div class="content-grid">
            <div class="info-card">
                <div class="info-row">
                    <span class="info-label">Industry</span>
                    <span class="separator">—</span>
                    <span class="info-value">%2</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Location</span>
                    <span class="separator">—</span>
                    <span class="info-value">%3</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Founded</span>
                    <span class="separator">—</span>
                    <span class="info-value">%4</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Employees</span>
                    <span class="separator">—</span>
                    <span class="info-value">%5</span>
                </div>
                <div class="info-row">
                    <span class="info-label">Projects</span>
                    <span class="separator">—</span>
                    <span class="info-value">%6</span>
                </div>
            </div>
        </div>
            </div>
    )")
                .arg(currentCompany->getName())
                .arg(currentCompany->getIndustry())
                .arg(currentCompany->getLocation())
                .arg(currentCompany->getFoundedYear())
                .arg(currentCompany->getEmployeeCount())
                .arg(currentCompany->getProjectCount());

    html += R"(
        </body>
        </html>
    )";

    companyInfoText->setHtml(html);
}

void DisplayHelper::showStatistics(QTextEdit* statisticsText,
                                   const Company* currentCompany) {
    if (currentCompany == nullptr || statisticsText == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    auto projects = currentCompany->getAllProjects();

    int totalEmployees = currentCompany->getEmployeeCount();
    int totalProjects = currentCompany->getProjectCount();
    double totalSalaries = currentCompany->getTotalSalaries();
    double totalBudget = currentCompany->getTotalBudget();

    double totalEmployeeCosts = 0.0;
    int totalAllocatedHours = 0;
    int totalEstimatedHours = 0;
    int activeEmployees = 0;
    int totalCapacity = 0;
    int totalUsedHours = 0;

    for (const auto& project : projects) {
        totalEmployeeCosts += project.getEmployeeCosts();
        totalAllocatedHours += project.getAllocatedHours();
        totalEstimatedHours += project.getEstimatedHours();
    }

    for (const auto& employee : employees) {
        if (employee && employee->getIsActive()) {
            activeEmployees++;
            totalCapacity += employee->getWeeklyHoursCapacity();
            totalUsedHours += employee->getCurrentWeeklyHours();
        }
    }

    std::map<QString, int> employeeTypeCount;
    for (const auto& employee : employees) {
        employeeTypeCount[employee->getEmployeeType()]++;
    }

    std::map<QString, int> projectStatusCount;
    for (const auto& project : projects) {
        projectStatusCount[project.getStatus()]++;
    }

    double utilizationPercent =
        (totalCapacity > 0)
            ? (static_cast<double>(totalUsedHours) / totalCapacity * 100.0)
            : 0.0;

    double budgetUtilizationPercent =
        (totalBudget > 0) ? (totalEmployeeCosts / totalBudget * 100.0) : 0.0;

    double averageSalary =
        (totalEmployees > 0) ? (totalSalaries / totalEmployees) : 0.0;

    double averageProjectBudget =
        (totalProjects > 0) ? (totalBudget / totalProjects) : 0.0;

    QString html = R"(
        <html>
        <head>
            <style>
                body {
                    font-family: 'Segoe UI', Arial, sans-serif;
                    margin: 0;
                    padding: 20px;
                    background-color: #ffffff;
                    color: #333333;
                }
                .header {
                    background: linear-gradient(135deg, #1976d2 0%, #1565c0 100%);
                    color: #ffffff;
                    padding: 25px;
                    border-radius: 12px;
                    margin-bottom: 25px;
                    box-shadow: 0 4px 12px rgba(25,118,210,0.2);
                    border: 1px solid #1565c0;
                }
                .header h1 {
                    margin: 0;
                    font-size: 28px;
                    font-weight: bold;
                    color: #000000;
                }
                .stats-grid {
                    display: grid;
                    grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
                    gap: 20px;
                    margin-bottom: 25px;
                }
                .stat-card {
                    background: #ffffff;
                    padding: 20px;
                    border-radius: 10px;
                    box-shadow: 0 2px 8px rgba(25,118,210,0.1);
                    border-left: 4px solid #2196f3;
                    border: 1px solid #e3f2fd;
                    transition: transform 0.2s;
                }
                .stat-card:hover {
                    transform: translateY(-2px);
                    box-shadow: 0 4px 16px rgba(25,118,210,0.25);
                    border-left-color: #42a5f5;
                    border-color: #90caf9;
                }
                .stat-label {
                    font-size: 13px;
                    color: #1976d2;
                    text-transform: uppercase;
                    letter-spacing: 0.5px;
                    margin-bottom: 8px;
                    font-weight: 600;
                }
                .stat-value {
                    font-size: 32px;
                    font-weight: bold;
                    color: #000000;
                    margin-bottom: 5px;
                }
                .stat-detail {
                    font-size: 12px;
                    color: #666666;
                    margin-top: 5px;
                }
                .section {
                    background: #ffffff;
                    padding: 25px;
                    border-radius: 10px;
                    margin-bottom: 25px;
                    box-shadow: 0 2px 8px rgba(25,118,210,0.1);
                    border: 1px solid #e3f2fd;
                }
                .section-title {
                    font-size: 20px;
                    font-weight: bold;
                    color: #1565c0;
                    margin-bottom: 20px;
                    padding-bottom: 10px;
                    border-bottom: 2px solid #2196f3;
                }
                .item-row {
                    display: flex;
                    justify-content: space-between;
                    padding: 12px 0;
                    border-bottom: 1px solid #e3f2fd;
                }
                .item-row:last-child {
                    border-bottom: none;
                }
                .item-label {
                    font-size: 15px;
                    color: #1976d2;
                }
                .item-value {
                    font-size: 15px;
                    font-weight: 600;
                    color: #000000;
                }
                .progress-bar {
                    background-color: #e3f2fd;
                    border-radius: 10px;
                    height: 20px;
                    margin-top: 8px;
                    overflow: hidden;
                    border: 1px solid #bbdefb;
                }
                .progress-fill {
                    height: 100%;
                    background: linear-gradient(90deg, #2196f3 0%, #1976d2 100%);
                    border-radius: 10px;
                    transition: width 0.3s;
                    display: flex;
                    align-items: center;
                    justify-content: center;
                    color: white;
                    font-size: 11px;
                    font-weight: bold;
                }
                .badge {
                    display: inline-block;
                    padding: 4px 12px;
                    border-radius: 12px;
                    font-size: 12px;
                    font-weight: 600;
                    margin-left: 8px;
                }
                .badge-success { background-color: #4caf50; color: white; }
                .badge-warning { background-color: #ff9800; color: white; }
                .badge-danger { background-color: #f44336; color: white; }
                .badge-info { background-color: #2196f3; color: white; }
            </style>
        </head>
        <body>
    )";

    html += QString(R"(
        <div class="header">
            <h1>📊 Company Statistics</h1>
        </div>
    )");

    html += R"(<div class="stats-grid">)";

    html += QString(R"(
        <div class="stat-card">
            <div class="stat-label">Total Employees</div>
            <div class="stat-value">%1</div>
            <div class="stat-detail">%2 active</div>
        </div>
    )")
                .arg(totalEmployees)
                .arg(activeEmployees);

    html += QString(R"(
        <div class="stat-card">
            <div class="stat-label">Total Projects</div>
            <div class="stat-value">%1</div>
            <div class="stat-detail">$%2 avg budget</div>
        </div>
    )")
                .arg(totalProjects)
                .arg(averageProjectBudget, 0, 'f', 2);

    html += QString(R"(
        <div class="stat-card">
            <div class="stat-label">Total Salaries</div>
            <div class="stat-value">$%1</div>
            <div class="stat-detail">$%2 avg per employee</div>
        </div>
    )")
                .arg(totalSalaries, 0, 'f', 2)
                .arg(averageSalary, 0, 'f', 2);

    html += QString(R"(
        <div class="stat-card">
            <div class="stat-label">Total Budget</div>
            <div class="stat-value">$%1</div>
            <div class="stat-detail">$%2 allocated</div>
        </div>
    )")
                .arg(totalBudget, 0, 'f', 2)
                .arg(totalEmployeeCosts, 0, 'f', 2);

    html += "</div>";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Employees by Type</div>)";
    for (const auto& [employeeType, count] : employeeTypeCount) {
        double percentage =
            (totalEmployees > 0)
                ? (static_cast<double>(count) / totalEmployees * 100.0)
                : 0.0;
        html += QString(R"(
            <div class="item-row">
                <span class="item-label">%1</span>
                <span class="item-value">%2 <span class="badge badge-info">%3%</span></span>
            </div>
        )")
                    .arg(employeeType)
                    .arg(count)
                    .arg(percentage, 0, 'f', 1);
    }
    html += "</div>";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Projects by Status</div>)";
    for (const auto& [status, count] : projectStatusCount) {
        double percentage =
            (totalProjects > 0)
                ? (static_cast<double>(count) / totalProjects * 100.0)
                : 0.0;
        html += QString(R"(
            <div class="item-row">
                <span class="item-label">%1</span>
                <span class="item-value">%2 <span class="badge badge-info">%3%</span></span>
            </div>
        )")
                    .arg(status)
                    .arg(count)
                    .arg(percentage, 0, 'f', 1);
    }
    html += "</div>";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Resource Utilization</div>)";

    html += QString(R"(
        <div class="item-row">
            <span class="item-label">Employee Capacity Utilization</span>
            <span class="item-value">%1%</span>
        </div>
        <div class="progress-bar">
            <div class="progress-fill" style="width: %1%;">%2h / %3h</div>
        </div>
    )")
                .arg(utilizationPercent, 0, 'f', 1)
                .arg(totalUsedHours)
                .arg(totalCapacity);

    html += QString(R"(
        <div class="item-row" style="margin-top: 20px;">
            <span class="item-label">Budget Utilization</span>
            <span class="item-value">%1%</span>
        </div>
        <div class="progress-bar">
            <div class="progress-fill" style="width: %1%;">$%2 / $%3</div>
        </div>
    )")
                .arg(budgetUtilizationPercent, 0, 'f', 1)
                .arg(totalEmployeeCosts, 0, 'f', 2)
                .arg(totalBudget, 0, 'f', 2);

    html += QString(R"(
        <div class="item-row" style="margin-top: 20px;">
            <span class="item-label">Hours Allocation</span>
            <span class="item-value">%1h / %2h</span>
        </div>
        <div class="progress-bar">
            <div class="progress-fill" style="width: %3%;">%1h allocated</div>
        </div>
    )")
                .arg(totalAllocatedHours)
                .arg(totalEstimatedHours)
                .arg((totalEstimatedHours > 0)
                         ? (static_cast<double>(totalAllocatedHours) /
                            totalEstimatedHours * 100.0)
                         : 0.0,
                     0, 'f', 1);

    html += "</div>";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Financial Summary</div>)";

    html += QString(R"(
        <div class="item-row">
            <span class="item-label">Total Employee Costs</span>
            <span class="item-value">$%1</span>
        </div>
        <div class="item-row">
            <span class="item-label">Remaining Budget</span>
            <span class="item-value">$%2</span>
        </div>
        <div class="item-row">
            <span class="item-label">Average Salary per Employee</span>
            <span class="item-value">$%3</span>
        </div>
        <div class="item-row">
            <span class="item-label">Average Project Budget</span>
            <span class="item-value">$%4</span>
        </div>
    )")
                .arg(totalEmployeeCosts, 0, 'f', 2)
                .arg((totalBudget - totalEmployeeCosts), 0, 'f', 2)
                .arg(averageSalary, 0, 'f', 2)
                .arg(averageProjectBudget, 0, 'f', 2);

    html += "</div>";

    html += R"(
        </body>
        </html>
    )";

    statisticsText->setHtml(html);
}
