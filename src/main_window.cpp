#include "../include/main_window.h"

#include <QAbstractItemView>
#include <QCoreApplication>
#include <QDialog>
#include <QDir>
#include <QFormLayout>
#include <QHeaderView>
#include <QStandardPaths>
#include <algorithm>
#include <map>

#include "../include/display_helper.h"

#include "../include/derived_employees.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    initializeCompanySetup();
}

void MainWindow::initializeCompanySetup() {
    CompanyManager::initializeCompany(companyData, companyManagementUI.selector);
    DisplayHelper::displayEmployees(employeeTabUI.table,
                                     companyData.currentCompany);
    DisplayHelper::displayProjects(projectTabUI.table,
                                    companyData.currentCompany);
    showCompanyInfo();
    showStatistics();
    
    // Update next IDs
    if (companyData.currentCompany != nullptr) {
        auto employees = companyData.currentCompany->getAllEmployees();
        if (!employees.empty()) {
            companyData.nextEmployeeId = employees.back()->getId() + 1;
        }
        auto projects = companyData.currentCompany->getAllProjects();
        if (!projects.empty()) {
            companyData.nextProjectId = projects.back().getId() + 1;
        }
    }
}

MainWindow::~MainWindow() {
    for (auto* company : companyData.companies) {
        delete company;
    }
}

void MainWindow::setupUI() {
    setWindowTitle("Information Systems for IT Companies");
    setMinimumSize(900, 700);

    // Set white theme
    setStyleSheet(R"(
        QMainWindow {
            background-color: #ffffff;
        }
        QTabWidget::pane {
            background-color: #ffffff;
            border: 1px solid #e0e0e0;
        }
        QTabBar::tab {
            background-color: #f5f5f5;
            color: #333333;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 5px;
            border-top-right-radius: 5px;
        }
        QTabBar::tab:selected {
            background-color: #ffffff;
            color: #2196F3;
            font-weight: bold;
            border-bottom: 2px solid #2196F3;
        }
        QTabBar::tab:hover {
            background-color: #eeeeee;
        }
        QPushButton {
            background-color: #2196F3;
            color: white;
            padding: 8px 16px;
            border: none;
            border-radius: 4px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1976D2;
        }
        QPushButton:pressed {
            background-color: #1565C0;
        }
        QLineEdit, QTextEdit, QComboBox, QDateEdit {
            padding: 6px;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            background-color: #ffffff;
            color: black;
        }
        QLineEdit:focus, QTextEdit:focus {
            border: 2px solid #2196F3;
        }
        QTableWidget {
            background-color: #ffffff;
            alternate-background-color: #f9f9f9;
            border: 1px solid #e0e0e0;
            gridline-color: #e0e0e0;
        }
        QTableWidget::item {
            padding: 5px;
            color: #1976D2;
        }
        QTableWidget::item:selected {
            background-color: #E3F2FD;
            color: #0D47A1;
        }
        QHeaderView::section {
            background-color: #2196F3;
            color: white;
            padding: 8px;
            font-weight: bold;
            border: none;
        }
        QLabel {
            color: #333333;
        }
        QTextEdit {
            background-color: #ffffff;
            color: black;
        }
        QMenuBar {
            background-color: #fafafa;
            border-bottom: 1px solid #e0e0e0;
        }
        QMenuBar::item:selected {
            background-color: #e3f2fd;
        }
        QMessageBox {
            background-color: white;
        }
        QMessageBox QLabel {
            color: #333333;
            padding: 10px;
        }
        QCalendarWidget {
            background-color: white;
            color: black;
        }
        QCalendarWidget QAbstractItemView:enabled {
            background-color: white;
            color: black;
            selection-background-color: #2196F3;
            selection-color: white;
        }
        QCalendarWidget QSpinBox {
            background-color: white;
            color: black;
        }
        QCalendarWidget QTableView {
            background-color: white;
            color: black;
            alternate-background-color: #f9f9f9;
        }
        QCalendarWidget QTableView::item {
            color: black;
            background-color: white;
        }
        QCalendarWidget QTableView::item:selected {
            background-color: #2196F3;
            color: white;
        }
        QCalendarWidget QTableView::item:hover {
            background-color: #E3F2FD;
        }
    )");

    // Create menu bar
    menuBarUI.menuBar = new QMenuBar(this);  // NOLINT
    setMenuBar(menuBarUI.menuBar);

    menuBarUI.fileMenu = menuBarUI.menuBar->addMenu("File");
    menuBarUI.saveAction = menuBarUI.fileMenu->addAction("Save");
    menuBarUI.loadAction = menuBarUI.fileMenu->addAction("Load");
    connect(menuBarUI.saveAction, &QAction::triggered, this,
            &MainWindow::saveData);
    connect(menuBarUI.loadAction, &QAction::triggered, this,
            &MainWindow::loadData);

    // Company management in menu
    QMenu* companyMenu = menuBarUI.menuBar->addMenu("Company");
    companyMenu->addAction("Add Company", this, &MainWindow::addCompany);
    companyMenu->addAction("Delete Company", this, &MainWindow::deleteCompany);

    // Company selector widget
    auto* companyWidget = new QWidget();
    auto* companyLayout = new QHBoxLayout(companyWidget);
    companyLayout->setContentsMargins(
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins,
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins);

    auto* companyLabel = new QLabel("Current Company:");
    companyManagementUI.selector = new QComboBox();  // NOLINT
    companyManagementUI.selector->setMinimumWidth(kCompanySelectorMinWidth);
    companyManagementUI.selector->setStyleSheet(R"(
        QComboBox {
            background-color: white;
            color: black;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
            padding: 5px;
        }
        QComboBox:hover {
            border: 1px solid #2196F3;
        }
        QComboBox::drop-down {
            border: none;
            background-color: white;
        }
        QComboBox::down-arrow {
            image: none;
            border: none;
        }
        QComboBox QAbstractItemView {
            background-color: white;
            color: black;
            selection-background-color: #2196F3;
            selection-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 4px;
        }
    )");

    companyManagementUI.addBtn = new QPushButton("Add");        // NOLINT
    companyManagementUI.deleteBtn = new QPushButton("Delete");  // NOLINT

    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(companyManagementUI.selector);
    companyLayout->addWidget(companyManagementUI.addBtn);
    companyLayout->addWidget(companyManagementUI.deleteBtn);
    companyLayout->addStretch();

    connect(companyManagementUI.selector,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::switchCompany);
    connect(companyManagementUI.addBtn, &QPushButton::clicked, this,
            &MainWindow::addCompany);
    connect(companyManagementUI.deleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteCompany);

    // Create tab widget
    tabWidgets.mainTabWidget = new QTabWidget(this);  // NOLINT

    // Main widget with company selector and tabs
    auto* centralWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(companyWidget);
    mainLayout->addWidget(tabWidgets.mainTabWidget);

    setCentralWidget(centralWidget);

    setupEmployeeTab();
    setupProjectTab();
    setupCompanyInfoTab();
    setupStatisticsTab();
}

