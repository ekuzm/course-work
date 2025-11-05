#include "main_window.h"

#include <QAbstractItemView>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFormLayout>
#include <QHeaderView>
#include <algorithm>
#include <fstream>
#include <map>
#include <ranges>

#include "derived_employees.h"
#include "display_helper.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    autoLoad();
    initializeCompanySetup();
}

void MainWindow::initializeCompanySetup() {
    DisplayHelper::displayEmployees(employeeTable, currentCompany);
    DisplayHelper::displayProjects(projectTable, currentCompany);
    showCompanyInfo();
    showStatistics();

    if (currentCompany != nullptr) {
        if (auto employees = currentCompany->getAllEmployees();
            !employees.empty()) {
            nextEmployeeId = employees.back()->getId() + 1;
        }
        if (auto projects = currentCompany->getAllProjects();
            !projects.empty()) {
            nextProjectId = projects.back().getId() + 1;
        }
    }
}

MainWindow::~MainWindow() {
    for (auto* company : companies) {
        delete company;
    }
}

void MainWindow::setupUI() {
    setWindowTitle("Information Systems for IT Companies");
    setMinimumSize(900, 700);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
            color: #1a1a1a;
        }
        QTabWidget::pane {
            background-color: #ffffff;
            border: none;
            border-radius: 0px;
            padding: 0px;
        }
        QTabBar::tab {
            background-color: #ffffff;
            color: #1a1a1a;
            padding: 14px 32px;
            margin-right: 0px;
            border-top-left-radius: 0px;
            border-top-right-radius: 0px;
            border: 1px solid #e0e0e0;
            border-bottom: none;
            font-weight: 500;
            font-size: 14px;
            margin-left: 2px;
        }
        QTabBar::tab:selected {
            background-color: #0066cc;
            color: #ffffff;
            font-weight: bold;
            border: 1px solid #0066cc;
            border-bottom: none;
        }
        QTabBar::tab:hover:!selected {
            background-color: #f5f5f5;
            color: #0066cc;
        }
        QPushButton {
            background-color: #0066cc;
            color: white;
            padding: 12px 28px;
            border: none;
            border-radius: 6px;
            font-weight: 600;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #0052a3;
        }
        QPushButton:pressed {
            background-color: #003d7a;
        }
        QLineEdit, QTextEdit, QComboBox, QDateEdit {
            padding: 10px 14px;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            background-color: #ffffff;
            color: #1a1a1a;
            selection-background-color: #0066cc;
            selection-color: white;
            font-size: 14px;
        }
        QLineEdit:focus, QTextEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 2px solid #0066cc;
            background-color: #ffffff;
        }
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #fafafa;
            border: 1px solid #e0e0e0;
            border-radius: 0px;
            gridline-color: #e0e0e0;
            selection-background-color: #0066cc;
            selection-color: white;
        }
        QTableWidget::item {
            padding: 12px;
            color: #1a1a1a;
            border: none;
        }
        QTableWidget::item:selected {
            background-color: #0066cc;
            color: white;
        }
        QTableWidget::item:hover:!selected {
            background-color: #f0f0f0;
        }
        QHeaderView::section {
            background-color: #f8f8f8;
            color: #1a1a1a;
            padding: 14px;
            font-weight: bold;
            font-size: 14px;
            border: none;
            border-right: 1px solid #e0e0e0;
            border-bottom: 2px solid #e0e0e0;
        }
        QHeaderView::section:last {
            border-right: none;
        }
        QLabel {
            color: #1a1a1a;
            font-size: 14px;
        }
        QTextEdit {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 12px;
        }
        QMenuBar {
            background-color: #ffffff;
            border-bottom: 2px solid #e0e0e0;
            color: #1a1a1a;
            padding: 6px;
        }
        QMenuBar::item:selected {
            background-color: #0066cc;
            color: white;
            border-radius: 4px;
        }
        QMenuBar::item {
            padding: 8px 16px;
        }
        QMenu {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 4px;
        }
        QMenu::item:selected {
            background-color: #0066cc;
            color: white;
            border-radius: 4px;
        }
        QMessageBox {
            background-color: #ffffff;
            color: #1a1a1a;
        }
        QMessageBox QLabel {
            color: #1a1a1a;
            padding: 12px;
            font-size: 14px;
        }
        QMessageBox QPushButton {
            min-width: 100px;
        }
        QToolTip {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 1px solid #0066cc;
            border-radius: 6px;
            padding: 8px 12px;
            font-size: 13px;
        }
        QCalendarWidget {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
        }
        QCalendarWidget QAbstractItemView:enabled {
            background-color: #ffffff;
            color: #1a1a1a;
            selection-background-color: #0066cc;
            selection-color: white;
            border-radius: 4px;
        }
        QCalendarWidget QSpinBox {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
        }
        QCalendarWidget QTableView {
            background-color: #ffffff;
            color: #1a1a1a;
            alternate-background-color: #fafafa;
            border: none;
        }
        QCalendarWidget QTableView::item {
            color: #1a1a1a;
            background-color: transparent;
        }
        QCalendarWidget QTableView::item:selected {
            background-color: #0066cc;
            color: white;
            border-radius: 4px;
        }
        QCalendarWidget QTableView::item:hover:!selected {
            background-color: #f0f0f0;
            border-radius: 4px;
        }
    )");

    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);

    fileMenu = menuBar->addMenu("File");
    saveAction = fileMenu->addAction("Save");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);

    auto* companyWidget = new QWidget();
    auto* companyLayout = new QHBoxLayout(companyWidget);
    companyLayout->setContentsMargins(
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins,
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins);

    auto* companyLabel = new QLabel("Current Company:");
    companySelector = new QComboBox();
    companySelector->setMinimumWidth(kCompanySelectorMinWidth);
    companySelector->setStyleSheet(R"(
        QComboBox {
            background-color: #ffffff;
            color: #1a1a1a;
            border: 2px solid #e0e0e0;
            border-radius: 6px;
            padding: 10px 14px;
            font-size: 14px;
        }
        QComboBox:hover {
            border: 2px solid #0066cc;
            background-color: #ffffff;
        }
        QComboBox::drop-down {
            border: none;
            background-color: #ffffff;
        }
        QComboBox::down-arrow {
            image: none;
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #1a1a1a;
            selection-background-color: #0066cc;
            selection-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 4px;
            outline: none;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #f0f0f0;
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: #0066cc;
            color: white;
        }
    )");

    companyAddBtn = new QPushButton("Add");
    companyDeleteBtn = new QPushButton("Delete");

    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(companySelector);
    companyLayout->addWidget(companyAddBtn);
    companyLayout->addWidget(companyDeleteBtn);
    companyLayout->addStretch();

    connect(companySelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::switchCompany);
    connect(companyAddBtn, &QPushButton::clicked, this,
            &MainWindow::addCompany);
    connect(companyDeleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteCompany);

    mainTabWidget = new QTabWidget(this);

    auto* centralWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(companyWidget);
    mainLayout->addWidget(mainTabWidget);

    setCentralWidget(centralWidget);

    setupEmployeeTab();
    setupProjectTab();
    setupCompanyInfoTab();
    setupStatisticsTab();
}

