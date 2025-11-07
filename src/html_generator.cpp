#include "html_generator.h"

#include <algorithm>
#include <cmath>
#include <memory>

#include "company.h"
#include "display_helper.h"
#include "employee.h"
#include "project.h"

QString HtmlGenerator::generateProjectDetailHtml(const Project& project, const Company* company) {
    if (!company) return "";

    QString status = project.getStatus().trimmed();
    bool projectCompleted = status.compare("Completed", Qt::CaseInsensitive) == 0;
    QString projectName = project.getName().isEmpty() ? QString("Untitled project")
                                                       : project.getName().toHtmlEscaped();
    QString statusLabel = status.isEmpty() ? QString("Status not set") : status.toHtmlEscaped();
    QString badgeClass = projectCompleted ? QString("phase-badge phase-completed")
                                          : QString("phase-badge phase-active");
    QString subtitle = projectCompleted ? QString("Team members who delivered this project")
                                        : QString("Team members currently assigned to this project");

    auto formatPercentText = [](double value) -> QString {
        double percent = value * 100.0;
        double rounded = std::round(percent * 10.0) / 10.0;
        if (std::fabs(rounded - std::round(rounded)) < 0.05) {
            return QString::number(static_cast<int>(std::round(rounded)));
        }
        return QString::number(rounded, 'f', 1);
    };

    QStringList memberCards;
    auto employees = company->getAllEmployees();
    for (const auto& employee : employees) {
        if (employee == nullptr || !employee->isAssignedToProject(project.getId())) {
            continue;
        }

        QString name = employee->getName().isEmpty()
                           ? QString("Employee #%1").arg(employee->getId())
                           : employee->getName();
        name = name.toHtmlEscaped();

        QString position = employee->getPosition().trimmed();
        QString department = employee->getDepartment().trimmed();
        QString roleLine = position.isEmpty() ? QString("Role not specified") : position.toHtmlEscaped();
        if (!department.isEmpty()) {
            roleLine = QString("%1 · %2").arg(roleLine, department.toHtmlEscaped());
        }

        bool isActiveEmployee = employee->getIsActive();
        QString badgeState = isActiveEmployee ? "member-badge member-badge-active"
                                              : "member-badge member-badge-inactive";
        QString badgeText;
        if (projectCompleted) {
            badgeText = isActiveEmployee ? QString("Delivered · still employed")
                                         : QString("Delivered · former employee");
            if (!isActiveEmployee) {
                badgeState += " member-badge-former";
            }
        } else {
            badgeText = isActiveEmployee ? QString("Active on project") : QString("Inactive");
        }

        QString involvement = projectCompleted
                                  ? (isActiveEmployee ? QString("Stayed with the company after delivery")
                                                      : QString("Contributed until completion"))
                                  : (isActiveEmployee ? QString("Currently contributing")
                                                      : QString("Temporarily unavailable"));

        int capacity = std::max(employee->getWeeklyHoursCapacity(), 0);
        int used = std::max(employee->getCurrentWeeklyHours(), 0);
        int available = std::max(employee->getAvailableHours(), 0);

        QString employmentDisplay =
            QString("%1% FTE").arg(formatPercentText(employee->getEmploymentRate()));
        QString salaryDisplay = QString("$%1").arg(QString::number(employee->getSalary(), 'f', 2));

        QString card = QString(R"(
            <div class="member-card">
                <div class="member-top">
                    <div>
                        <div class="member-name">%1</div>
                        <div class="member-role">%2</div>
                    </div>
                    <span class="%3">%4</span>
                </div>
                <div class="member-context">%5</div>
                <div class="member-divider"></div>
                <div class="member-stats">
                    <div class="stat">
                        <div class="stat-label">Weekly capacity</div>
                        <div class="stat-value">%6h</div>
                    </div>
                    <div class="stat">
                        <div class="stat-label">Allocated</div>
                        <div class="stat-value">%7h</div>
                    </div>
                    <div class="stat">
                        <div class="stat-label">Free</div>
                        <div class="stat-value">%8h</div>
                    </div>
                </div>
                <div class="member-footer">
                    <span class="meta-tag">Employment: %9</span>
                    <span class="meta-tag">Base salary: %10</span>
                </div>
            </div>
        )")
                            .arg(name)
                            .arg(roleLine)
                            .arg(badgeState)
                            .arg(badgeText)
                            .arg(involvement)
                            .arg(capacity)
                            .arg(used)
                            .arg(available)
                            .arg(employmentDisplay)
                            .arg(salaryDisplay);

        memberCards.append(card);
    }

    int teamCount = memberCards.size();
    QString badges = QString("Team size: %1</span>").arg(teamCount);

    QString teamContent;
    if (memberCards.isEmpty()) {
        teamContent = QString("<div class='empty'>%1</div>")
                          .arg(projectCompleted
                                   ? QString("No team members were recorded for this project.")
                                   : QString("No team members assigned yet. Use the Assign button to add specialists."));
    } else {
        teamContent = QString("<div class='team-grid'>%1</div>").arg(memberCards.join(""));
    }

    QString html = QString(R"(
        <html>
        <head>
            <style>
                body { font-family: 'Segoe UI', Arial, sans-serif; margin: 0; padding: 0; background: transparent; color: #12324a; }
                .wrapper { background: #ffffff; border-radius: 18px; border: 1px solid #dce7f8; padding: 30px 32px; box-shadow: 0 26px 48px rgba(15, 76, 129, 0.14); }
                .header { display: flex; justify-content: space-between; align-items: flex-start; gap: 24px; flex-wrap: wrap; margin-bottom: 18px; }
                .title-block { display: flex; flex-direction: column; gap: 6px; }
                .title { font-size: 24px; font-weight: 800; margin: 0; color: #102a43; }
                .subtitle { font-size: 14px; font-weight: 500; color: #5a6e84; }
                .header-badges { display: flex; gap: 10px; flex-wrap: wrap; }
                .phase-badge { display: inline-flex; align-items: center; justify-content: center; padding: 6px 16px; border-radius: 999px; font-size: 12px; font-weight: 600; letter-spacing: 0.3px; }
                .phase-active { background: #e5f1ff; color: #0b4a85; border: 1px solid #bcd7ff; }
                .phase-completed { background: #e8f8f0; color: #1f6b3a; border: 1px solid #c4ead3; }
                .phase-neutral { background: #f0f4fa; color: #3b4e6b; border: 1px solid #d8e0ec; }
                .team-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(260px, 1fr)); gap: 18px; margin-bottom: 18px; }
                .member-card { background: linear-gradient(160deg, #f8fbff 0%, #eef3ff 100%); border: 1px solid #d4e2fc; border-radius: 16px; padding: 20px 22px; box-shadow: 0 20px 35px rgba(18, 71, 120, 0.12); display: flex; flex-direction: column; gap: 16px; margin-bottom: 18px; }
                .member-top { display: flex; justify-content: space-between; align-items: flex-start; gap: 16px; }
                .member-name { font-size: 18px; font-weight: 700; color: #0f3d68; }
                .member-role { font-size: 13px; color: #5a6e84; font-weight: 500; }
                .member-badge { padding: 6px 12px; border-radius: 999px; font-size: 11px; font-weight: 600; letter-spacing: 0.3px; white-space: nowrap; }
                .member-badge-active { background: #e5f4ff; color: #0b4a85; border: 1px solid #bcd7ff; }
                .member-badge-inactive { background: #fbeaea; color: #8b1f2d; border: 1px solid #f3c6cc; }
                .member-badge-former { background: #fdf0e6; color: #9a4e1a; border: 1px solid #f6cfa9; }
                .member-context { font-size: 13px; font-weight: 500; color: #3b4e6b; }
                .member-divider { height: 1px; background: linear-gradient(90deg, rgba(15,61,104,0.05), rgba(15,61,104,0.25), rgba(15,61,104,0.05)); }
                .member-stats { display: grid; grid-template-columns: repeat(3, minmax(0, 1fr)); gap: 12px; }
                .stat { background: rgba(255,255,255,0.65); border: 1px solid rgba(210, 224, 245, 0.8); border-radius: 12px; padding: 12px; text-align: center; }
                .stat-label { font-size: 11px; text-transform: uppercase; letter-spacing: 0.6px; color: #6b7c93; margin-bottom: 6px; }
                .stat-value { font-size: 16px; font-weight: 700; color: #0f3d68; }
                .member-footer { display: flex; flex-wrap: wrap; gap: 10px; }
                .meta-tag { display: inline-flex; align-items: center; padding: 6px 12px; border-radius: 10px; background: #edf2ff; color: #365073; font-size: 12px; font-weight: 600; }
                .empty { padding: 26px; text-align: center; border: 1px dashed #d8e0ec; border-radius: 16px; background: #f6f9ff; color: #5a6e84; font-size: 14px; font-weight: 500; }
            </style>
        </head>
        <body>
            <div class="wrapper">
                <div class="header">
                    <div class="title-block">
                        <div class="title">%1</div>
                        <div class="subtitle">%2</div>
                    </div>
                    <div class="header-badges">%3</div>
                </div>
                %4
            </div>
        </body>
        </html>
    )")
                        .arg(projectName)
                        .arg(subtitle)
                        .arg(badges)
                        .arg(teamContent);

    return html;
}

QString HtmlGenerator::generateProjectAssignmentsHtml(const Project& project, const Company* company) {
    if (!company) return "";

    QString html = R"(
        <html>
        <head>
            <style>
                body { font-family: 'Segoe UI', Arial, sans-serif; margin: 0; padding: 10px; background-color: #ffffff; }
                .header { background: linear-gradient(135deg, #1976d2 0%, #1565c0 100%); color: white; padding: 20px; border-radius: 10px; margin-bottom: 20px; }
                .header h1 { margin: 0; font-size: 24px; color: #000000; }
                .header-detail { font-size: 14px; margin-top: 8px; padding: 8px 12px; background-color: #d0d0d0; border-radius: 6px; color: #000000; display: inline-block; font-weight: normal; }
                .section { background: #f5f5f5; padding: 15px; border-radius: 8px; margin-bottom: 15px; border-left: 4px solid #2196f3; }
                .section-title { font-size: 18px; font-weight: bold; color: #1565c0; margin-bottom: 12px; }
                .metrics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 12px; margin-bottom: 15px; }
                .metric-card { background: white; padding: 12px; border-radius: 6px; border: 1px solid #e0e0e0; }
                .metric-label { font-size: 11px; color: #666; text-transform: uppercase; margin-bottom: 5px; }
                .metric-value { font-size: 20px; font-weight: bold; color: #1976d2; }
                .task-item { background: white; padding: 12px; margin-bottom: 8px; border-radius: 6px; border-left: 3px solid #2196f3; }
                .task-header { font-weight: bold; color: #333; margin-bottom: 6px; }
                .task-details { font-size: 13px; color: #666; }
                .task-badge { display: inline-block; padding: 3px 8px; border-radius: 12px; font-size: 11px; font-weight: 600; margin-left: 8px; }
                .badge-management { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-development { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-design { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-qa { background: #d0d0d0; color: #000000; font-weight: normal; }
                .employee-item { background: white; padding: 12px; margin-bottom: 8px; border-radius: 6px; border-left: 3px solid #4caf50; }
                .employee-header { font-weight: bold; color: #333; margin-bottom: 8px; }
                .employee-details { font-size: 13px; padding: 8px 12px; background-color: #d0d0d0; border-radius: 6px; color: #000000; display: inline-block; margin-bottom: 0; font-weight: normal; }
                .progress-bar { background: #e0e0e0; border-radius: 10px; height: 20px; margin-top: 8px; overflow: hidden; }
                .progress-fill { height: 100%; background: #d0d0d0; display: flex; align-items: center; justify-content: center; color: #000000; font-size: 11px; font-weight: normal; }
                .empty-state { text-align: center; padding: 30px; color: #999; font-style: italic; }
            </style>
        </head>
        <body>
    )";

    html += QString(R"(
        <div class="header">
            <h1>📊 %1</h1>
            <div class="header-detail">Status: %2 | Client: %3 | Budget: $%4</div>
        </div>
    )")
                .arg(project.getName())
                .arg(project.getStatus())
                .arg(project.getClientName())
                .arg(project.getBudget(), 0, 'f', 2);

    int totalEstimated = project.getEstimatedHours();
    int totalAllocated = project.getAllocatedHours();
    int needed = totalEstimated - totalAllocated;
    double budgetUsed = project.getEmployeeCosts();
    double budgetRemaining = project.getBudget() - budgetUsed;
    double hoursPercent = totalEstimated > 0 ? (static_cast<double>(totalAllocated) / totalEstimated * 100.0) : 0.0;
    double budgetPercent = project.getBudget() > 0 ? (budgetUsed / project.getBudget() * 100.0) : 0.0;

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Project Metrics</div>)";
    html += R"(<div class="metrics-grid">)";

    html += QString(R"(
        <div class="metric-card">
            <div class="metric-label">Estimated Hours</div>
            <div class="metric-value">%1h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Allocated Hours</div>
            <div class="metric-value">%2h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Hours Needed</div>
            <div class="metric-value">%3h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Budget Used</div>
            <div class="metric-value">$%4</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Budget Remaining</div>
            <div class="metric-value">$%5</div>
        </div>
    )")
                .arg(totalEstimated)
                .arg(totalAllocated)
                .arg(needed)
                .arg(budgetUsed, 0, 'f', 2)
                .arg(budgetRemaining, 0, 'f', 2);

    html += R"(</div>)";

    html += QString(R"(
        <div style="margin-top: 15px;">
            <div style="font-size: 13px; color: #666; margin-bottom: 5px;">Hours Allocation: %1%</div>
            <div class="progress-bar">
                <div class="progress-fill" style="width: %1%;">%2h / %3h</div>
            </div>
        </div>
        <div style="margin-top: 15px;">
            <div style="font-size: 13px; color: #666; margin-bottom: 5px;">Budget Utilization: %4%</div>
            <div class="progress-bar">
                <div class="progress-fill" style="width: %4%;">$%5 / $%6</div>
            </div>
        </div>
    )")
                .arg(hoursPercent, 0, 'f', 1)
                .arg(totalAllocated)
                .arg(totalEstimated)
                .arg(budgetPercent, 0, 'f', 1)
                .arg(budgetUsed, 0, 'f', 2)
                .arg(project.getBudget(), 0, 'f', 2);

    html += R"(</div>)";

    auto tasks = company->getProjectTasks(project.getId());
    auto allEmployees = company->getAllEmployees();

    std::vector<std::shared_ptr<Employee>> projectEmployees;
    for (const auto& emp : allEmployees) {
        if (emp && emp->isAssignedToProject(project.getId())) {
            projectEmployees.push_back(emp);
        }
    }

    html += R"(<div class="section">)";
    html += QString(R"(<div class="section-title">Team Members (%1)</div>)").arg(projectEmployees.size());

    if (projectEmployees.empty()) {
        html += R"(<div class="empty-state">No employees assigned to this project yet.</div>)";
    } else {
        for (const auto& emp : projectEmployees) {
            QString badgeClass = "badge-info";
            if (emp->getEmployeeType() == "Manager")
                badgeClass = "badge-management";
            else if (emp->getEmployeeType() == "Developer")
                badgeClass = "badge-development";
            else if (emp->getEmployeeType() == "Designer")
                badgeClass = "badge-design";
            else if (emp->getEmployeeType() == "QA")
                badgeClass = "badge-qa";

            double utilization = emp->getWeeklyHoursCapacity() > 0
                                     ? (static_cast<double>(emp->getCurrentWeeklyHours()) /
                                        emp->getWeeklyHoursCapacity() * 100.0)
                                     : 0.0;

            html += QString(R"(
                <div class="employee-item">
                    <div class="employee-header">
                        %1 <span class="task-badge %2">%3</span>
                    </div>
                    <div class="employee-details">
                        Position: %4 | Salary: $%5 | Capacity: %6h/week | Used: %7h/week (%8%) | Available: %9h/week
                    </div>
                </div>
            )")
                        .arg(emp->getName())
                        .arg(badgeClass)
                        .arg(emp->getEmployeeType())
                        .arg(emp->getPosition())
                        .arg(emp->getSalary(), 0, 'f', 2)
                        .arg(emp->getWeeklyHoursCapacity())
                        .arg(emp->getCurrentWeeklyHours())
                        .arg(utilization, 0, 'f', 1)
                        .arg(emp->getAvailableHours());
        }
    }

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html += QString(R"(<div class="section-title">Tasks (%1)</div>)").arg(tasks.size());

    if (tasks.empty()) {
        html += R"(<div class="empty-state">No tasks in this project yet.</div>)";
    } else {
        for (const auto& task : tasks) {
            QString badgeClass = "badge-info";
            if (task.getType() == "Management")
                badgeClass = "badge-management";
            else if (task.getType() == "Development")
                badgeClass = "badge-development";
            else if (task.getType() == "Design")
                badgeClass = "badge-design";
            else if (task.getType() == "QA")
                badgeClass = "badge-qa";

            int remaining = task.getEstimatedHours() - task.getAllocatedHours();
            double taskPercent = task.getEstimatedHours() > 0
                                     ? (static_cast<double>(task.getAllocatedHours()) /
                                        task.getEstimatedHours() * 100.0)
                                     : 0.0;

            html += QString(R"(
                <div class="task-item">
                    <div class="task-header">
                        %1 <span class="task-badge %2">%3</span>
                    </div>
                    <div class="task-details">
                        Estimated: %4h | Allocated: %5h | Remaining: %6h | Priority: %7 | Status: %8
                    </div>
                    <div class="progress-bar" style="margin-top: 8px;">
                        <div class="progress-fill" style="width: %9%;">%10% Complete</div>
                    </div>
                </div>
            )")
                        .arg(task.getName())
                        .arg(badgeClass)
                        .arg(task.getType())
                        .arg(task.getEstimatedHours())
                        .arg(task.getAllocatedHours())
                        .arg(remaining)
                        .arg(task.getPriority())
                        .arg(task.getStatus())
                        .arg(taskPercent, 0, 'f', 1)
                        .arg(taskPercent, 0, 'f', 1);
        }
    }

    html += R"(</div>)";
    html += R"(
        </body>
        </html>
    )";

    return html;
}

QString HtmlGenerator::generateEmployeeHistoryHtml(const Employee& employee, const Company* company,
                                                   const std::vector<const Project*>& employeeProjects) {
    if (!company) return "";

    QString html = R"(
        <html>
        <head>
            <style>
                body { font-family: 'Segoe UI', Arial, sans-serif; margin: 0; padding: 10px; background-color: #ffffff; }
                .header { background-color: #ffffff; color: #000000; padding: 20px; border-radius: 10px; margin-bottom: 20px; border: 1px solid #e0e0e0; }
                .header h1 { margin: 0; font-size: 24px; color: #000000; }
                .header h1 .badge { background: #ffffff; color: #000000; font-weight: normal; }
                .header-detail { font-size: 14px; margin-top: 8px; padding: 8px 12px; background-color: #d0d0d0; border-radius: 6px; color: #000000; display: inline-block; font-weight: normal; }
                .section { background: #f5f5f5; padding: 15px; border-radius: 8px; margin-bottom: 15px; border-left: 4px solid #2196f3; }
                .section-title { font-size: 18px; font-weight: bold; color: #1565c0; margin-bottom: 12px; }
                .metrics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 12px; margin-bottom: 15px; }
                .metric-card { background: white; padding: 12px; border-radius: 6px; border: 1px solid #e0e0e0; }
                .metric-label { font-size: 11px; color: #666; text-transform: uppercase; margin-bottom: 5px; }
                .metric-value { font-size: 20px; font-weight: bold; color: #1976d2; }
                .project-item { background: white; padding: 12px; margin-bottom: 8px; border-radius: 6px; border-left: 3px solid #2196f3; }
                .project-header { font-weight: bold; color: #333; margin-bottom: 8px; }
                .project-details { font-size: 13px; padding: 8px 12px; background-color: #d0d0d0; border-radius: 6px; color: #000000; display: inline-block; margin-bottom: 0; font-weight: normal; }
                .task-item { background: #fafafa; padding: 10px; margin: 8px 0 8px 20px; border-radius: 5px; border-left: 2px solid #2196f3; }
                .task-header { font-weight: 600; color: #555; margin-bottom: 5px; font-size: 14px; }
                .task-details { font-size: 12px; color: #777; }
                .badge { display: inline-block; padding: 3px 8px; border-radius: 12px; font-size: 11px; font-weight: 600; margin-left: 8px; }
                .badge-management { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-development { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-design { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-qa { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-status { background: #d0d0d0; color: #000000; font-weight: normal; }
                .badge-type { background: transparent; color: #000000; }
                .progress-bar { background: #e0e0e0; border-radius: 10px; height: 18px; margin-top: 8px; overflow: hidden; }
                .progress-fill { height: 100%; background: #d0d0d0; display: flex; align-items: center; justify-content: center; color: #000000; font-size: 10px; font-weight: normal; }
                .empty-state { text-align: center; padding: 30px; color: #999; font-style: italic; }
                .info-row { display: flex; justify-content: space-between; padding: 5px 0; border-bottom: 1px solid #e0e0e0; }
                .info-row:last-child { border-bottom: none; }
                .info-label { color: #666; }
                .info-value { color: #333; font-weight: 600; }
            </style>
        </head>
        <body>
    )";

    double utilization = employee.getWeeklyHoursCapacity() > 0
                             ? (static_cast<double>(employee.getCurrentWeeklyHours()) /
                                employee.getWeeklyHoursCapacity() * 100.0)
                             : 0.0;

    QString badgeClass = "badge-type";
    if (employee.getEmployeeType() == "Manager")
        badgeClass = "badge-management";
    else if (employee.getEmployeeType() == "Developer")
        badgeClass = "badge-development";
    else if (employee.getEmployeeType() == "Designer")
        badgeClass = "badge-design";
    else if (employee.getEmployeeType() == "QA")
        badgeClass = "badge-qa";

    QString activeBadge = employee.getIsActive()
                              ? R"(<span class="badge badge-status">Active</span>)"
                              : R"(<span class="badge badge-status" style="background: transparent; color: #000000;">Inactive</span>)";

    html += QString(R"(
        <div class="header">
            <h1>%1 <span class="badge %2">%3</span> %4</h1>
            <div class="header-detail">Position: %5 | Department: %6 | Salary: $%7</div>
        </div>
    )")
                .arg(employee.getName())
                .arg(badgeClass)
                .arg(employee.getEmployeeType())
                .arg(activeBadge)
                .arg(employee.getPosition())
                .arg(employee.getDepartment())
                .arg(employee.getSalary(), 0, 'f', 2);

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Workload Metrics</div>)";
    html += R"(<div class="metrics-grid">)";

    html += QString(R"(
        <div class="metric-card">
            <div class="metric-label">Weekly Capacity</div>
            <div class="metric-value">%1h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Current Hours</div>
            <div class="metric-value">%2h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Available Hours</div>
            <div class="metric-value">%3h</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Utilization</div>
            <div class="metric-value">%4%</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Employment Rate</div>
            <div class="metric-value">%5</div>
        </div>
        <div class="metric-card">
            <div class="metric-label">Active Projects</div>
            <div class="metric-value">%6</div>
        </div>
    )")
                .arg(employee.getWeeklyHoursCapacity())
                .arg(employee.getCurrentWeeklyHours())
                .arg(employee.getAvailableHours())
                .arg(utilization, 0, 'f', 1)
                .arg(employee.getEmploymentRate(), 0, 'f', 2)
                .arg(employeeProjects.size());

    html += R"(</div>)";

    html += QString(R"(
        <div style="margin-top: 15px;">
            <div style="font-size: 13px; color: #666; margin-bottom: 5px;">Weekly Capacity Utilization: %1%</div>
            <div class="progress-bar">
                <div class="progress-fill" style="width: %1%;">%2h / %3h</div>
            </div>
        </div>
    )")
                .arg(utilization, 0, 'f', 1)
                .arg(employee.getCurrentWeeklyHours())
                .arg(employee.getWeeklyHoursCapacity());

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Employee Details</div>)";

    QString employmentRateFormatted = DisplayHelper::formatEmploymentRate(employee.getEmploymentRate());
    employmentRateFormatted.replace(" (Full)", "");
    employmentRateFormatted.replace(" (3/4)", "");
    employmentRateFormatted.replace(" (Half)", "");
    employmentRateFormatted.replace(" (1/4)", "");
    if (employmentRateFormatted.isEmpty()) employmentRateFormatted = "Custom";

    html += QString(R"(
        <div class="info-row">
            <span class="info-label">Employee ID</span>
            <span class="info-value">#%1</span>
        </div>
        <div class="info-row">
            <span class="info-label">Employee Type</span>
            <span class="info-value">%2</span>
        </div>
        <div class="info-row">
            <span class="info-label">Position</span>
            <span class="info-value">%3</span>
        </div>
        <div class="info-row">
            <span class="info-label">Department</span>
            <span class="info-value">%4</span>
        </div>
        <div class="info-row">
            <span class="info-label">Monthly Salary</span>
            <span class="info-value">$%5</span>
        </div>
        <div class="info-row">
            <span class="info-label">Employment Rate</span>
            <span class="info-value">%6 (%7)</span>
        </div>
        <div class="info-row">
            <span class="info-label">Status</span>
            <span class="info-value">%8</span>
        </div>
    )")
                .arg(employee.getId())
                .arg(employee.getEmployeeType())
                .arg(employee.getPosition())
                .arg(employee.getDepartment())
                .arg(employee.getSalary(), 0, 'f', 2)
                .arg(employee.getEmploymentRate(), 0, 'f', 2)
                .arg(employmentRateFormatted)
                .arg(employee.getIsActive() ? "Active" : "Inactive");

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html += QString(R"(<div class="section-title">Project History (%1 Projects)</div>)").arg(employeeProjects.size());

    if (employeeProjects.empty()) {
        html += R"(<div class="empty-state">This employee is not assigned to any projects yet.</div>)";
    } else {
        for (const auto* proj : employeeProjects) {
            auto tasks = company->getProjectTasks(proj->getId());

            int projectEstimated = proj->getEstimatedHours();
            int projectAllocated = proj->getAllocatedHours();
            double projectProgress = projectEstimated > 0
                                         ? (static_cast<double>(projectAllocated) / projectEstimated * 100.0)
                                         : 0.0;

            html += QString(R"(
                <div class="project-item">
                    <div class="project-header">
                        %1 <span class="badge badge-status">%2</span>
                    </div>
                    <div class="project-details">
                        Client: %3 | Budget: $%4 | Employee Costs: $%5 | Hours: %6h estimated, %7h allocated (%8%)
                    </div>
            )")
                        .arg(proj->getName())
                        .arg(proj->getStatus())
                        .arg(proj->getClientName())
                        .arg(proj->getBudget(), 0, 'f', 2)
                        .arg(proj->getEmployeeCosts(), 0, 'f', 2)
                        .arg(projectEstimated)
                        .arg(projectAllocated)
                        .arg(projectProgress, 0, 'f', 1);

            if (!tasks.empty()) {
                html += R"(<div style="margin-top: 12px; padding-top: 12px; border-top: 1px solid #e0e0e0;">)";
                html += R"(<div style="font-size: 13px; color: #666; margin-bottom: 8px; font-weight: 600;">Tasks:</div>)";

                for (const auto& task : tasks) {
                    QString taskBadgeClass = "badge-type";
                    if (task.getType() == "Management")
                        taskBadgeClass = "badge-management";
                    else if (task.getType() == "Development")
                        taskBadgeClass = "badge-development";
                    else if (task.getType() == "Design")
                        taskBadgeClass = "badge-design";
                    else if (task.getType() == "QA")
                        taskBadgeClass = "badge-qa";

                    int taskRemaining = task.getEstimatedHours() - task.getAllocatedHours();
                    double taskProgress = task.getEstimatedHours() > 0
                                              ? (static_cast<double>(task.getAllocatedHours()) /
                                                 task.getEstimatedHours() * 100.0)
                                              : 0.0;

                    html += QString(R"(
                        <div class="task-item">
                            <div class="task-header">
                                %1 <span class="badge %2">%3</span>
                            </div>
                            <div class="task-details">
                                Estimated: %5h | Allocated: %6h | Remaining: %7h | Priority: %8 | Progress: %9% | Status: %4
                            </div>
                        </div>
                    )")
                                .arg(task.getName())
                                .arg(taskBadgeClass)
                                .arg(task.getType())
                                .arg(task.getStatus())
                                .arg(task.getEstimatedHours())
                                .arg(task.getAllocatedHours())
                                .arg(taskRemaining)
                                .arg(task.getPriority())
                                .arg(taskProgress, 0, 'f', 1);
                }

                html += R"(</div>)";
            }

            html += R"(</div>)";
        }
    }

    html += R"(</div>)";
    html += R"(
        </body>
        </html>
    )";

    return html;
}