void MainWindow::setupEmployeeTab() {
    employeeTabUI.tab = new QWidget();  // NOLINT

    auto* mainLayout = new QVBoxLayout(employeeTabUI.tab);
    mainLayout->setSpacing(kLayoutSpacing);
    mainLayout->setContentsMargins(kLayoutMargins, kLayoutMargins,
                                   kLayoutMargins, kLayoutMargins);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    employeeTabUI.searchEdit = new QLineEdit();  // NOLINT
    employeeTabUI.searchEdit->setPlaceholderText(
        "Search employees by name, department, or position...");
    employeeTabUI.searchEdit->setMinimumHeight(kSearchEditMinHeight);
    employeeTabUI.searchBtn = new QPushButton("Search");  // NOLINT
    searchLayout->addWidget(employeeTabUI.searchEdit);
    searchLayout->addWidget(employeeTabUI.searchBtn);
    mainLayout->addLayout(searchLayout);

    // Table
    employeeTabUI.table = new QTableWidget();  // NOLINT
    employeeTabUI.table->setColumnCount(6);
    QStringList headers = {"ID",         "Name",   "Position",
                           "Department", "Salary", "Type"};
    employeeTabUI.table->setHorizontalHeaderLabels(headers);
    employeeTabUI.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeeTabUI.table->setSelectionMode(QAbstractItemView::SingleSelection);
    employeeTabUI.table->horizontalHeader()->setStretchLastSection(true);
    employeeTabUI.table->setAlternatingRowColors(true);

    // Set column widths for better visibility
    employeeTabUI.table->setColumnWidth(0, kTableColumnWidthId);        // ID
    employeeTabUI.table->setColumnWidth(1, kTableColumnWidthName);      // Name
    employeeTabUI.table->setColumnWidth(2, kTableColumnWidthPosition);  // Position
    employeeTabUI.table->setColumnWidth(3,
                                  kTableColumnWidthDepartment);  // Department
    employeeTabUI.table->setColumnWidth(4, kTableColumnWidthSalary);   // Salary
    employeeTabUI.table->setColumnWidth(5, kTableColumnWidthType);     // Type

    // Set row height for better readability
    employeeTabUI.table->verticalHeader()->setDefaultSectionSize(kTableRowHeight);

    mainLayout->addWidget(employeeTabUI.table);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    employeeTabUI.addBtn = new QPushButton("Add Employee");        // NOLINT
    employeeTabUI.editBtn = new QPushButton("Edit Employee");      // NOLINT
    employeeTabUI.deleteBtn = new QPushButton("Delete Employee");  // NOLINT
    buttonLayout->addWidget(employeeTabUI.addBtn);
    buttonLayout->addWidget(employeeTabUI.editBtn);
    buttonLayout->addWidget(employeeTabUI.deleteBtn);
    mainLayout->addLayout(buttonLayout);

    connect(employeeTabUI.addBtn, &QPushButton::clicked, this,
            &MainWindow::addEmployee);
    connect(employeeTabUI.editBtn, &QPushButton::clicked, this,
            &MainWindow::editEmployee);
    connect(employeeTabUI.deleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteEmployee);
    connect(employeeTabUI.searchBtn, &QPushButton::clicked, this,
            &MainWindow::searchEmployee);

    tabWidgets.mainTabWidget->addTab(employeeTabUI.tab, "Employees");
}