void MainWindow::setupEmployeeTab() {
    employeeTab = new QWidget();

    auto* mainLayout = new QHBoxLayout(employeeTab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* leftPanel = new QWidget();
    leftPanel->setMaximumWidth(220);
    auto* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    employeeAddBtn = new QPushButton("➕ Add");
    employeeAddBtn->setMinimumHeight(50);
    employeeEditBtn = new QPushButton("✏️ Edit");
    employeeEditBtn->setMinimumHeight(50);
    employeeDeleteBtn = new QPushButton("🗑️ Delete");
    employeeDeleteBtn->setMinimumHeight(50);
    employeeHistoryBtn = new QPushButton("📋 History");
    employeeHistoryBtn->setMinimumHeight(50);
    employeeRefreshBtn = new QPushButton("↻ Refresh");
    employeeRefreshBtn->setMinimumHeight(50);

    leftLayout->addWidget(employeeAddBtn);
    leftLayout->addWidget(employeeEditBtn);
    leftLayout->addWidget(employeeDeleteBtn);
    leftLayout->addWidget(employeeHistoryBtn);
    leftLayout->addWidget(employeeRefreshBtn);
    leftLayout->addStretch();

    auto* rightPanel = new QWidget();
    auto* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setSpacing(15);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto* searchLayout = new QHBoxLayout();
    employeeSearchEdit = new QLineEdit();
    employeeSearchEdit->setPlaceholderText("🔍 Search...");
    employeeSearchEdit->setMinimumHeight(40);
    employeeSearchBtn = new QPushButton("Search");
    employeeSearchBtn->setMinimumWidth(100);
    searchLayout->addWidget(employeeSearchEdit);
    searchLayout->addWidget(employeeSearchBtn);
    rightLayout->addLayout(searchLayout);

    employeeTable = new QTableWidget();
    employeeTable->setColumnCount(7);
    QStringList headers = {"ID",   "Name", "Department", "Salary",
                           "Rate", "Type", "Project"};
    employeeTable->setHorizontalHeaderLabels(headers);
    employeeTable->horizontalHeader()->setVisible(true);
    employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    employeeTable->horizontalHeader()->setStretchLastSection(true);
    employeeTable->setAlternatingRowColors(true);
    employeeTable->setShowGrid(true);

    employeeTable->setColumnWidth(0, 60);
    employeeTable->setColumnWidth(1, 160);
    employeeTable->setColumnWidth(2, 140);
    employeeTable->setColumnWidth(3, 120);
    employeeTable->setColumnWidth(4, 120);
    employeeTable->setColumnWidth(5, 180);

    employeeTable->verticalHeader()->setDefaultSectionSize(45);
    employeeTable->verticalHeader()->setVisible(false);

    rightLayout->addWidget(employeeTable);

    mainLayout->addWidget(leftPanel, 0);
    mainLayout->addWidget(rightPanel, 1);

    connect(employeeAddBtn, &QPushButton::clicked, this,
            &MainWindow::addEmployee);
    connect(employeeEditBtn, &QPushButton::clicked, this,
            &MainWindow::editEmployee);
    connect(employeeDeleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteEmployee);
    connect(employeeHistoryBtn, &QPushButton::clicked, this,
            &MainWindow::viewEmployeeHistory);
    connect(employeeRefreshBtn, &QPushButton::clicked, this,
            &MainWindow::refreshEmployeeTable);
    connect(employeeSearchBtn, &QPushButton::clicked, this,
            &MainWindow::searchEmployee);

    mainTabWidget->addTab(employeeTab, "Employees");
}

void MainWindow::setupProjectTab() {
    projectTab = new QWidget();

    auto* mainLayout = new QVBoxLayout(projectTab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* topPanel = new QWidget();
    auto* topLayout = new QVBoxLayout(topPanel);
    topLayout->setSpacing(10);
    topLayout->setContentsMargins(0, 0, 0, 0);

    auto* firstRow = new QHBoxLayout();
    firstRow->setSpacing(15);
    projectAddBtn = new QPushButton("➕ Add");
    projectAddBtn->setMinimumHeight(45);
    projectAddBtn->setMinimumWidth(150);
    projectEditBtn = new QPushButton("✏️ Edit");
    projectEditBtn->setMinimumHeight(45);
    projectEditBtn->setMinimumWidth(150);
    projectDeleteBtn = new QPushButton("🗑️ Delete");
    projectDeleteBtn->setMinimumHeight(45);
    projectDeleteBtn->setMinimumWidth(150);

    firstRow->addStretch();
    firstRow->addWidget(projectAddBtn);
    firstRow->addWidget(projectEditBtn);
    firstRow->addWidget(projectDeleteBtn);
    firstRow->addStretch();

    auto* secondRow = new QHBoxLayout();
    secondRow->setSpacing(15);
    projectAddTaskBtn = new QPushButton("Add Task");
    projectAddTaskBtn->setMinimumHeight(45);
    projectAddTaskBtn->setMinimumWidth(140);
    projectAssignBtn = new QPushButton("Assign to Task");
    projectAssignBtn->setMinimumHeight(45);
    projectAssignBtn->setMinimumWidth(160);
    projectAutoAssignBtn = new QPushButton("Auto Assign");
    projectAutoAssignBtn->setMinimumHeight(45);
    projectAutoAssignBtn->setMinimumWidth(150);

    secondRow->addStretch();
    secondRow->addWidget(projectAddTaskBtn);
    secondRow->addWidget(projectAssignBtn);
    secondRow->addWidget(projectAutoAssignBtn);
    secondRow->addStretch();

    auto* thirdRow = new QHBoxLayout();
    thirdRow->setSpacing(15);
    projectViewAssignmentsBtn = new QPushButton("📊 View Team");
    projectViewAssignmentsBtn->setMinimumHeight(45);
    projectViewAssignmentsBtn->setMinimumWidth(200);

    thirdRow->addStretch();
    thirdRow->addWidget(projectViewAssignmentsBtn);
    thirdRow->addStretch();

    topLayout->addLayout(firstRow);
    topLayout->addLayout(secondRow);
    topLayout->addLayout(thirdRow);

    mainLayout->addWidget(topPanel);

    auto* searchLayout = new QHBoxLayout();
    projectSearchEdit = new QLineEdit();
    projectSearchEdit->setPlaceholderText("🔍 Search projects...");
    projectSearchEdit->setMinimumHeight(40);
    projectSearchBtn = new QPushButton("Search");
    projectSearchBtn->setMinimumWidth(120);
    searchLayout->addWidget(projectSearchEdit);
    searchLayout->addWidget(projectSearchBtn);
    mainLayout->addLayout(searchLayout);

    projectTable = new QTableWidget();
    projectTable->setColumnCount(7);
    QStringList headers = {"ID",         "Name",         "Status", "Budget",
                           "Est. Hours", "Alloc. Hours", "Client"};
    projectTable->setHorizontalHeaderLabels(headers);
    projectTable->horizontalHeader()->setVisible(true);
    projectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    projectTable->setSelectionMode(QAbstractItemView::SingleSelection);
    projectTable->horizontalHeader()->setStretchLastSection(true);
    projectTable->setAlternatingRowColors(true);

    projectTable->setColumnWidth(0, 60);
    projectTable->setColumnWidth(1, 220);
    projectTable->setColumnWidth(2, 120);
    projectTable->setColumnWidth(3, 120);
    projectTable->setColumnWidth(4, 110);
    projectTable->setColumnWidth(5, 110);
    projectTable->setColumnWidth(6, 180);

    projectTable->verticalHeader()->setDefaultSectionSize(45);
    projectTable->verticalHeader()->setVisible(false);
    projectTable->setShowGrid(true);

    mainLayout->addWidget(projectTable);

    connect(projectAddBtn, &QPushButton::clicked, this,
            &MainWindow::addProject);
    connect(projectEditBtn, &QPushButton::clicked, this,
            &MainWindow::editProject);
    connect(projectDeleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteProject);
    connect(projectAddTaskBtn, &QPushButton::clicked, this,
            &MainWindow::addProjectTask);
    connect(projectAssignBtn, &QPushButton::clicked, this,
            &MainWindow::assignEmployeeToTask);
    connect(projectAutoAssignBtn, &QPushButton::clicked, this,
            &MainWindow::autoAssignToProject);
    connect(projectViewAssignmentsBtn, &QPushButton::clicked, this,
            &MainWindow::viewProjectAssignments);
    connect(projectSearchBtn, &QPushButton::clicked, this,
            &MainWindow::searchProject);

    mainTabWidget->addTab(projectTab, "Projects");
}

void MainWindow::setupCompanyInfoTab() {
    infoTab = new QWidget();

    auto* layout = new QVBoxLayout(infoTab);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(20);

    auto* titleLabel = new QLabel("📋 Company Information");
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; padding: 10px;");
    layout->addWidget(titleLabel);

    companyInfoText = new QTextEdit();
    companyInfoText->setReadOnly(true);
    companyInfoText->setStyleSheet(
        "font-size: 15px; line-height: 1.8; padding: 20px; border-radius: "
        "10px;");
    layout->addWidget(companyInfoText);

    mainTabWidget->addTab(infoTab, "Company Info");
}

void MainWindow::setupStatisticsTab() {
    statsTab = new QWidget();

    auto* layout = new QVBoxLayout(statsTab);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(25);

    auto* topBar = new QHBoxLayout();
    auto* titleLabel = new QLabel("📊 Company Statistics");
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; padding: 10px; color: #000000;");

    refreshStatsBtn = new QPushButton("↻ Refresh Statistics");
    refreshStatsBtn->setMinimumHeight(45);
    refreshStatsBtn->setMinimumWidth(200);

    topBar->addWidget(titleLabel);
    topBar->addStretch();
    topBar->addWidget(refreshStatsBtn);
    layout->addLayout(topBar);

    statisticsText = new QTextEdit();
    statisticsText->setReadOnly(true);
    statisticsText->setStyleSheet(
        "font-size: 15px; line-height: 2.0; padding: 25px; border-radius: "
        "10px;");
    layout->addWidget(statisticsText, 1);

    connect(refreshStatsBtn, &QPushButton::clicked, this,
            &MainWindow::showStatistics);

    mainTabWidget->addTab(statsTab, "Statistics");
}

int MainWindow::getSelectedEmployeeId() const {
    if (int rowIndex = employeeTable->currentRow(); rowIndex >= 0) {
        if (const QTableWidgetItem* tableItem =
                employeeTable->item(rowIndex, 0);
            tableItem != nullptr) {
            bool conversionSuccess = false;
            int employeeId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? employeeId : -1;
        }
    }
    return -1;
}

int MainWindow::getSelectedProjectId() const {
    if (int rowIndex = projectTable->currentRow(); rowIndex >= 0) {
        if (const QTableWidgetItem* tableItem = projectTable->item(rowIndex, 0);
            tableItem != nullptr) {
            bool conversionSuccess = false;
            int projectId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? projectId : -1;
        }
    }
    return -1;
}

bool MainWindow::checkDuplicateProjectOnEdit(const QString& projectName,
                                             int excludeId,
                                             const Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingProjects = currentCompany->getAllProjects();
    auto duplicateFound = std::ranges::any_of(
        existingProjects, [&projectName, excludeId](const auto& project) {
            return project.getId() != excludeId &&
                   project.getName().toLower() == projectName.toLower();
        });
    return !duplicateFound;
}

void MainWindow::addEmployee() {
    if (currentCompany == nullptr) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);
    QComboBox* typeCombo = nullptr;
    QLineEdit* nameEdit = nullptr;
    QLineEdit* salaryEdit = nullptr;
    QLineEdit* deptEdit = nullptr;
    QComboBox* employmentRateCombo = nullptr;
    QComboBox* managerProject = nullptr;
    QLineEdit* devLanguage = nullptr;
    QLineEdit* devExperience = nullptr;
    QLineEdit* designerTool = nullptr;
    QLineEdit* designerProjects = nullptr;
    QLineEdit* qaTestType = nullptr;
    QLineEdit* qaBugs = nullptr;
    QLabel* managerProjectLabel = nullptr;
    QLabel* devLanguageLabel = nullptr;
    QLabel* devExperienceLabel = nullptr;
    QLabel* designerToolLabel = nullptr;
    QLabel* designerProjectsLabel = nullptr;
    QLabel* qaTestTypeLabel = nullptr;
    QLabel* qaBugsLabel = nullptr;

    EmployeeDialogHelper::createEmployeeDialog(dialog, form, typeCombo, nameEdit,
                                               salaryEdit, deptEdit, employmentRateCombo,
                                               managerProject, devLanguage, devExperience,
                                               designerTool, designerProjects, qaTestType,
                                               qaBugs, managerProjectLabel, devLanguageLabel,
                                               devExperienceLabel, designerToolLabel,
                                               designerProjectsLabel, qaTestTypeLabel, qaBugsLabel);

    auto projects = currentCompany->getAllProjects();
    managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        managerProject->addItem(proj.getName(), proj.getId());
    }

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString name = nameEdit->text().trimmed();
            QString salaryText = salaryEdit->text().trimmed();
            QString department = deptEdit->text().trimmed();

            if (name.isEmpty()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Employee name cannot be empty!\n\n"
                    "Please enter a valid name for the employee.");
                return;
            }

            bool salaryConversionSuccess = false;
            double salary = salaryText.toDouble(&salaryConversionSuccess);
            if (!salaryConversionSuccess) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid salary format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         salaryText +
                                         "\"\n"
                                         "Valid range: $" +
                                         QString::number(kMinSalary) + " to $" +
                                         QString::number(kMaxSalary));
                return;
            }

            if (salary < kMinSalary || salary > kMaxSalary) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Salary out of valid range!\n\n"
                                     "Current value: $" +
                                         QString::number(salary, 'f', 2) +
                                         "\n"
                                         "Valid range: $" +
                                         QString::number(kMinSalary) + " to $" +
                                         QString::number(kMaxSalary));
                return;
            }

            if (department.isEmpty()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Department cannot be empty!\n\n"
                    "Please enter a department name for the employee.");
                return;
            }

            if (!EmployeeDialogHelper::checkDuplicateEmployee(name,
                                                              currentCompany)) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "Employee with this name already exists!\n\n"
                    "Employee name: \"" +
                        name +
                        "\"\n"
                        "Please choose a different name or edit the existing "
                        "employee.");
                return;
            }

            QString employeeType = typeCombo->currentText();

            if (employeeType == "Developer") {
                QString language = devLanguage->text().trimmed();
                if (language.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Programming language cannot be empty!\n\n"
                        "Please enter a programming language for the "
                        "developer.");
                    return;
                }

                bool conversionSuccess = false;
                QString experienceText =
                    devExperience->text().trimmed();
                int years = experienceText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Invalid years of experience format!\n\n"
                        "Please enter a valid number.\n"
                        "Current value: \"" +
                            experienceText +
                            "\"\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxYearsOfExperience) + " years");
                    return;
                }
                if (years < 0 || years > kMaxYearsOfExperience) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Years of experience out of valid range!\n\n"
                        "Current value: " +
                            QString::number(years) +
                            " years\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxYearsOfExperience) + " years");
                    return;
                }
            } else if (employeeType == "Designer") {
                QString tool = designerTool->text().trimmed();
                if (tool.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Design tool cannot be empty!\n\n"
                        "Please enter a design tool for the designer.");
                    return;
                }

                bool conversionSuccess = false;
                QString projectsText =
                    designerProjects->text().trimmed();
                int projects = projectsText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Invalid number of projects format!\n\n"
                        "Please enter a valid number.\n"
                        "Current value: \"" +
                            projectsText +
                            "\"\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxNumberOfProjects));
                    return;
                }
                if (projects < 0 || projects > kMaxNumberOfProjects) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Number of projects out of valid range!\n\n"
                        "Current value: " +
                            QString::number(projects) +
                            "\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxNumberOfProjects));
                    return;
                }
            } else if (employeeType == "QA") {
                QString testType = qaTestType->text().trimmed();
                if (testType.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Testing type cannot be empty!\n\n"
                        "Please enter a testing type for the QA employee.");
                    return;
                }

                bool conversionSuccess = false;
                QString bugsText = qaBugs->text().trimmed();
                int bugs = bugsText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(&dialog, "Validation Error",
                                         "Invalid bugs found format!\n\n"
                                         "Please enter a valid number.\n"
                                         "Current value: \"" +
                                             bugsText +
                                             "\"\n"
                                             "Valid range: 0 to " +
                                             QString::number(kMaxBugsFound));
                    return;
                }
                if (bugs < 0 || bugs > kMaxBugsFound) {
                    QMessageBox::warning(&dialog, "Validation Error",
                                         "Bugs found out of valid range!\n\n"
                                         "Current value: " +
                                             QString::number(bugs) +
                                             "\n"
                                             "Valid range: 0 to " +
                                             QString::number(kMaxBugsFound));
                    return;
                }
            }

            int employeeId = nextEmployeeId;
            nextEmployeeId++;

            auto employee = EmployeeDialogHelper::createEmployeeFromType(
                employeeType, employeeId, name, salary, department,
                employmentRateCombo, managerProject, devLanguage, devExperience,
                designerTool, designerProjects, qaTestType, qaBugs);
            if (employee == nullptr) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Failed to create employee!\n\n"
                                     "Please check that all required fields "
                                     "are filled correctly.");
                return;
            }

            currentCompany->addEmployee(employee);
            DisplayHelper::displayEmployees(employeeTable, currentCompany);
            if (auto employees = currentCompany->getAllEmployees();
                !employees.empty()) {
                nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(&dialog, "Success",
                                     "Employee added successfully!\n\n"
                                     "Name: " +
                                         name +
                                         "\n"
                                         "Type: " +
                                         employeeType +
                                         "\n"
                                         "Salary: $" +
                                         QString::number(salary, 'f', 2));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                "Failed to add employee!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                &dialog, "Save Error",
                " Failed to save data to file!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "The employee may have been added but the data could not "
                    "be saved.\n"
                    "Please check file permissions and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::editEmployee() {
    if (currentCompany == nullptr) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to edit.");
        return;
    }

    auto employee = currentCompany->getEmployee(employeeId);
    if (employee == nullptr) {
        QMessageBox::warning(this, "Error", "Employee not found!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);
    QString currentType = employee->getEmployeeType();
    QLineEdit* nameEdit = nullptr;
    QLineEdit* salaryEdit = nullptr;
    QLineEdit* deptEdit = nullptr;
    QComboBox* employmentRateCombo = nullptr;
    QComboBox* managerProject = nullptr;
    QLineEdit* devLanguage = nullptr;
    QLineEdit* devExperience = nullptr;
    QLineEdit* designerTool = nullptr;
    QLineEdit* designerProjects = nullptr;
    QLineEdit* qaTestType = nullptr;
    QLineEdit* qaBugs = nullptr;
    QLabel* managerProjectLabel = nullptr;
    QLabel* devLanguageLabel = nullptr;
    QLabel* devExperienceLabel = nullptr;
    QLabel* designerToolLabel = nullptr;
    QLabel* designerProjectsLabel = nullptr;
    QLabel* qaTestTypeLabel = nullptr;
    QLabel* qaBugsLabel = nullptr;

    EmployeeDialogHelper::createEditEmployeeDialog(dialog, form, employee,
                                                   nameEdit, salaryEdit, deptEdit,
                                                   employmentRateCombo, managerProject,
                                                   devLanguage, devExperience,
                                                   designerTool, designerProjects,
                                                   qaTestType, qaBugs,
                                                   managerProjectLabel, devLanguageLabel,
                                                   devExperienceLabel, designerToolLabel,
                                                   designerProjectsLabel, qaTestTypeLabel, qaBugsLabel);

    auto projects = currentCompany->getAllProjects();
    managerProject->addItem("(No Project)", -1);
    for (const auto& proj : projects) {
        managerProject->addItem(proj.getName(), proj.getId());
    }

    if (const auto* manager = dynamic_cast<const Manager*>(employee.get())) {
        int currentProjectId = manager->getManagedProjectId();
        int index = managerProject->findData(currentProjectId);
        if (index >= 0) {
            managerProject->setCurrentIndex(index);
        }
    }

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString name = nameEdit->text().trimmed();
            QString salaryText = salaryEdit->text().trimmed();
            QString department = deptEdit->text().trimmed();

            if (name.isEmpty()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Employee name cannot be empty!\n\n"
                    "Please enter a valid name for the employee.");
                return;
            }

            bool salaryConversionSuccess = false;
            double salary = salaryText.toDouble(&salaryConversionSuccess);
            if (!salaryConversionSuccess) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid salary format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         salaryText +
                                         "\"\n"
                                         "Valid range: $" +
                                         QString::number(kMinSalary) + " to $" +
                                         QString::number(kMaxSalary));
                return;
            }

            if (salary < kMinSalary || salary > kMaxSalary) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Salary out of valid range!\n\n"
                                     "Current value: $" +
                                         QString::number(salary, 'f', 2) +
                                         "\n"
                                         "Valid range: $" +
                                         QString::number(kMinSalary) + " to $" +
                                         QString::number(kMaxSalary));
                return;
            }

            if (department.isEmpty()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Department cannot be empty!\n\n"
                    "Please enter a department name for the employee.");
                return;
            }

            if (!EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(
                    name, employeeId, currentCompany)) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "Employee with this name already exists!\n\n"
                    "Employee name: \"" +
                        name +
                        "\"\n"
                        "Employee ID: " +
                        QString::number(employeeId) +
                        "\n"
                        "Please choose a different name or edit the existing "
                        "employee.");
                return;
            }

            if (currentType == "Developer") {
                QString language = devLanguage->text().trimmed();
                if (language.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Programming language cannot be empty!\n\n"
                        "Please enter a programming language for the "
                        "developer.");
                    return;
                }

                bool conversionSuccess = false;
                QString experienceText =
                    devExperience->text().trimmed();
                int years = experienceText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Invalid years of experience format!\n\n"
                        "Please enter a valid number.\n"
                        "Current value: \"" +
                            experienceText +
                            "\"\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxYearsOfExperience) + " years");
                    return;
                }
                if (years < 0 || years > kMaxYearsOfExperience) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Years of experience out of valid range!\n\n"
                        "Current value: " +
                            QString::number(years) +
                            " years\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxYearsOfExperience) + " years");
                    return;
                }
            } else if (currentType == "Designer") {
                QString tool = designerTool->text().trimmed();
                if (tool.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Design tool cannot be empty!\n\n"
                        "Please enter a design tool for the designer.");
                    return;
                }

                bool conversionSuccess = false;
                QString projectsText =
                    designerProjects->text().trimmed();
                int projects = projectsText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Invalid number of projects format!\n\n"
                        "Please enter a valid number.\n"
                        "Current value: \"" +
                            projectsText +
                            "\"\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxNumberOfProjects));
                    return;
                }
                if (projects < 0 || projects > kMaxNumberOfProjects) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Number of projects out of valid range!\n\n"
                        "Current value: " +
                            QString::number(projects) +
                            "\n"
                            "Valid range: 0 to " +
                            QString::number(kMaxNumberOfProjects));
                    return;
                }
            } else if (currentType == "QA") {
                QString testType = qaTestType->text().trimmed();
                if (testType.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Testing type cannot be empty!\n\n"
                        "Please enter a testing type for the QA employee.");
                    return;
                }

                bool conversionSuccess = false;
                QString bugsText = qaBugs->text().trimmed();
                int bugs = bugsText.toInt(&conversionSuccess);
                if (!conversionSuccess) {
                    QMessageBox::warning(&dialog, "Validation Error",
                                         "Invalid bugs found format!\n\n"
                                         "Please enter a valid number.\n"
                                         "Current value: \"" +
                                             bugsText +
                                             "\"\n"
                                             "Valid range: 0 to " +
                                             QString::number(kMaxBugsFound));
                    return;
                }
                if (bugs < 0 || bugs > kMaxBugsFound) {
                    QMessageBox::warning(&dialog, "Validation Error",
                                         "Bugs found out of valid range!\n\n"
                                         "Current value: " +
                                             QString::number(bugs) +
                                             "\n"
                                             "Valid range: 0 to " +
                                             QString::number(kMaxBugsFound));
                    return;
                }
            }

            auto updatedEmployee = EmployeeDialogHelper::createEmployeeFromType(
                currentType, employeeId, name, salary, department,
                employmentRateCombo, managerProject, devLanguage, devExperience,
                designerTool, designerProjects, qaTestType, qaBugs);
            if (updatedEmployee == nullptr) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Failed to update employee!\n\n"
                                     "Please check that all required fields "
                                     "are filled correctly.");
                return;
            }

            currentCompany->removeEmployee(employeeId);
            currentCompany->addEmployee(updatedEmployee);

            DisplayHelper::displayEmployees(employeeTable, currentCompany);
            if (auto employees = currentCompany->getAllEmployees();
                !employees.empty()) {
                nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(&dialog, "Success",
                                     " Employee updated successfully!\n\n"
                                     "Name: " +
                                         name +
                                         "\n"
                                         "Type: " +
                                         currentType +
                                         "\n"
                                         "Salary: $" +
                                         QString::number(salary, 'f', 2));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                " Failed to edit employee!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                &dialog, "Save Error",
                " Failed to save data to file!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "The employee may have been edited but the data could not "
                    "be saved.\n"
                    "Please check file permissions and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::deleteEmployee() {
    if (currentCompany == nullptr) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to delete.");
        return;
    }

    int userChoice =
        QMessageBox::question(this, "Confirm Delete",
                              "Are you sure you want to delete this employee?",
                              QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            currentCompany->removeEmployee(employeeId);
            DisplayHelper::displayEmployees(employeeTable, currentCompany);
            if (auto employees = currentCompany->getAllEmployees();
                !employees.empty()) {
                nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(this, "Success",
                                     "Employee deleted successfully!");

        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::searchEmployee() const {
    if (currentCompany == nullptr) return;

    QString searchTerm = employeeSearchEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        DisplayHelper::displayEmployees(employeeTable, currentCompany);
        return;
    }

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(0);
    int rowIndex = 0;

    for (const auto& employee : employees) {
        if (employee->getName().toLower().contains(searchTerm) ||
            employee->getDepartment().toLower().contains(searchTerm) ||
            employee->getPosition().toLower().contains(searchTerm)) {
            employeeTable->insertRow(rowIndex);
            employeeTable->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(employee->getId())));
            employeeTable->setItem(rowIndex, 1,
                                   new QTableWidgetItem(employee->getName()));
            employeeTable->setItem(
                rowIndex, 2, new QTableWidgetItem(employee->getDepartment()));
            employeeTable->setItem(rowIndex, 3,
                                   new QTableWidgetItem(QString::number(
                                       employee->getSalary(), 'f', 2)));
            employeeTable->setItem(
                rowIndex, 4, new QTableWidgetItem(employee->getEmployeeType()));

            QString projectInfo =
                DisplayHelper::formatProjectInfo(employee, currentCompany);
            employeeTable->setItem(rowIndex, 5,
                                   new QTableWidgetItem(projectInfo));
            rowIndex++;
        }
    }
}

