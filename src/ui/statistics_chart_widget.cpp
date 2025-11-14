#include "ui/statistics_chart_widget.h"

#include <QColor>
#include <QFont>
#include <QLinearGradient>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <algorithm>
#include <map>

#include "entities/company.h"
#include "entities/employee.h"

StatisticsChartWidget::StatisticsChartWidget(QWidget* parent)
    : QWidget(parent), animationProgress(0.0) {
    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        animationProgress += 0.02;
        if (animationProgress >= 1.0) {
            animationProgress = 1.0;
            animationTimer->stop();
        }
        update();
    });
}

void StatisticsChartWidget::setData(const Company* company) {
    this->company = company;
    animationProgress = 0.0;
    animationTimer->start(16);
    update();
}

void StatisticsChartWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    if (company == nullptr) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int width = this->width();
    int height = this->height();

    QLinearGradient bgGradient(0, 0, width, height);
    bgGradient.setColorAt(0, QColor(245, 247, 250));
    bgGradient.setColorAt(0.5, QColor(250, 252, 255));
    bgGradient.setColorAt(1, QColor(255, 255, 255));
    painter.fillRect(0, 0, width, height, bgGradient);

    auto employees = company->getAllEmployees();

    if (employees.empty()) {
        painter.setPen(QPen(QColor(150, 150, 150), 1));
        painter.setFont(QFont("Segoe UI", 16));
        painter.drawText(0, 0, width, height, Qt::AlignCenter,
                         "No employees data available");
        return;
    }

    std::vector<std::pair<std::shared_ptr<Employee>, double>> employeeData;
    for (const auto& emp : employees) {
        if (emp && emp->getIsActive()) {
            employeeData.push_back({emp, emp->getSalary()});
        }
    }

    std::sort(employeeData.begin(), employeeData.end(),
              [](const auto& a, const auto& b) { return a.second < b.second; });

    if (employeeData.empty()) {
        painter.setPen(QPen(QColor(150, 150, 150), 1));
        painter.setFont(QFont("Segoe UI", 16));
        painter.drawText(0, 0, width, height, Qt::AlignCenter,
                         "No active employees");
        return;
    }

    drawMainEmployeeSalaryChart(painter, width, height, employeeData,
                                animationProgress);
}