void MainWindow::setupProjectTab() {
    projectTabUI.tab = new QWidget();  // NOLINT

    auto* mainLayout = new QVBoxLayout(projectTabUI.tab);
    mainLayout->setSpacing(kLayoutSpacing);
    mainLayout->setContentsMargins(kLayoutMargins, kLayoutMargins,
                                   kLayoutMargins, kLayoutMargins);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    projectTabUI.searchEdit = new QLineEdit();  // NOLINT
    projectTabUI.searchEdit->setPlaceholderText(
        "Search projects by name, status, or client...");
    projectTabUI.searchEdit->setMinimumHeight(kSearchEditMinHeight);
    projectTabUI.searchBtn = new QPushButton("Search");  // NOLINT
    searchLayout->addWidget(projectTabUI.searchEdit);
    searchLayout->addWidget(projectTabUI.searchBtn);
    mainLayout->addLayout(searchLayout);

    // Table
    projectTabUI.table = new QTableWidget();  // NOLINT
    projectTabUI.table->setColumnCount(5);
    QStringList headers = {"ID", "Name", "Status", "Budget", "Client"};
    projectTabUI.table->setHorizontalHeaderLabels(headers);
    projectTabUI.table->setSelectionBehavior(QAbstractItemView::SelectRows);
    projectTabUI.table->setSelectionMode(QAbstractItemView::SingleSelection);
    projectTabUI.table->horizontalHeader()->setStretchLastSection(true);
    projectTabUI.table->setAlternatingRowColors(true);

    // Set column widths for better visibility
    projectTabUI.table->setColumnWidth(0, kTableColumnWidthId);           // ID
    projectTabUI.table->setColumnWidth(1, kTableColumnWidthProjectName);  // Name
    projectTabUI.table->setColumnWidth(2, kTableColumnWidthStatus);       // Status
    projectTabUI.table->setColumnWidth(3, kTableColumnWidthBudget);       // Budget
    projectTabUI.table->setColumnWidth(4, kTableColumnWidthClient);       // Client

    // Set row height for better readability
    projectTabUI.table->verticalHeader()->setDefaultSectionSize(kTableRowHeight);

    mainLayout->addWidget(projectTabUI.table);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    projectTabUI.addBtn = new QPushButton("Add Project");        // NOLINT
    projectTabUI.editBtn = new QPushButton("Edit Project");      // NOLINT
    projectTabUI.deleteBtn = new QPushButton("Delete Project");  // NOLINT
    buttonLayout->addWidget(projectTabUI.addBtn);
    buttonLayout->addWidget(projectTabUI.editBtn);
    buttonLayout->addWidget(projectTabUI.deleteBtn);
    mainLayout->addLayout(buttonLayout);

    connect(projectTabUI.addBtn, &QPushButton::clicked, this,
            &MainWindow::addProject);
    connect(projectTabUI.editBtn, &QPushButton::clicked, this,
            &MainWindow::editProject);
    connect(projectTabUI.deleteBtn, &QPushButton::clicked, this,
            &MainWindow::deleteProject);
    connect(projectTabUI.searchBtn, &QPushButton::clicked, this,
            &MainWindow::searchProject);

    tabWidgets.mainTabWidget->addTab(projectTabUI.tab, "Projects");
}