void MainWindow::addProject() {
    if (currentCompany == nullptr) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);

    auto* projectTypeCombo = new QComboBox();
    projectTypeCombo->addItems({"Web Development", "Mobile App",
                                "Software Product", "Consulting", "Other"});
    projectTypeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Project Type:", projectTypeCombo);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., E-commerce Platform");
    form->addRow("Project Name:", nameEdit);

    auto* descEdit = new QTextEdit();
    descEdit->setMaximumHeight(kDescEditMaxHeight);
    descEdit->setPlaceholderText("Brief description of the project...");
    form->addRow("Description:", descEdit);

    auto* statusCombo = new QComboBox();
    statusCombo->addItems({"Analysis", "Planning", "Design", "Development",
                           "Testing", "Deployment", "Maintenance",
                           "Completed"});
    statusCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Status:", statusCombo);

    auto* startDate = new QDateEdit();
    startDate->setDate(QDate::currentDate());
    startDate->setCalendarPopup(true);
    startDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("Start Date:", startDate);

    auto* endDate = new QDateEdit();
    endDate->setDate(QDate::currentDate().addDays(kDefaultProjectEndDateDays));
    endDate->setCalendarPopup(true);
    endDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("End Date:", endDate);

    auto* budgetEdit = new QLineEdit();
    budgetEdit->setPlaceholderText("e.g., 50000");
    form->addRow("Budget ($):", budgetEdit);

    auto* estimatedHoursEdit = new QLineEdit();
    estimatedHoursEdit->setPlaceholderText("e.g., 160 (hours)");
    form->addRow("Estimated Hours:", estimatedHoursEdit);

    auto* clientNameEdit = new QLineEdit();
    clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
    auto* clientNameLabel = new QLabel("Client Name:");
    form->addRow(clientNameLabel, clientNameEdit);

    auto updateClientFieldAvailability = [clientNameEdit, clientNameLabel,
                                          statusCombo]() {
        QString currentStatus = statusCombo->currentText();
        bool isAnalysisPhase = (currentStatus == "Analysis");

        if (!isAnalysisPhase) {
            clientNameEdit->setVisible(false);
            clientNameLabel->setVisible(false);
            clientNameEdit->clear();
        } else {
            clientNameEdit->setVisible(true);
            clientNameLabel->setVisible(true);
            clientNameEdit->setEnabled(true);
            clientNameLabel->setEnabled(true);
            clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
            clientNameLabel->setText("Client Name:");
        }
    };

    updateClientFieldAvailability();

    connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [updateClientFieldAvailability](int) {
                updateClientFieldAvailability();
            });

    auto* clientIndustryEdit = new QLineEdit();
    clientIndustryEdit->setPlaceholderText("e.g., Retail, Finance");
    auto* clientIndustryLabel = new QLabel("Client Industry:");
    form->addRow(clientIndustryLabel, clientIndustryEdit);
    clientIndustryLabel->setVisible(false);
    clientIndustryEdit->setVisible(false);

    auto* clientContactEdit = new QLineEdit();
    clientContactEdit->setPlaceholderText("e.g., john@company.com");
    auto* clientContactLabel = new QLabel("Client Contact:");
    form->addRow(clientContactLabel, clientContactEdit);
    clientContactLabel->setVisible(false);
    clientContactEdit->setVisible(false);

    connect(projectTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [clientIndustryLabel, clientIndustryEdit, clientContactLabel,
             clientContactEdit](int index) {
                constexpr int CONSULTING_INDEX = 3;
                constexpr int OTHER_INDEX = 4;
                bool showDetails =
                    (index == CONSULTING_INDEX || index == OTHER_INDEX);
                clientIndustryLabel->setVisible(showDetails);
                clientIndustryEdit->setVisible(showDetails);
                clientContactLabel->setVisible(showDetails);
                clientContactEdit->setVisible(showDetails);
            });

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString projectName = nameEdit->text().trimmed();
            if (projectName.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Project name cannot be empty!\n\n"
                                     "Please enter a name for the project.");
                return;
            }

            bool conversionSuccess = false;
            QString budgetText = budgetEdit->text().trimmed();
            double projectBudget = budgetText.toDouble(&conversionSuccess);
            if (!conversionSuccess) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid budget format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         budgetText +
                                         "\"\n"
                                         "Valid range: $0 to $" +
                                         QString::number(kMaxBudget, 'f', 0));
                return;
            }

            if (projectBudget < 0 || projectBudget > kMaxBudget) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Budget out of valid range!\n\n"
                    "Current value: $" +
                        QString::number(projectBudget, 'f', 2) +
                        "\n"
                        "Valid range: $0 to $" +
                        QString::number(kMaxBudget, 'f', 0));
                return;
            }

            QString selectedStatus = statusCombo->currentText();
            QString clientName = clientNameEdit->text().trimmed();

            if (selectedStatus == "Analysis") {
                if (clientName.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Client name cannot be empty in Analysis phase!\n\n"
                        "When project status is 'Analysis', you must provide a "
                        "client name.");
                    return;
                }
            } else {
                if (!clientName.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Client can only be added during Analysis phase!\n\n"
                        "Current status: " +
                            selectedStatus +
                            "\n"
                            "Current client name: \"" +
                            clientName +
                            "\"\n\n"
                            "Please change project status to 'Analysis' or "
                            "clear the client name.");
                    return;
                }
                clientName = "";
            }

            if (endDate->date() < startDate->date()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "End date cannot be before start date!\n\n"
                    "Start date: " +
                        startDate->date().toString("yyyy-MM-dd") +
                        "\n"
                        "End date: " +
                        endDate->date().toString("yyyy-MM-dd") +
                        "\n\n"
                        "Please adjust the dates.");
                return;
            }

            QString estimatedHoursText = estimatedHoursEdit->text().trimmed();
            int estimatedHours = estimatedHoursText.toInt(&conversionSuccess);
            if (!conversionSuccess || estimatedHoursText.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid estimated hours format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         estimatedHoursText +
                                         "\"\n"
                                         "Valid range: 0 to " +
                                         QString::number(kMaxEstimatedHours) +
                                         " hours");
                return;
            }
            if (estimatedHours < 0) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Estimated hours cannot be negative!\n\n"
                                     "Current value: " +
                                         QString::number(estimatedHours) +
                                         " hours\n"
                                         "Valid range: 0 to " +
                                         QString::number(kMaxEstimatedHours) +
                                         " hours");
                return;
            }
            if (estimatedHours > kMaxEstimatedHours) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Estimated hours exceeds maximum allowed!\n\n"
                    "Current value: " +
                        QString::number(estimatedHours) +
                        " hours\n"
                        "Maximum allowed: " +
                        QString::number(kMaxEstimatedHours) +
                        " hours\n"
                        "Valid range: 0 to " +
                        QString::number(kMaxEstimatedHours) + " hours");
                return;
            }

            auto existingProjects = currentCompany->getAllProjects();
            for (const auto& project : existingProjects) {
                if (project.getName().toLower() == projectName.toLower()) {
                    QMessageBox::warning(
                        &dialog, "Duplicate Error",
                        "A project with this name already exists!\n\n"
                        "Project name: \"" +
                            projectName +
                            "\"\n"
                            "Please choose a different name.");
                    return;
                }
            }

            int projectId = nextProjectId;
            nextProjectId++;
            Project project(projectId, projectName,
                            descEdit->toPlainText().trimmed(), selectedStatus,
                            startDate->date(), endDate->date(), projectBudget,
                            clientName, estimatedHours);

            currentCompany->addProject(project);
            DisplayHelper::displayProjects(projectTable, currentCompany);
            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(
                &dialog, "Success",
                "Project added successfully!\n\n"
                "Name: " +
                    projectName +
                    "\n"
                    "Status: " +
                    selectedStatus +
                    "\n"
                    "Budget: $" +
                    QString::number(projectBudget, 'f', 2));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                "Failed to add project!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                &dialog, "Save Error",
                "Failed to save data to file!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "The project may have been added but the data could not be "
                    "saved.\n"
                    "Please check file permissions and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::editProject() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project to edit.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (project == nullptr) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");
    dialog.setMinimumWidth(400);

    auto* form = new QFormLayout(&dialog);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., E-commerce Platform");
    nameEdit->setText(project->getName());
    form->addRow("Project Name:", nameEdit);

    auto* descEdit = new QTextEdit();
    descEdit->setMaximumHeight(kDescEditMaxHeight);
    descEdit->setPlaceholderText("Brief description of the project...");
    descEdit->setPlainText(project->getDescription());
    form->addRow("Description:", descEdit);

    auto* statusCombo = new QComboBox();
    statusCombo->addItems({"Analysis", "Planning", "Design", "Development",
                           "Testing", "Deployment", "Maintenance",
                           "Completed"});
    statusCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Status:", statusCombo);

    if (int statusIndex = statusCombo->findText(project->getStatus());
        statusIndex >= 0)
        statusCombo->setCurrentIndex(statusIndex);

    auto* startDate = new QDateEdit();
    startDate->setDate(project->getStartDate());
    startDate->setCalendarPopup(true);
    startDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("Start Date:", startDate);

    auto* endDate = new QDateEdit();
    endDate->setDate(project->getEndDate());
    endDate->setCalendarPopup(true);
    endDate->setMinimumWidth(kDateEditMinWidth);
    form->addRow("End Date:", endDate);

    auto* budgetEdit = new QLineEdit();
    budgetEdit->setPlaceholderText("e.g., 50000");
    budgetEdit->setText(QString::number(project->getBudget(), 'f', 2));
    form->addRow("Budget ($):", budgetEdit);

    auto* estimatedHoursEdit = new QLineEdit();
    estimatedHoursEdit->setPlaceholderText("e.g., 160 (hours)");
    estimatedHoursEdit->setText(QString::number(project->getEstimatedHours()));
    form->addRow("Estimated Hours:", estimatedHoursEdit);

    auto* clientNameEdit = new QLineEdit();
    clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
    clientNameEdit->setText(project->getClientName());
    auto* clientNameLabel = new QLabel("Client Name:");
    form->addRow(clientNameLabel, clientNameEdit);

    auto updateClientFieldAvailabilityEdit = [clientNameEdit, clientNameLabel,
                                              statusCombo]() {
        QString currentStatus = statusCombo->currentText();
        bool isAnalysisPhase = (currentStatus == "Analysis");

        if (!isAnalysisPhase) {
            clientNameEdit->setVisible(false);
            clientNameLabel->setVisible(false);
            clientNameEdit->clear();
        } else {
            clientNameEdit->setVisible(true);
            clientNameLabel->setVisible(true);
            clientNameEdit->setEnabled(true);
            clientNameLabel->setEnabled(true);
            clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
            clientNameLabel->setText("Client Name:");
        }
    };

    updateClientFieldAvailabilityEdit();

    connect(statusCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [updateClientFieldAvailabilityEdit](int) {
                updateClientFieldAvailabilityEdit();
            });

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString projectName = nameEdit->text().trimmed();
            if (projectName.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Project name cannot be empty!\n\n"
                                     "Please enter a name for the project.");
                return;
            }

            bool conversionSuccess = false;
            QString budgetText = budgetEdit->text().trimmed();
            double projectBudget = budgetText.toDouble(&conversionSuccess);
            if (!conversionSuccess) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid budget format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         budgetText +
                                         "\"\n"
                                         "Valid range: $0 to $" +
                                         QString::number(kMaxBudget, 'f', 0));
                return;
            }

            if (projectBudget < 0 || projectBudget > kMaxBudget) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Budget out of valid range!\n\n"
                    "Current value: $" +
                        QString::number(projectBudget, 'f', 2) +
                        "\n"
                        "Valid range: $0 to $" +
                        QString::number(kMaxBudget, 'f', 0));
                return;
            }

            QString selectedStatus = statusCombo->currentText();
            QString clientName = clientNameEdit->text().trimmed();

            if (selectedStatus == "Analysis") {
                if (clientName.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Client name cannot be empty in Analysis phase!\n\n"
                        "When project status is 'Analysis', you must provide a "
                        "client name.");
                    return;
                }
            } else {
                if (!clientName.isEmpty()) {
                    QMessageBox::warning(
                        &dialog, "Validation Error",
                        "Client can only be added during Analysis phase!\n\n"
                        "Current status: " +
                            selectedStatus +
                            "\n"
                            "Current client name: \"" +
                            clientName +
                            "\"\n\n"
                            "Please change project status to 'Analysis' or "
                            "clear the client name.");
                    return;
                }
                clientName = "";
            }

            if (endDate->date() < startDate->date()) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "End date cannot be before start date!\n\n"
                    "Start date: " +
                        startDate->date().toString("yyyy-MM-dd") +
                        "\n"
                        "End date: " +
                        endDate->date().toString("yyyy-MM-dd") +
                        "\n\n"
                        "Please adjust the dates.");
                return;
            }

            QString estimatedHoursText = estimatedHoursEdit->text().trimmed();
            int estimatedHours = estimatedHoursText.toInt(&conversionSuccess);
            if (!conversionSuccess || estimatedHoursText.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid estimated hours format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         estimatedHoursText +
                                         "\"\n"
                                         "Valid range: 0 to " +
                                         QString::number(kMaxEstimatedHours) +
                                         " hours");
                return;
            }
            if (estimatedHours < 0) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Estimated hours cannot be negative!\n\n"
                                     "Current value: " +
                                         QString::number(estimatedHours) +
                                         " hours\n"
                                         "Valid range: 0 to " +
                                         QString::number(kMaxEstimatedHours) +
                                         " hours");
                return;
            }
            if (estimatedHours > kMaxEstimatedHours) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Estimated hours exceeds maximum allowed!\n\n"
                    "Current value: " +
                        QString::number(estimatedHours) +
                        " hours\n"
                        "Maximum allowed: " +
                        QString::number(kMaxEstimatedHours) +
                        " hours\n"
                        "Valid range: 0 to " +
                        QString::number(kMaxEstimatedHours) + " hours");
                return;
            }

            if (!checkDuplicateProjectOnEdit(projectName, projectId,
                                             currentCompany)) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "A project with this name already exists!\n\n"
                    "Project name: \"" +
                        projectName +
                        "\"\n"
                        "Project ID: " +
                        QString::number(projectId) +
                        "\n"
                        "Please choose a different name.");
                return;
            }

            const Project* oldProject = currentCompany->getProject(projectId);
            std::vector<Task> savedTasks;
            double savedEmployeeCosts = 0.0;
            if (oldProject) {
                savedTasks = oldProject->getTasks();
                savedEmployeeCosts = oldProject->getEmployeeCosts();
            }

            Project updatedProject(projectId, nameEdit->text().trimmed(),
                                   descEdit->toPlainText().trimmed(),
                                   statusCombo->currentText(),
                                   startDate->date(), endDate->date(),
                                   projectBudget, clientName, estimatedHours);

            for (const auto& task : savedTasks) {
                updatedProject.addTask(task);
            }

            if (savedEmployeeCosts > 0.0) {
                updatedProject.addEmployeeCost(savedEmployeeCosts);
            }

            currentCompany->removeProject(projectId);
            currentCompany->addProject(updatedProject);

            DisplayHelper::displayProjects(projectTable, currentCompany);
            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(
                &dialog, "Success",
                "Project updated successfully!\n\n"
                "Name: " +
                    projectName +
                    "\n"
                    "Status: " +
                    selectedStatus +
                    "\n"
                    "Budget: $" +
                    QString::number(projectBudget, 'f', 2));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                "Failed to edit project!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                &dialog, "Save Error",
                "Failed to save data to file!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "The project may have been edited but the data could not "
                    "be saved.\n"
                    "Please check file permissions and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::deleteProject() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select a project to delete.");
        return;
    }

    int userChoice = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete this project?",
        QMessageBox::Yes | QMessageBox::No);
    if (userChoice == QMessageBox::Yes) {
        try {
            currentCompany->removeProject(projectId);
            DisplayHelper::displayProjects(projectTable, currentCompany);
            if (auto projects = currentCompany->getAllProjects();
                !projects.empty()) {
                nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            QMessageBox::information(this, "Success",
                                     "Project deleted successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::searchProject() const {
    if (currentCompany == nullptr) return;

    QString searchTerm = projectSearchEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        DisplayHelper::displayProjects(projectTable, currentCompany);
        return;
    }

    auto projects = currentCompany->getAllProjects();
    projectTable->setRowCount(0);
    int rowIndex = 0;

    for (const auto& project : projects) {
        if (project.getName().toLower().contains(searchTerm) ||
            project.getStatus().toLower().contains(searchTerm) ||
            project.getClientName().toLower().contains(searchTerm)) {
            projectTable->insertRow(rowIndex);
            projectTable->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(project.getId())));
            projectTable->setItem(rowIndex, 1,
                                  new QTableWidgetItem(project.getName()));
            projectTable->setItem(rowIndex, 2,
                                  new QTableWidgetItem(project.getStatus()));
            projectTable->setItem(rowIndex, 3,
                                  new QTableWidgetItem(QString::number(
                                      project.getBudget(), 'f', 2)));
            projectTable->setItem(rowIndex, 4,
                                  new QTableWidgetItem(QString::number(
                                      project.getEstimatedHours())));
            projectTable->setItem(rowIndex, 5,
                                  new QTableWidgetItem(QString::number(
                                      project.getAllocatedHours())));
            projectTable->setItem(
                rowIndex, 6, new QTableWidgetItem(project.getClientName()));
            rowIndex++;
        }
    }
}

