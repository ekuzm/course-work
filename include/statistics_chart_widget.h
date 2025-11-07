#pragma once

#include <QTimer>
#include <QWidget>

class Company;

class StatisticsChartWidget : public QWidget {
    Q_OBJECT

   public:
    explicit StatisticsChartWidget(QWidget* parent = nullptr);
    void setData(const Company* company);

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    void drawMainEmployeeSalaryChart(
        class QPainter& painter, int width, int height,
        const std::vector<std::pair<std::shared_ptr<class Employee>, double>>& employeeData,
        double progress);

    const Company* company = nullptr;
    QTimer* animationTimer = nullptr;
    double animationProgress = 0.0;
};