void MainWindow::setupCompanyInfoTab() {
    tabWidgets.infoTab = new QWidget();  // NOLINT
    auto* layout = new QVBoxLayout(tabWidgets.infoTab);
    layout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                               kLayoutMargins);

    tabWidgets.companyInfoText = new QTextEdit();  // NOLINT
    tabWidgets.companyInfoText->setReadOnly(true);
    tabWidgets.companyInfoText->setStyleSheet("font-size: 14px; line-height: 1.6;");
    layout->addWidget(tabWidgets.companyInfoText);

    tabWidgets.mainTabWidget->addTab(tabWidgets.infoTab, "Company Info");
}

void MainWindow::setupStatisticsTab() {
    tabWidgets.statsTab = new QWidget();  // NOLINT
    auto* layout = new QVBoxLayout(tabWidgets.statsTab);
    layout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                               kLayoutMargins);

    tabWidgets.statisticsText = new QTextEdit();  // NOLINT
    tabWidgets.statisticsText->setReadOnly(true);
    tabWidgets.statisticsText->setStyleSheet(
        "font-size: 14px; line-height: 1.6; background-color: white; color: "
        "black;");
    layout->addWidget(tabWidgets.statisticsText);

    tabWidgets.refreshStatsBtn = new QPushButton("Refresh Statistics");  // NOLINT
    layout->addWidget(tabWidgets.refreshStatsBtn);

    connect(tabWidgets.refreshStatsBtn, &QPushButton::clicked, this,
            &MainWindow::showStatistics);

    tabWidgets.mainTabWidget->addTab(tabWidgets.statsTab, "Statistics");
}



int MainWindow::getSelectedEmployeeId() const {
    int rowIndex = employeeTabUI.table->currentRow();
    if (rowIndex >= 0) {
        QTableWidgetItem* tableItem = employeeTabUI.table->item(rowIndex, 0);
        if (tableItem != nullptr) {
            bool conversionSuccess = false;
            int employeeId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? employeeId : -1;
        }
    }
    return -1;
}

int MainWindow::getSelectedProjectId() const {
    int rowIndex = projectTabUI.table->currentRow();
    if (rowIndex >= 0) {
        QTableWidgetItem* tableItem = projectTabUI.table->item(rowIndex, 0);
        if (tableItem != nullptr) {
            bool conversionSuccess = false;
            int projectId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? projectId : -1;
        }
    }
    return -1;
}

bool MainWindow::checkDuplicateProjectOnEdit(const QString& projectName,
                                              int excludeId,
                                              Company* currentCompany) {
    if (currentCompany == nullptr) {
        return true;
    }
    auto existingProjects = currentCompany->getAllProjects();
    auto duplicateFound = std::any_of(
        existingProjects.begin(), existingProjects.end(),
        [&projectName, excludeId](const auto& project) {
            return project.getId() != excludeId &&
                   project.getName().toLower() == projectName.toLower();
        });
    return !duplicateFound;
}