QString MainWindow::getDataDirectory() {
    QDir buildDir = QDir::current();
    if (buildDir.dirName() != "build") {
        QDir projectRoot = QDir::current();
        if (projectRoot.cd("build")) {
            buildDir = projectRoot;
        } else {
            projectRoot.cdUp();
            if (projectRoot.cd("build")) {
                buildDir = projectRoot;
            }
        }
    }

    QString dataDirPath = buildDir.absoluteFilePath("data");
    return dataDirPath;
}

void MainWindow::saveData() {
    if (companies.empty()) return;

    try {
        QString dataDirPath = getDataDirectory();
        QDir dataDir(dataDirPath);

        if (!dataDir.exists()) {
            dataDir.mkpath(".");
        }

        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (!companiesDir.exists()) companiesDir.mkpath(".");
        if (!employeesDir.exists()) employeesDir.mkpath(".");
        if (!projectsDir.exists()) projectsDir.mkpath(".");

        companiesDir.setNameFilters(QStringList() << "*.txt");
        companiesDir.setFilter(QDir::Files);
        for (const QString& fileName : companiesDir.entryList()) {
            companiesDir.remove(fileName);
        }

        employeesDir.setNameFilters(QStringList() << "*.txt");
        employeesDir.setFilter(QDir::Files);
        for (const QString& fileName : employeesDir.entryList()) {
            employeesDir.remove(fileName);
        }

        projectsDir.setNameFilters(QStringList() << "*.txt");
        projectsDir.setFilter(QDir::Files);
        for (const QString& fileName : projectsDir.entryList()) {
            projectsDir.remove(fileName);
        }

        int savedCount = 0;
        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] != nullptr) {
                QString index = QString::number(i + 1);

                QString companyFileName = QString("company_%1.txt").arg(index);
                QString companyFilePath =
                    companiesDir.absoluteFilePath(companyFileName);
                FileManager::saveCompany(*companies[i], companyFilePath);

                QString employeesFileName =
                    QString("employees_%1.txt").arg(index);
                QString employeesFilePath =
                    employeesDir.absoluteFilePath(employeesFileName);
                FileManager::saveEmployees(*companies[i], employeesFilePath);

                QString projectsFileName =
                    QString("projects_%1.txt").arg(index);
                QString projectsFilePath =
                    projectsDir.absoluteFilePath(projectsFileName);
                FileManager::saveProjects(*companies[i], projectsFilePath);

                savedCount++;
            }
        }

        QMessageBox::information(
            this, "Success",
            QString(
                "Data saved successfully!\nDirectory: %1\nCompanies saved: %2")
                .arg(dataDirPath)
                .arg(savedCount));
    } catch (const FileManagerException& e) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to save: ") + e.what());
    }
}