void StatisticsChartWidget::drawMainEmployeeSalaryChart(
    QPainter& painter, int width, int height,
    const std::vector<std::pair<std::shared_ptr<Employee>, double>>&
        employeeData,
    double progress) {
    int padding = 60;
    int chartX = padding;
    int chartY = padding + 50;
    int chartWidth = width - padding * 2;
    int chartHeight = height - padding * 2 - 50;

    painter.setPen(QPen(QColor(33, 33, 33), 2));
    painter.setFont(QFont("Segoe UI", 20, QFont::Bold));
    painter.drawText(0, 20, width, 40, Qt::AlignCenter,
                     "Employee Salaries Distribution");

    painter.setPen(QPen(QColor(100, 100, 100), 1));
    painter.setFont(QFont("Segoe UI", 11));
    QString subtitle = QString("Total Employees: %1 | Total Salary: $%2")
                           .arg(employeeData.size())
                           .arg(company->getTotalSalaries(), 0, 'f', 2);
    painter.drawText(0, 50, width, 30, Qt::AlignCenter, subtitle);

    double maxSalary = 0;
    for (const auto& [emp, salary] : employeeData) {
        if (salary > maxSalary) maxSalary = salary;
    }
    if (maxSalary == 0) maxSalary = 1;

    painter.setPen(QPen(QColor(230, 230, 230), 1));
    painter.setFont(QFont("Segoe UI", 9));
    int gridLines = 5;
    for (int i = 0; i <= gridLines; i++) {
        double value = maxSalary * (1.0 - static_cast<double>(i) / gridLines);
        int y = chartY + static_cast<int>((static_cast<double>(i) / gridLines) *
                                          chartHeight);
        painter.drawLine(chartX - 10, y, chartX + chartWidth, y);

        painter.setPen(QPen(QColor(120, 120, 120), 1));
        painter.drawText(0, y - 10, chartX - 15, 20,
                         Qt::AlignRight | Qt::AlignVCenter,
                         "$" + QString::number(value, 'f', 0));
        painter.setPen(QPen(QColor(230, 230, 230), 1));
    }

    painter.setPen(QPen(QColor(200, 200, 200), 2));
    painter.drawLine(chartX, chartY, chartX, chartY + chartHeight);

    painter.drawLine(chartX, chartY + chartHeight, chartX + chartWidth,
                     chartY + chartHeight);

    int barCount = static_cast<int>(employeeData.size());
    int barSpacing = 8;
    int availableWidth = chartWidth - 20;
    int minBarWidth = 20;
    int maxBarWidth = 80;
    int barWidth = qBound(
        minBarWidth, (availableWidth - barSpacing * (barCount - 1)) / barCount,
        maxBarWidth);
    int baseY = chartY + chartHeight;

    std::map<QString, QColor> fixedTypeColors = {
        {"Developer", QColor(33, 150, 243)},
        {"Manager", QColor(76, 175, 80)},
        {"Designer", QColor(255, 152, 0)},
        {"QA", QColor(156, 39, 176)},
    };

    QColor defaultColor = QColor(128, 128, 128);

    int currentX = chartX + 10;

    for (size_t i = 0; i < employeeData.size(); i++) {
        const auto& [emp, salary] = employeeData[i];
        QString empType = emp->getEmployeeType();

        QColor baseColor = defaultColor;
        if (fixedTypeColors.find(empType) != fixedTypeColors.end()) {
            baseColor = fixedTypeColors[empType];
        }

        double normalizedSalary = salary / maxSalary;
        int maxBarHeight = chartHeight - 30;
        int targetBarHeight =
            static_cast<int>(normalizedSalary * maxBarHeight * progress);

        QLinearGradient gradient(currentX, baseY - targetBarHeight,
                                 currentX + barWidth, baseY);
        gradient.setColorAt(0, baseColor.lighter(130));
        gradient.setColorAt(0.5, baseColor);
        gradient.setColorAt(1, baseColor.darker(120));

        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(QColor(0, 0, 0, 40)));
        painter.drawRoundedRect(currentX + 3, baseY - targetBarHeight + 3,
                                barWidth, targetBarHeight, 6, 6);

        painter.setBrush(QBrush(gradient));
        painter.setPen(QPen(baseColor.darker(150), 1.5));
        painter.drawRoundedRect(currentX, baseY - targetBarHeight, barWidth,
                                targetBarHeight, 6, 6);

        if (targetBarHeight > 25) {
            painter.setPen(QPen(QColor(255, 255, 255), 1));
            painter.setFont(QFont("Segoe UI", 9, QFont::Bold));
            QString salaryText = "$" + QString::number(salary, 'f', 0);
            painter.drawText(currentX, baseY - targetBarHeight - 18, barWidth,
                             16, Qt::AlignCenter, salaryText);
        }

        painter.setPen(QPen(QColor(66, 66, 66), 1));
        painter.setFont(QFont("Segoe UI", 8));
        QString name = emp->getName();
        if (name.length() > 12) {
            name = name.left(10) + "..";
        }
        QRect nameRect(currentX, baseY + 5, barWidth, 20);
        painter.drawText(nameRect, Qt::AlignCenter | Qt::TextWordWrap, name);

        painter.setPen(QPen(QColor(150, 150, 150), 1));
        painter.setFont(QFont("Segoe UI", 7));
        painter.drawText(currentX, baseY + 25, barWidth, 15, Qt::AlignCenter,
                         "#" + QString::number(emp->getId()));

        currentX += barWidth + barSpacing;
    }

    int legendX = chartX + chartWidth - 150;
    int legendY = chartY + 20;
    painter.setPen(QPen(QColor(33, 33, 33), 1));
    painter.setFont(QFont("Segoe UI", 10, QFont::Bold));
    painter.drawText(legendX, legendY, 150, 20, Qt::AlignLeft,
                     "Employee Types:");

    legendY += 25;
    for (const auto& [type, color] : fixedTypeColors) {
        painter.setBrush(QBrush(color));
        painter.setPen(QPen(color.darker(130), 1));
        painter.drawRect(legendX, legendY, 15, 15);

        painter.setPen(QPen(QColor(66, 66, 66), 1));
        painter.setFont(QFont("Segoe UI", 9));
        painter.drawText(legendX + 20, legendY, 130, 15,
                         Qt::AlignLeft | Qt::AlignVCenter, type);
        legendY += 20;
    }
}