void MainWindow::addEmployee() {
    if (companyData.currentCompany == nullptr) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Employee");
    dialog.setMinimumWidth(kDefaultDialogMinWidth);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);
    EmployeeFormWidgets widgets =
        EmployeeDialogHelper::createEmployeeDialog(dialog, form);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            QString name = widgets.nameEdit->text().trimmed();
            double salary = widgets.salaryEdit->text().toDouble();
            QString department = widgets.deptEdit->text().trimmed();

            if (!EmployeeDialogHelper::validateEmployeeInput(name, salary,
                                                              department)) {
                QMessageBox::warning(this, "Validation Error",
                                     "Invalid input! Please check all fields.");
                return;
            }

            if (!EmployeeDialogHelper::checkDuplicateEmployee(
                    name, companyData.currentCompany)) {
                return;
            }

            int employeeId = companyData.nextEmployeeId;
            companyData.nextEmployeeId++;
            QString employeeType = widgets.typeCombo->currentText();

            auto employee = EmployeeDialogHelper::createEmployeeFromType(
                employeeType, employeeId, name, salary, department, widgets);
            if (employee == nullptr) {
                QMessageBox::warning(this, "Validation Error",
                                     "Invalid employee data!");
                return;
            }

            companyData.currentCompany->addEmployee(employee);
            DisplayHelper::displayEmployees(employeeTabUI.table,
                                            companyData.currentCompany);
            auto employees = companyData.currentCompany->getAllEmployees();
            if (!employees.empty()) {
                companyData.nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();
            QMessageBox::information(this, "Success",
                                     "Employee added successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to add employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::editEmployee() {
    if (companyData.currentCompany == nullptr) return;

    int employeeId = getSelectedEmployeeId();
    if (employeeId < 0) {
        QMessageBox::warning(this, "Error",
                             "Please select an employee to edit.");
        return;
    }

    auto employee = companyData.currentCompany->getEmployee(employeeId);
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
    EmployeeFormWidgets widgets =
        EmployeeDialogHelper::createEditEmployeeDialog(dialog, form, employee);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            QString name = widgets.nameEdit->text().trimmed();
            double salary = widgets.salaryEdit->text().toDouble();
            QString department = widgets.deptEdit->text().trimmed();

            if (!EmployeeDialogHelper::validateEmployeeInput(name, salary, department)) {
                QMessageBox::warning(this, "Validation Error",
                                     "Invalid input! Please check all fields.");
                return;
            }

            if (!EmployeeDialogHelper::checkDuplicateEmployeeOnEdit(name, employeeId, companyData.currentCompany)) {
                return;
            }

            auto updatedEmployee = EmployeeDialogHelper::createEmployeeFromType(
                currentType, employeeId, name, salary, department, widgets);
            if (updatedEmployee == nullptr) {
                return;
            }

            companyData.currentCompany->removeEmployee(employeeId);
            companyData.currentCompany->addEmployee(updatedEmployee);

            DisplayHelper::displayEmployees(employeeTabUI.table,
                                            companyData.currentCompany);
            auto employees = companyData.currentCompany->getAllEmployees();
            if (!employees.empty()) {
                companyData.nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();
            QMessageBox::information(this, "Success",
                                     "Employee updated successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to edit employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::deleteEmployee() {
    if (companyData.currentCompany == nullptr) return;

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
            companyData.currentCompany->removeEmployee(employeeId);
            DisplayHelper::displayEmployees(employeeTabUI.table,
                                            companyData.currentCompany);
            auto employees = companyData.currentCompany->getAllEmployees();
            if (!employees.empty()) {
                companyData.nextEmployeeId = employees.back()->getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after deletion
            QMessageBox::information(this, "Success",
                                     "Employee deleted successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to delete employee: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::searchEmployee() {
    if (companyData.currentCompany == nullptr) return;

    QString searchTerm = employeeTabUI.searchEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        DisplayHelper::displayEmployees(employeeTabUI.table,
                                         companyData.currentCompany);
        return;
    }

    auto employees = companyData.currentCompany->getAllEmployees();
    employeeTabUI.table->setRowCount(0);
    int rowIndex = 0;

    for (const auto& employee : employees) {
        if (employee->getName().toLower().contains(searchTerm) ||
            employee->getDepartment().toLower().contains(searchTerm) ||
            employee->getPosition().toLower().contains(searchTerm)) {
            employeeTabUI.table->insertRow(rowIndex);
            employeeTabUI.table->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(employee->getId())));
            employeeTabUI.table->setItem(rowIndex, 1,
                                   new QTableWidgetItem(employee->getName()));
            employeeTabUI.table->setItem(
                rowIndex, 2, new QTableWidgetItem(employee->getPosition()));
            employeeTabUI.table->setItem(
                rowIndex, 3, new QTableWidgetItem(employee->getDepartment()));
            employeeTabUI.table->setItem(rowIndex, 4,
                                   new QTableWidgetItem(QString::number(
                                       employee->getSalary(), 'f', 2)));
            employeeTabUI.table->setItem(
                rowIndex, 5, new QTableWidgetItem(employee->getEmployeeType()));
            rowIndex++;
        }
    }
}

void MainWindow::addProject() {
    if (companyData.currentCompany == nullptr) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Project");
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);

    // Project type selector
    auto* projectTypeCombo = new QComboBox();
    projectTypeCombo->addItems({"Web Development", "Mobile App",
                                "Software Product", "Consulting", "Other"});
    projectTypeCombo->setStyleSheet("background-color: white;");
    form->addRow("Project Type:", projectTypeCombo);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., E-commerce Platform");
    form->addRow("Project Name:", nameEdit);

    auto* descEdit = new QTextEdit();
    descEdit->setMaximumHeight(kDescEditMaxHeight);
    descEdit->setPlaceholderText("Brief description of the project...");
    form->addRow("Description:", descEdit);

    auto* statusCombo = new QComboBox();
    statusCombo->addItems(
        {"Planning", "Active", "In Progress", "Completed", "On Hold"});
    statusCombo->setStyleSheet("background-color: white;");
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

    // Client information fields (type-specific)
    auto* clientNameEdit = new QLineEdit();
    clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
    auto* clientNameLabel = new QLabel("Client Name:");
    form->addRow(clientNameLabel, clientNameEdit);

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

    // Show/hide client fields based on project type
    connect(projectTypeCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            [clientIndustryLabel, clientIndustryEdit, clientContactLabel,
             clientContactEdit](int index) {
                // For consulting and other types, show additional client fields
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
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            // Validation
            if (nameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Project name cannot be empty!");
                return;
            }
            bool conversionSuccess = false;
            double projectBudget =
                budgetEdit->text().toDouble(&conversionSuccess);
            if (!conversionSuccess || projectBudget < 0) {
                QMessageBox::warning(this, "Validation Error",
                                     "Please enter a valid budget amount!");
                return;
            }
            if (clientNameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Client name cannot be empty!");
                return;
            }
            if (endDate->date() < startDate->date()) {
                QMessageBox::warning(this, "Validation Error",
                                     "End date cannot be before start date!");
                return;
            }

            // Check for duplicate project by name
            QString projectName = nameEdit->text().trimmed();
            auto existingProjects = companyData.currentCompany->getAllProjects();
            for (const auto& project : existingProjects) {
                if (project.getName().toLower() == projectName.toLower()) {
                    QMessageBox::warning(
                        this, "Duplicate Error",
                        "A project with this name already exists!");
                    return;
                }
            }

            // Generate ID only after all validation passes
            int projectId = companyData.nextProjectId;
            companyData.nextProjectId++;
            ProjectParams params{nameEdit->text().trimmed(),
                                 descEdit->toPlainText().trimmed(),
                                 statusCombo->currentText(),
                                 startDate->date(),
                                 endDate->date(),
                                 projectBudget,
                                 clientNameEdit->text().trimmed()};
            Project project(projectId, params);

            companyData.currentCompany->addProject(project);
            DisplayHelper::displayProjects(projectTabUI.table,
                                           companyData.currentCompany);
            auto projects = companyData.currentCompany->getAllProjects();
            if (!projects.empty()) {
                companyData.nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after adding
            QMessageBox::information(this, "Success",
                                     "Project added successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to add project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::editProject() {
    if (companyData.currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project to edit.");
        return;
    }

    auto* project = companyData.currentCompany->getProject(projectId);
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
    statusCombo->addItems(
        {"Planning", "Active", "In Progress", "Completed", "On Hold"});
    statusCombo->setStyleSheet("background-color: white;");
    form->addRow("Status:", statusCombo);

    // Set current status
    int statusIndex = statusCombo->findText(project->getStatus());
    if (statusIndex >= 0) statusCombo->setCurrentIndex(statusIndex);

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

    auto* clientNameEdit = new QLineEdit();
    clientNameEdit->setPlaceholderText("e.g., Company ABC Inc.");
    clientNameEdit->setText(project->getClientName());
    form->addRow("Client Name:", clientNameEdit);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            // Validation
            if (nameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Project name cannot be empty!");
                return;
            }
            bool conversionSuccess = false;
            double projectBudget =
                budgetEdit->text().toDouble(&conversionSuccess);
            if (!conversionSuccess || projectBudget < 0) {
                QMessageBox::warning(this, "Validation Error",
                                     "Please enter a valid budget amount!");
                return;
            }
            if (clientNameEdit->text().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Client name cannot be empty!");
                return;
            }
            if (endDate->date() < startDate->date()) {
                QMessageBox::warning(this, "Validation Error",
                                     "End date cannot be before start date!");
                return;
            }

            // Check for duplicate name (excluding current project)
            QString projectName = nameEdit->text().trimmed();
            if (!checkDuplicateProjectOnEdit(projectName, projectId,
                                               companyData.currentCompany)) {
                QMessageBox::warning(
                    this, "Duplicate Error",
                    "A project with this name already exists!");
                return;
            }

            // Create updated project
            ProjectParams params{nameEdit->text().trimmed(),
                                 descEdit->toPlainText().trimmed(),
                                 statusCombo->currentText(),
                                 startDate->date(),
                                 endDate->date(),
                                 projectBudget,
                                 clientNameEdit->text().trimmed()};
            Project updatedProject(projectId, params);

            // Remove old project and add updated one
            companyData.currentCompany->removeProject(projectId);
            companyData.currentCompany->addProject(updatedProject);

            DisplayHelper::displayProjects(projectTabUI.table,
                                           companyData.currentCompany);
            auto projects = companyData.currentCompany->getAllProjects();
            if (!projects.empty()) {
                companyData.nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after editing
            QMessageBox::information(this, "Success",
                                     "Project updated successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to edit project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::deleteProject() {
    if (companyData.currentCompany == nullptr) return;

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
            companyData.currentCompany->removeProject(projectId);
            DisplayHelper::displayProjects(projectTabUI.table,
                                           companyData.currentCompany);
            auto projects = companyData.currentCompany->getAllProjects();
            if (!projects.empty()) {
                companyData.nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after deletion
            QMessageBox::information(this, "Success",
                                     "Project deleted successfully!");
        } catch (const CompanyException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to delete project: ") + e.what());
        } catch (const FileManagerException& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to save: ") + e.what());
        }
    }
}

void MainWindow::searchProject() {
    if (companyData.currentCompany == nullptr) return;

    QString searchTerm = projectTabUI.searchEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        DisplayHelper::displayProjects(projectTabUI.table,
                                        companyData.currentCompany);
        return;
    }

    auto projects = companyData.currentCompany->getAllProjects();
    projectTabUI.table->setRowCount(0);
    int rowIndex = 0;

    for (const auto& project : projects) {
        if (project.getName().toLower().contains(searchTerm) ||
            project.getStatus().toLower().contains(searchTerm) ||
            project.getClientName().toLower().contains(searchTerm)) {
            projectTabUI.table->insertRow(rowIndex);
            projectTabUI.table->setItem(
                rowIndex, 0,
                new QTableWidgetItem(QString::number(project.getId())));
            projectTabUI.table->setItem(rowIndex, 1,
                                  new QTableWidgetItem(project.getName()));
            projectTabUI.table->setItem(rowIndex, 2,
                                  new QTableWidgetItem(project.getStatus()));
            projectTabUI.table->setItem(rowIndex, 3,
                                  new QTableWidgetItem(QString::number(
                                      project.getBudget(), 'f', 2)));
            projectTabUI.table->setItem(
                rowIndex, 4, new QTableWidgetItem(project.getClientName()));
            rowIndex++;
        }
    }
}

void MainWindow::saveData() {
        if (companyData.companies.empty()) return;

    try {
        // Save to build directory
        QDir buildDir = QDir::current();
        // If we're not in build directory, try to find it
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

        QString filepath = buildDir.absoluteFilePath("companies.txt");
        // Save all companies
        FileManager::saveCompanies(companyData.companies, filepath);
        QMessageBox::information(
            this, "Success",
            QString("Data saved successfully to:\n") + filepath +
                QString("\n\nCompanies saved: %1").arg(companyData.companies.size()));
    } catch (const FileManagerException& e) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to save: ") + e.what());
    }
}

void MainWindow::autoSave() const {
        if (companyData.companies.empty()) return;

    try {
        // Save to build directory
        QDir buildDir = QDir::current();
        // If we're not in build directory, try to find it
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

        QString filepath = buildDir.absoluteFilePath("companies.txt");
        // Save all companies
        FileManager::saveCompanies(companyData.companies, filepath);
        // Silent save - no message box
    } catch (const FileManagerException& /* e */) {
        // Silently fail - don't interrupt user workflow
        // Could optionally log to console in debug mode
    }
}

void MainWindow::loadData() {
    try {
        // Load from build directory
        QDir buildDir = QDir::current();
        // If we're not in build directory, try to find it
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

        QString filepath = buildDir.absoluteFilePath("companies.txt");

        // Load all companies
        std::vector<Company*> loadedCompanies =
            FileManager::loadCompanies(filepath);

        // Clear old companies
        for (auto* company : companyData.companies) {
            delete company;
        }
        companyData.companies.clear();

        // Set loaded companies
        companyData.companies = loadedCompanies;

        // Set first company as current if available
        if (!companyData.companies.empty()) {
            companyData.currentCompanyIndex = 0;
            companyData.currentCompany = companyData.companies[0];
        } else {
            companyData.currentCompany = nullptr;
            companyData.currentCompanyIndex = -1;
        }

        // Update company selector
        refreshCompanyList();

        if (companyData.currentCompany != nullptr) {
            DisplayHelper::displayEmployees(employeeTabUI.table,
                                            companyData.currentCompany);
            auto employees = companyData.currentCompany->getAllEmployees();
            if (!employees.empty()) {
                companyData.nextEmployeeId = employees.back()->getId() + 1;
            }
            DisplayHelper::displayProjects(projectTabUI.table,
                                           companyData.currentCompany);
            auto projects = companyData.currentCompany->getAllProjects();
            if (!projects.empty()) {
                companyData.nextProjectId = projects.back().getId() + 1;
            }
            showCompanyInfo();
            showStatistics();
        }

        QMessageBox::information(
            this, "Success",
            QString("Data loaded successfully!\nCompanies loaded: %1")
                .arg(companyData.companies.size()));
    } catch (const FileManagerException& e) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to load: ") + e.what());
    }
}


void MainWindow::refreshEmployeeTable() {
    DisplayHelper::displayEmployees(employeeTabUI.table,
                                     companyData.currentCompany);
}

void MainWindow::refreshProjectTable() {
    DisplayHelper::displayProjects(projectTabUI.table,
                                    companyData.currentCompany);
}

void MainWindow::showCompanyInfo() {
    DisplayHelper::showCompanyInfo(tabWidgets.companyInfoText,
                                   companyData.currentCompany);
}

void MainWindow::showStatistics() {
    DisplayHelper::showStatistics(tabWidgets.statisticsText,
                                  companyData.currentCompany);
}

void MainWindow::refreshCompanyList() {
    CompanyManager::refreshCompanyList(companyData, companyManagementUI.selector);
}

void MainWindow::addCompany() {
    CompanyManager::addCompany(companyData, companyManagementUI.selector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTabUI.table,
                                     companyData.currentCompany);
    DisplayHelper::displayProjects(projectTabUI.table,
                                    companyData.currentCompany);
    showCompanyInfo();
    showStatistics();
    autoSave();  // Save new company to file
}

void MainWindow::switchCompany() {
    if (companyManagementUI.selector != nullptr) {
        // Save previous company before switching
        if (companyData.currentCompany != nullptr) {
            autoSave();
        }

        int newIndex = companyManagementUI.selector->currentIndex();
        CompanyManager::switchCompany(companyData, companyManagementUI.selector,
                                     newIndex);
        DisplayHelper::displayEmployees(employeeTabUI.table,
                                        companyData.currentCompany);
        DisplayHelper::displayProjects(projectTabUI.table,
                                       companyData.currentCompany);
        showCompanyInfo();
        showStatistics();
        
        // Update next IDs
        auto employees = companyData.currentCompany->getAllEmployees();
        if (!employees.empty()) {
            companyData.nextEmployeeId = employees.back()->getId() + 1;
        }
        auto projects = companyData.currentCompany->getAllProjects();
        if (!projects.empty()) {
            companyData.nextProjectId = projects.back().getId() + 1;
        }
    }
}

void MainWindow::deleteCompany() {
    CompanyManager::deleteCompany(companyData, companyManagementUI.selector, this);
    refreshCompanyList();
    DisplayHelper::displayEmployees(employeeTabUI.table,
                                     companyData.currentCompany);
    DisplayHelper::displayProjects(projectTabUI.table,
                                    companyData.currentCompany);
    showCompanyInfo();
    showStatistics();
    if (companyData.currentCompany != nullptr) {
        autoSave();
    }
}