void MainWindow::autoSave() const {
    if (companies.empty()) return;

    try {
        QString dataDirPath = getDataDirectory();
        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (!companiesDir.exists()) companiesDir.mkpath(".");
        if (!employeesDir.exists()) employeesDir.mkpath(".");
        if (!projectsDir.exists()) projectsDir.mkpath(".");

        for (size_t i = 0; i < companies.size(); ++i) {
            if (companies[i] != nullptr) {
                QString index = QString::number(i + 1);

                QString companyFileName = QString("company_%1.txt").arg(index);
                QString companyFilePath =
                    companiesDir.absoluteFilePath(companyFileName);
                FileManager::saveCompany(*companies[i], companyFilePath);

                QString employeesFileName =
                    QString("employees_%1.txt").arg(index);
                QString employeesFilePath =
                    employeesDir.absoluteFilePath(employeesFileName);
                FileManager::saveEmployees(*companies[i], employeesFilePath);

                QString projectsFileName =
                    QString("projects_%1.txt").arg(index);
                QString projectsFilePath =
                    projectsDir.absoluteFilePath(projectsFileName);
                FileManager::saveProjects(*companies[i], projectsFilePath);
            }
        }
    } catch (const FileManagerException&) {
    }
}

void MainWindow::autoLoad() {
    try {
        QString dataDirPath = getDataDirectory();
        QDir companiesDir(dataDirPath + "/companies");
        QDir employeesDir(dataDirPath + "/employees");
        QDir projectsDir(dataDirPath + "/projects");

        if (companiesDir.exists()) {
            companiesDir.setNameFilters(QStringList() << "company_*.txt");
            companiesDir.setFilter(QDir::Files);
            QStringList companyFiles =
                companiesDir.entryList(QDir::Files, QDir::Name);

            if (!companyFiles.isEmpty()) {
                std::vector<Company*> loadedCompanies;

                for (const QString& fileName : companyFiles) {
                    QString index = fileName;
                    index.replace("company_", "").replace(".txt", "");
                    bool conversionOk = false;
                    int companyIndex = index.toInt(&conversionOk);
                    if (!conversionOk) continue;

                    try {
                        QString companyFilePath =
                            companiesDir.absoluteFilePath(fileName);
                        Company company =
                            FileManager::loadCompany(companyFilePath);

                        QString employeesFileName =
                            QString("employees_%1.txt").arg(index);
                        QString employeesFilePath =
                            employeesDir.absoluteFilePath(employeesFileName);
                        if (QFile::exists(employeesFilePath)) {
                            FileManager::loadEmployees(company,
                                                       employeesFilePath);
                        }

                        QString projectsFileName =
                            QString("projects_%1.txt").arg(index);
                        QString projectsFilePath =
                            projectsDir.absoluteFilePath(projectsFileName);
                        if (QFile::exists(projectsFilePath)) {
                            FileManager::loadProjects(company,
                                                      projectsFilePath);
                        }

                        loadedCompanies.push_back(new Company(company));
                    } catch (const FileManagerException&) {
                        continue;
                    }
                }

                if (!loadedCompanies.empty()) {
                    for (auto* company : companies) {
                        delete company;
                    }
                    companies.clear();
                    companies = loadedCompanies;

                    if (!companies.empty()) {
                        currentCompanyIndex = 0;
                        currentCompany = companies[0];
                    } else {
                        currentCompany = nullptr;
                        currentCompanyIndex = -1;
                    }
                    return;
                }
            }
        }

        if (companiesDir.exists()) {
            companiesDir.setNameFilters(QStringList() << "company_*.txt");
            companiesDir.setFilter(QDir::Files);
            QStringList companyFiles =
                companiesDir.entryList(QDir::Files, QDir::Name);

            if (!companyFiles.isEmpty()) {
                std::vector<Company*> loadedCompanies;
                for (const QString& fileName : companyFiles) {
                    QString filepath = companiesDir.absoluteFilePath(fileName);
                    try {
                        Company company = FileManager::loadFromFile(filepath);
                        loadedCompanies.push_back(new Company(company));
                    } catch (const FileManagerException&) {
                    }
                }

                for (auto* company : companies) {
                    delete company;
                }
                companies.clear();
                companies = loadedCompanies;

                if (!companies.empty()) {
                    currentCompanyIndex = 0;
                    currentCompany = companies[0];
                } else {
                    currentCompany = nullptr;
                    currentCompanyIndex = -1;
                }
            }
        }

        if (companySelector != nullptr) {
            refreshCompanyList();
        }

        if (companies.empty()) {
            CompanyManager::initializeCompany(
                companies, currentCompany, currentCompanyIndex, nextEmployeeId,
                nextProjectId, companySelector);
        }
    } catch (const FileManagerException&) {
    }
}

void MainWindow::closeEvent(QCloseEvent* event) { event->accept(); }

void MainWindow::refreshEmployeeTable() const {
    DisplayHelper::displayEmployees(employeeTable, currentCompany);
}

void MainWindow::refreshProjectTable() const {
    DisplayHelper::displayProjects(projectTable, currentCompany);
}

void MainWindow::showCompanyInfo() const {
    DisplayHelper::showCompanyInfo(companyInfoText, currentCompany);
}

void MainWindow::showStatistics() const {
    DisplayHelper::showStatistics(statisticsText, currentCompany);
}

void MainWindow::refreshCompanyList() {
    CompanyManager::refreshCompanyList(companies, companySelector);
}

void MainWindow::addCompany() {
    CompanyManager::addCompany(companies, currentCompany, currentCompanyIndex,
                               companySelector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTable, currentCompany);
    DisplayHelper::displayProjects(projectTable, currentCompany);
    showCompanyInfo();
    showStatistics();
}

void MainWindow::switchCompany() {
    if (companySelector != nullptr) {
        int newIndex = companySelector->currentIndex();
        CompanyManager::switchCompany(companies, currentCompany,
                                      currentCompanyIndex, companySelector,
                                      newIndex);
        DisplayHelper::displayEmployees(employeeTable, currentCompany);
        DisplayHelper::displayProjects(projectTable, currentCompany);
        showCompanyInfo();
        showStatistics();

        if (auto employees = currentCompany->getAllEmployees();
            !employees.empty()) {
            nextEmployeeId = employees.back()->getId() + 1;
        }
        if (auto projects = currentCompany->getAllProjects();
            !projects.empty()) {
            nextProjectId = projects.back().getId() + 1;
        }
    }
}

void MainWindow::deleteCompany() {
    CompanyManager::deleteCompany(companies, currentCompany,
                                  currentCompanyIndex, companySelector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTable, currentCompany);
    DisplayHelper::displayProjects(projectTable, currentCompany);
    showCompanyInfo();
    showStatistics();
}

void MainWindow::addProjectTask() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Add Task to Project");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet(
        "QDialog { background-color: white; } "
        "QComboBox { background-color: white; color: black; } "
        "QLineEdit { background-color: white; color: black; } "
        "QLabel { color: black; }");

    auto* form = new QFormLayout(&dialog);

    auto* taskNameEdit = new QLineEdit();
    taskNameEdit->setPlaceholderText("e.g., API Development");
    form->addRow("Task Name:", taskNameEdit);

    auto* taskTypeCombo = new QComboBox();
    taskTypeCombo->addItems({"Development", "QA", "Design", "Management"});
    taskTypeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    form->addRow("Task Type:", taskTypeCombo);

    auto* taskEstHoursEdit = new QLineEdit();
    taskEstHoursEdit->setPlaceholderText("e.g., 80");
    form->addRow("Estimated Hours:", taskEstHoursEdit);

    auto* priorityEdit = new QLineEdit();
    priorityEdit->setPlaceholderText("e.g., 5 (higher = more important)");
    form->addRow("Priority:", priorityEdit);

    auto* okButton = new QPushButton("Add Task");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            QString taskName = taskNameEdit->text().trimmed();
            if (taskName.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     " Task name cannot be empty!\n\n"
                                     "Please enter a name for the task.");
                return;
            }

            bool success = false;
            int taskEst = taskEstHoursEdit->text().toInt(&success);
            if (!success) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     " Invalid estimated hours format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         taskEstHoursEdit->text() + "\"");
                return;
            }

            if (taskEst <= 0) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    " Estimated hours must be greater than 0!\n\n"
                    "Current value: " +
                        QString::number(taskEst) +
                        " hours\n"
                        "Please enter a value between 1 and " +
                        QString::number(kMaxEstimatedHours));
                return;
            }

            if (taskEst > kMaxEstimatedHours) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    " Estimated hours exceeds maximum allowed!\n\n"
                    "Current value: " +
                        QString::number(taskEst) +
                        " hours\n"
                        "Maximum allowed: " +
                        QString::number(kMaxEstimatedHours) +
                        " hours\n"
                        "Please enter a value between 1 and " +
                        QString::number(kMaxEstimatedHours));
                return;
            }

            QString priorityText = priorityEdit->text().trimmed();
            if (priorityText.isEmpty()) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     " Priority cannot be empty!\n\n"
                                     "Please enter a priority value (0-" +
                                         QString::number(kMaxPriority) +
                                         ").\n"
                                         "Higher numbers = higher priority.");
                return;
            }

            success = false;
            int priority = priorityText.toInt(&success);
            if (!success) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     " Invalid priority format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         priorityText +
                                         "\"\n"
                                         "Priority must be between 0 and " +
                                         QString::number(kMaxPriority));
                return;
            }

            if (priority < 0 || priority > kMaxPriority) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     " Priority out of valid range!\n\n"
                                     "Current value: " +
                                         QString::number(priority) +
                                         "\n"
                                         "Valid range: 0 to " +
                                         QString::number(kMaxPriority) +
                                         "\n"
                                         "Higher numbers = higher priority.");
                return;
            }

            auto tasks = currentCompany->getProjectTasks(projectId);
            for (const auto& existingTask : tasks) {
                if (existingTask.getName().toLower() == taskName.toLower()) {
                    QMessageBox::warning(
                        &dialog, "Duplicate Error",
                        "A task with this name already exists!\n\n"
                        "Task name: \"" +
                            taskName +
                            "\"\n"
                            "Project ID: " +
                            QString::number(projectId) +
                            "\n"
                            "Please choose a different name for the task.");
                    return;
                }
            }

            const auto* proj = currentCompany->getProject(projectId);
            if (!proj) throw CompanyException("Project not found");
            int nextId = proj->getNextTaskId();
            Task task(nextId, taskName, taskTypeCombo->currentText(), taskEst,
                      priority);
            currentCompany->addTaskToProject(projectId, task);

            DisplayHelper::displayProjects(projectTable, currentCompany);
            QMessageBox::information(&dialog, "Success",
                                     "Task added successfully!\n\n"
                                     "Task name: " +
                                         taskName +
                                         "\n"
                                         "Type: " +
                                         taskTypeCombo->currentText() +
                                         "\n"
                                         "Estimated hours: " +
                                         QString::number(taskEst) +
                                         "\n"
                                         "Priority: " +
                                         QString::number(priority));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                "Failed to add task!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::assignEmployeeToTask() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    auto tasks = currentCompany->getProjectTasks(projectId);
    if (tasks.empty()) {
        QMessageBox::warning(
            this, "Error", "No tasks in this project. Please add tasks first.");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Assign Employee to Task");
    dialog.setMinimumWidth(450);
    dialog.setStyleSheet(
        "QDialog { background-color: white; } "
        "QComboBox { background-color: white; color: black; } "
        "QLineEdit { background-color: white; color: black; } "
        "QLabel { color: black; }");

    auto* form = new QFormLayout(&dialog);

    auto* taskCombo = new QComboBox();
    taskCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    for (const auto& task : tasks) {
        int remaining = task.getEstimatedHours() - task.getAllocatedHours();
        QString status = remaining > 0 ? QString("Needs: %1h").arg(remaining)
                                       : "Fully allocated";
        taskCombo->addItem(QString("[%1] %2 (%3)")
                               .arg(task.getType())
                               .arg(task.getName())
                               .arg(status),
                           task.getId());
    }
    form->addRow("Task:", taskCombo);

    auto* employeeCombo = new QComboBox();
    employeeCombo->setStyleSheet(
        "QComboBox { background-color: white; color: black; } "
        "QComboBox QAbstractItemView { background-color: white; color: black; "
        "selection-background-color: #0078d4; }");
    auto employees = currentCompany->getAllEmployees();

    const Project* project = currentCompany->getProject(projectId);
    QString projectStatus = (project != nullptr) ? project->getStatus() : "";

    if (projectStatus == "Completed") {
        QMessageBox::warning(
            this, "Error",
            QString("Cannot assign employees to completed project."));
        return;
    }

    auto roleMatchesSDLC = [](const QString& projectStatus,
                              const QString& position) {
        if (projectStatus == "Analysis" || projectStatus == "Planning") {
            return position == "Manager";
        }
        if (projectStatus == "Design") {
            return position == "Designer";
        } else if (projectStatus == "Development") {
            return position == "Developer";
        } else if (projectStatus == "Testing") {
            return position == "QA";
        } else if (projectStatus == "Deployment") {
            return position == "Manager";
        } else if (projectStatus == "Maintenance") {
            return true;
        }
        return false;
    };

    int matchingCount = 0;
    for (const auto& emp : employees) {
        if (emp && emp->getIsActive()) {
            int available = emp->getAvailableHours();
            int capacity = emp->getWeeklyHoursCapacity();
            int current = emp->getCurrentWeeklyHours();
            bool matches = roleMatchesSDLC(projectStatus, emp->getPosition());

            QString info =
                QString("%1 - %2 | Capacity: %3h | Used: %4h | Available: %5h")
                    .arg(emp->getName())
                    .arg(emp->getPosition())
                    .arg(capacity)
                    .arg(current)
                    .arg(available);

            QString expectedRole;
            if (projectStatus == "Analysis" || projectStatus == "Planning") {
                expectedRole = "Manager";
            } else if (projectStatus == "Design") {
                expectedRole = "Designer";
            } else if (projectStatus == "Development") {
                expectedRole = "Developer";
            } else if (projectStatus == "Testing") {
                expectedRole = "QA";
            } else if (projectStatus == "Deployment") {
                expectedRole = "Manager";
            } else if (projectStatus == "Maintenance") {
                expectedRole = "any role";
            }

            if (!matches && !projectStatus.isEmpty()) {
                info += QString(" [%1 role required for %2 phase]")
                            .arg(expectedRole)
                            .arg(projectStatus);
            }

            employeeCombo->addItem(info, emp->getId());
            if (matches && available > 0) matchingCount++;
        }
    }

    if (employeeCombo->count() == 0) {
        QMessageBox::warning(this, "Error", "No available employees found!");
        return;
    }

    if (matchingCount == 0 && !projectStatus.isEmpty()) {
        QString expectedRole;
        if (projectStatus == "Analysis" || projectStatus == "Planning") {
            expectedRole = "Manager";
        } else if (projectStatus == "Design") {
            expectedRole = "Designer";
        } else if (projectStatus == "Development") {
            expectedRole = "Developer";
        } else if (projectStatus == "Testing") {
            expectedRole = "QA";
        } else if (projectStatus == "Deployment") {
            expectedRole = "Manager";
        } else if (projectStatus == "Maintenance") {
            expectedRole = "any role";
        }

        QMessageBox::information(
            this, "Note",
            QString("No %1 employees available for %2 phase.")
                .arg(expectedRole)
                .arg(projectStatus));
    }

    form->addRow("Employee:", employeeCombo);

    QString expectedRoleText;
    if (projectStatus == "Analysis" || projectStatus == "Planning") {
        expectedRoleText = "Manager role required";
    } else if (projectStatus == "Design") {
        expectedRoleText = "Designer role required";
    } else if (projectStatus == "Development") {
        expectedRoleText = "Developer role required";
    } else if (projectStatus == "Testing") {
        expectedRoleText = "QA role required";
    } else if (projectStatus == "Deployment") {
        expectedRoleText = "Manager role required";
    } else if (projectStatus == "Maintenance") {
        expectedRoleText = "Any role allowed";
    } else {
        expectedRoleText = "Unknown phase";
    }

    auto* taskInfoLabel =
        new QLabel(QString("Project Phase: %1 | %2")
                       .arg(projectStatus.isEmpty() ? "Unknown" : projectStatus)
                       .arg(expectedRoleText));
    form->addRow(taskInfoLabel);

    connect(
        taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [employeeCombo, &employees, roleMatchesSDLC, taskInfoLabel,
         projectStatus](int index) {
            employeeCombo->clear();
            int matchingCount = 0;

            for (const auto& emp : employees) {
                if (emp && emp->getIsActive()) {
                    int available = emp->getAvailableHours();
                    int capacity = emp->getWeeklyHoursCapacity();
                    int current = emp->getCurrentWeeklyHours();
                    bool matches =
                        roleMatchesSDLC(projectStatus, emp->getPosition());

                    QString info =
                        QString("%1 - %2 | Cap: %3h | Used: %4h | Free: %5h")
                            .arg(emp->getName())
                            .arg(emp->getPosition())
                            .arg(capacity)
                            .arg(current)
                            .arg(available);

                    if (!matches && !projectStatus.isEmpty()) {
                        QString expectedRole;
                        if (projectStatus == "Analysis" ||
                            projectStatus == "Planning") {
                            expectedRole = "Manager";
                        } else if (projectStatus == "Design") {
                            expectedRole = "Designer";
                        } else if (projectStatus == "Development") {
                            expectedRole = "Developer";
                        } else if (projectStatus == "Testing") {
                            expectedRole = "QA";
                        } else if (projectStatus == "Deployment") {
                            expectedRole = "Manager";
                        } else if (projectStatus == "Maintenance") {
                            expectedRole = "any role";
                        }
                        info += QString(" [%1 role required for %2 phase]")
                                    .arg(expectedRole)
                                    .arg(projectStatus);
                    } else if (available > 0) {
                        matchingCount++;
                    }

                    employeeCombo->addItem(info, emp->getId());
                }
            }

            QString expectedRole;
            if (projectStatus == "Analysis" || projectStatus == "Planning") {
                expectedRole = "Manager";
            } else if (projectStatus == "Design") {
                expectedRole = "Designer";
            } else if (projectStatus == "Development") {
                expectedRole = "Developer";
            } else if (projectStatus == "Testing") {
                expectedRole = "QA";
            } else if (projectStatus == "Deployment") {
                expectedRole = "Manager";
            } else if (projectStatus == "Maintenance") {
                expectedRole = "any role";
            }

            if (matchingCount > 0) {
                taskInfoLabel->setText(QString("Phase: %1 | Found %2 %3 "
                                               "employees with available hours")
                                           .arg(projectStatus.isEmpty()
                                                    ? "Unknown"
                                                    : projectStatus)
                                           .arg(matchingCount)
                                           .arg(expectedRole));
                taskInfoLabel->setStyleSheet("color: green;");
            } else {
                taskInfoLabel->setText(
                    QString("Phase: %1 | No %2 employees with available hours")
                        .arg(projectStatus.isEmpty() ? "Unknown"
                                                     : projectStatus)
                        .arg(expectedRole));
                taskInfoLabel->setStyleSheet("color: orange;");
            }
        });

    auto* hoursEdit = new QLineEdit();
    hoursEdit->setPlaceholderText("e.g., 20 (hours per week)");

    connect(taskCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [hoursEdit, &tasks](int index) {
                if (index < 0 || static_cast<size_t>(index) >= tasks.size())
                    return;
                const auto& selectedTask = tasks[index];
                int remaining = selectedTask.getEstimatedHours() -
                                selectedTask.getAllocatedHours();
                if (remaining > 0) {
                    hoursEdit->setText(QString::number(remaining));
                } else {
                    hoursEdit->clear();
                }
            });

    form->addRow("Hours per week:", hoursEdit);

    auto* okButton = new QPushButton("Assign");
    form->addRow(okButton);

    connect(okButton, &QPushButton::clicked, [&]() {
        try {
            int taskId = taskCombo->currentData().toInt();
            int employeeId = employeeCombo->currentData().toInt();
            bool success = false;
            QString hoursText = hoursEdit->text().trimmed();
            int hours = hoursText.toInt(&success);
            if (!success) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Invalid hours format!\n\n"
                                     "Please enter a valid number.\n"
                                     "Current value: \"" +
                                         hoursText +
                                         "\"\n"
                                         "Valid range: 1 to " +
                                         QString::number(kMaxHoursPerWeek) +
                                         " hours per week");
                return;
            }

            if (hours <= 0) {
                QMessageBox::warning(&dialog, "Validation Error",
                                     "Hours must be greater than 0!\n\n"
                                     "Current value: " +
                                         QString::number(hours) +
                                         " hours\n"
                                         "Valid range: 1 to " +
                                         QString::number(kMaxHoursPerWeek) +
                                         " hours per week");
                return;
            }

            if (hours > kMaxHoursPerWeek) {
                QMessageBox::warning(
                    &dialog, "Validation Error",
                    "Hours exceeds maximum allowed per week!\n\n"
                    "Current value: " +
                        QString::number(hours) +
                        " hours\n"
                        "Maximum allowed: " +
                        QString::number(kMaxHoursPerWeek) +
                        " hours per week\n"
                        "Valid range: 1 to " +
                        QString::number(kMaxHoursPerWeek) + " hours per week");
                return;
            }

            currentCompany->assignEmployeeToTask(employeeId, projectId, taskId,
                                                 hours);
            DisplayHelper::displayProjects(projectTable, currentCompany);

            QString taskName = "";
            QString employeeName = "";
            for (const auto& task : tasks) {
                if (task.getId() == taskId) {
                    taskName = task.getName();
                    break;
                }
            }
            auto emp = currentCompany->getEmployee(employeeId);
            if (emp) {
                employeeName = emp->getName();
            }

            QMessageBox::information(
                &dialog, "Success",
                "Employee assigned to task successfully!\n\n"
                "Task: " +
                    taskName +
                    "\n"
                    "Employee: " +
                    employeeName +
                    "\n"
                    "Hours per week: " +
                    QString::number(hours));
            dialog.accept();
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                &dialog, "Error",
                "Failed to assign employee to task!\n\n"
                "Error details: " +
                    QString(e.what()) +
                    "\n\n"
                    "Please check the input data and try again.");
        } catch (const std::exception& e) {
            QMessageBox::critical(&dialog, "Unexpected Error",
                                  "An unexpected error occurred!\n\n"
                                  "Error details: " +
                                      QString(e.what()) +
                                      "\n\n"
                                      "Please try again or contact support if "
                                      "the problem persists.");
        }
    });

    dialog.exec();
}

void MainWindow::autoAssignToProject() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    auto tasks = currentCompany->getProjectTasks(projectId);
    if (tasks.empty()) {
        QMessageBox::warning(this, "Error",
                             "Project has no tasks. Please add tasks first "
                             "before using auto-assign!");
        return;
    }

    int response = QMessageBox::question(
        this, "Auto Assign",
        QString("Automatically assign available employees to project "
                "'%1'?\n\nEstimated: %2 hours\nAllocated: %3 hours")
            .arg(project->getName())
            .arg(project->getEstimatedHours())
            .arg(project->getAllocatedHours()),
        QMessageBox::Yes | QMessageBox::No);

    if (response == QMessageBox::Yes) {
        try {
            currentCompany->autoAssignEmployeesToProject(projectId);
            DisplayHelper::displayProjects(projectTable, currentCompany);
            QMessageBox::information(this, "Success",
                                     "Employees auto-assigned successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to auto-assign: ") + e.what());
        }
    }
}

void MainWindow::viewProjectAssignments() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project first.");
        return;
    }

    const auto* project = currentCompany->getProject(projectId);
    if (!project) {
        QMessageBox::warning(this, "Error", "Project not found!");
        return;
    }

    auto tasks = currentCompany->getProjectTasks(projectId);
    auto allEmployees = currentCompany->getAllEmployees();

    std::vector<std::shared_ptr<Employee>> projectEmployees;
    for (const auto& emp : allEmployees) {
        if (emp && emp->isAssignedToProject(projectId)) {
            projectEmployees.push_back(emp);
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Project Team: " + project->getName());
    dialog.setMinimumSize(800, 600);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("font-size: 14px; border: none;");

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
                .arg(project->getName())
                .arg(project->getStatus())
                .arg(project->getClientName())
                .arg(project->getBudget(), 0, 'f', 2);

    int totalEstimated = project->getEstimatedHours();
    int totalAllocated = project->getAllocatedHours();
    int needed = totalEstimated - totalAllocated;
    double budgetUsed = project->getEmployeeCosts();
    double budgetRemaining = project->getBudget() - budgetUsed;
    double budgetPercent = project->getBudget() > 0
                               ? (budgetUsed / project->getBudget() * 100.0)
                               : 0.0;
    double hoursPercent =
        totalEstimated > 0
            ? (static_cast<double>(totalAllocated) / totalEstimated * 100.0)
            : 0.0;

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
                .arg(project->getBudget(), 0, 'f', 2);

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html += QString(R"(<div class="section-title">Team Members (%1)</div>)")
                .arg(projectEmployees.size());

    if (projectEmployees.empty()) {
        html +=
            R"(<div class="empty-state">No employees assigned to this project yet.</div>)";
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

            double utilization =
                emp->getWeeklyHoursCapacity() > 0
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
    html += QString(R"(<div class="section-title">Tasks (%1)</div>)")
                .arg(tasks.size());

    if (tasks.empty()) {
        html +=
            R"(<div class="empty-state">No tasks in this project yet.</div>)";
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
            double taskPercent =
                task.getEstimatedHours() > 0
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

    textEdit->setHtml(html);
    layout->addWidget(textEdit);

    auto* closeButton = new QPushButton("Close");
    closeButton->setStyleSheet(
        "QPushButton { background-color: #1976d2; color: white; padding: 10px "
        "30px; "
        "border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1565c0; }");
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog.exec();
}

void MainWindow::viewEmployeeHistory() {
    if (currentCompany == nullptr) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error", "Please select an employee first.");
        return;
    }

    auto employee = currentCompany->getEmployee(employeeId);
    if (!employee) {
        QMessageBox::warning(this, "Error", "Employee not found!");
        return;
    }

    auto assignedProjectIds = employee->getAssignedProjects();
    auto allProjects = currentCompany->getAllProjects();

    std::vector<const Project*> employeeProjects;
    for (const auto& proj : allProjects) {
        if (std::find(assignedProjectIds.begin(), assignedProjectIds.end(),
                      proj.getId()) != assignedProjectIds.end()) {
            employeeProjects.push_back(&proj);
        }
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Employee History: " + employee->getName());
    dialog.setMinimumSize(800, 600);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("font-size: 14px; border: none;");

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

    double utilization =
        employee->getWeeklyHoursCapacity() > 0
            ? (static_cast<double>(employee->getCurrentWeeklyHours()) /
               employee->getWeeklyHoursCapacity() * 100.0)
            : 0.0;

    QString badgeClass = "badge-type";
    if (employee->getEmployeeType() == "Manager")
        badgeClass = "badge-management";
    else if (employee->getEmployeeType() == "Developer")
        badgeClass = "badge-development";
    else if (employee->getEmployeeType() == "Designer")
        badgeClass = "badge-design";
    else if (employee->getEmployeeType() == "QA")
        badgeClass = "badge-qa";

    QString activeBadge =
        employee->getIsActive()
            ? R"(<span class="badge badge-status">Active</span>)"
            : R"(<span class="badge badge-status" style="background: transparent; color: #000000;">Inactive</span>)";

    html += QString(R"(
        <div class="header">
            <h1>%1 <span class="badge %2">%3</span> %4</h1>
            <div class="header-detail">Position: %5 | Department: %6 | Salary: $%7</div>
        </div>
    )")
                .arg(employee->getName())
                .arg(badgeClass)
                .arg(employee->getEmployeeType())
                .arg(activeBadge)
                .arg(employee->getPosition())
                .arg(employee->getDepartment())
                .arg(employee->getSalary(), 0, 'f', 2);

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
                .arg(employee->getWeeklyHoursCapacity())
                .arg(employee->getCurrentWeeklyHours())
                .arg(employee->getAvailableHours())
                .arg(utilization, 0, 'f', 1)
                .arg(employee->getEmploymentRate(), 0, 'f', 2)
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
                .arg(employee->getCurrentWeeklyHours())
                .arg(employee->getWeeklyHoursCapacity());

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html += R"(<div class="section-title">Employee Details</div>)";

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
                .arg(employee->getId())
                .arg(employee->getEmployeeType())
                .arg(employee->getPosition())
                .arg(employee->getDepartment())
                .arg(employee->getSalary(), 0, 'f', 2)
                .arg(employee->getEmploymentRate(), 0, 'f', 2)
                .arg([](double rate) -> QString {
                    if (rate == 1.0) return "Full Time";
                    if (rate == 0.75) return "Three Quarters";
                    if (rate == 0.5) return "Half Time";
                    if (rate == 0.25) return "Quarter Time";
                    return "Custom";
                }(employee->getEmploymentRate()))
                .arg(employee->getIsActive() ? "Active" : "Inactive");

    html += R"(</div>)";

    html += R"(<div class="section">)";
    html +=
        QString(
            R"(<div class="section-title">Project History (%1 Projects)</div>)")
            .arg(employeeProjects.size());

    if (employeeProjects.empty()) {
        html +=
            R"(<div class="empty-state">This employee is not assigned to any projects yet.</div>)";
    } else {
        for (const auto* proj : employeeProjects) {
            auto tasks = currentCompany->getProjectTasks(proj->getId());

            int projectEstimated = proj->getEstimatedHours();
            int projectAllocated = proj->getAllocatedHours();
            double projectProgress =
                projectEstimated > 0 ? (static_cast<double>(projectAllocated) /
                                        projectEstimated * 100.0)
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
                html +=
                    R"(<div style="margin-top: 12px; padding-top: 12px; border-top: 1px solid #e0e0e0;">)";
                html +=
                    R"(<div style="font-size: 13px; color: #666; margin-bottom: 8px; font-weight: 600;">Tasks:</div>)";

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

                    int taskRemaining =
                        task.getEstimatedHours() - task.getAllocatedHours();
                    double taskProgress =
                        task.getEstimatedHours() > 0
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

    textEdit->setHtml(html);
    layout->addWidget(textEdit);

    auto* closeButton = new QPushButton("Close");
    closeButton->setStyleSheet(
        "QPushButton { background-color: #1976d2; color: white; padding: 10px "
        "30px; "
        "border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1565c0; }");
    connect(closeButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog.exec();
}
