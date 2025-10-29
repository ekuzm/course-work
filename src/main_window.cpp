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

#include "../include/derived_employees.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setupUI();
    initializeCompanySetup();
}

void MainWindow::initializeCompanySetup() {
    while (true) {
        QDialog dialog(this);
        dialog.setWindowTitle("Setup Your IT Company");
        dialog.setMinimumWidth(kDefaultDialogMinWidth);
        dialog.setStyleSheet("QDialog { background-color: white; }");

        auto* form = new QFormLayout(&dialog);

        auto* nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("e.g., Yandex");
        nameEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Company Name:", nameEdit);

        auto* industryEdit = new QLineEdit();
        industryEdit->setPlaceholderText("e.g., Software Development");
        industryEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Industry:", industryEdit);

        auto* locationEdit = new QLineEdit();
        locationEdit->setPlaceholderText("e.g., Minsk, Belarus");
        locationEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Location:", locationEdit);

        auto* yearEdit = new QLineEdit();
        yearEdit->setPlaceholderText("e.g., 2020");
        yearEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Founded Year:", yearEdit);

        auto* okButton = new QPushButton("Create Company");
        form->addRow(okButton);
        connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

        int result = dialog.exec();

        // If user closed the dialog without creating company
        if (result != QDialog::Accepted) {
            QCoreApplication::quit();
            return;
        }

        // If dialog was accepted, validate input
        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        // Validation
        if (companyName.isEmpty()) {
            QMessageBox::warning(this, "Error",
                                 "Company name cannot be empty!");
            continue;  // Restart dialog
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(this, "Error", "Industry cannot be empty!");
            continue;  // Restart dialog
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(this, "Error", "Location cannot be empty!");
            continue;  // Restart dialog
        }
        if (!conversionSuccess || foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(this, "Error", "Please enter a valid year!");
            continue;  // Restart dialog
        }

        // All validation passed, create company
        currentCompany = new Company(companyName, companyIndustry,
                                     companyLocation, foundedYear);
        companies.push_back(currentCompany);
        currentCompanyIndex = companies.size() - 1;

        // Update company selector
        companySelector->addItem(companyName);
        companySelector->setCurrentIndex(currentCompanyIndex);

        displayEmployees();
        displayProjects();
        showCompanyInfo();
        showStatistics();

        // Successfully created company, exit the loop
        return;
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
    menuBar = new QMenuBar(this);  // NOLINT
    setMenuBar(menuBar);

    fileMenu = menuBar->addMenu("File");
    saveAction = fileMenu->addAction("Save");
    loadAction = fileMenu->addAction("Load");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveData);
    connect(loadAction, &QAction::triggered, this, &MainWindow::loadData);

    // Company management in menu
    QMenu* companyMenu = menuBar->addMenu("Company");
    companyMenu->addAction("Add Company", this, &MainWindow::addCompany);
    companyMenu->addAction("Delete Company", this, &MainWindow::deleteCompany);

    // Company selector widget
    auto* companyWidget = new QWidget();
    auto* companyLayout = new QHBoxLayout(companyWidget);
    companyLayout->setContentsMargins(kCompanyLayoutMargins, kCompanyLayoutVerticalMargins,
                                      kCompanyLayoutMargins, kCompanyLayoutVerticalMargins);

    auto* companyLabel = new QLabel("Current Company:");
    companySelector = new QComboBox();  // NOLINT
    companySelector->setMinimumWidth(kCompanySelectorMinWidth);
    companySelector->setStyleSheet(R"(
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

    addCompanyBtn = new QPushButton("Add");        // NOLINT
    deleteCompanyBtn = new QPushButton("Delete");  // NOLINT

    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(companySelector);
    companyLayout->addWidget(addCompanyBtn);
    companyLayout->addWidget(deleteCompanyBtn);
    companyLayout->addStretch();

    connect(companySelector,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &MainWindow::switchCompany);
    connect(addCompanyBtn, &QPushButton::clicked, this,
            &MainWindow::addCompany);
    connect(deleteCompanyBtn, &QPushButton::clicked, this,
            &MainWindow::deleteCompany);

    // Create tab widget
    tabWidget = new QTabWidget(this);  // NOLINT

    // Main widget with company selector and tabs
    auto* centralWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(companyWidget);
    mainLayout->addWidget(tabWidget);

    setCentralWidget(centralWidget);

    setupEmployeeTab();
    setupProjectTab();
    setupCompanyInfoTab();
    setupStatisticsTab();
}

void MainWindow::setupEmployeeTab() {
    employeeTab = new QWidget();  // NOLINT

    auto* mainLayout = new QVBoxLayout(employeeTab);
    mainLayout->setSpacing(kLayoutSpacing);
    mainLayout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                                    kLayoutMargins);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    searchEmployeeEdit = new QLineEdit();  // NOLINT
    searchEmployeeEdit->setPlaceholderText(
        "Search employees by name, department, or position...");
    searchEmployeeEdit->setMinimumHeight(kSearchEditMinHeight);
    searchEmployeeBtn = new QPushButton("Search");  // NOLINT
    searchLayout->addWidget(searchEmployeeEdit);
    searchLayout->addWidget(searchEmployeeBtn);
    mainLayout->addLayout(searchLayout);

    // Table
    employeeTable = new QTableWidget();  // NOLINT
    employeeTable->setColumnCount(6);
    QStringList headers = {"ID",         "Name",   "Position",
                           "Department", "Salary", "Type"};
    employeeTable->setHorizontalHeaderLabels(headers);
    employeeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    employeeTable->setSelectionMode(QAbstractItemView::SingleSelection);
    employeeTable->horizontalHeader()->setStretchLastSection(true);
    employeeTable->setAlternatingRowColors(true);

    // Set column widths for better visibility
    employeeTable->setColumnWidth(0, kTableColumnWidthId);            // ID
    employeeTable->setColumnWidth(1, kTableColumnWidthName);           // Name
    employeeTable->setColumnWidth(2, kTableColumnWidthPosition);       // Position
    employeeTable->setColumnWidth(3, kTableColumnWidthDepartment);     // Department
    employeeTable->setColumnWidth(4, kTableColumnWidthSalary);         // Salary
    employeeTable->setColumnWidth(5, kTableColumnWidthType);            // Type

    // Set row height for better readability
    employeeTable->verticalHeader()->setDefaultSectionSize(kTableRowHeight);

    mainLayout->addWidget(employeeTable);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    addEmployeeBtn = new QPushButton("Add Employee");        // NOLINT
    editEmployeeBtn = new QPushButton("Edit Employee");      // NOLINT
    deleteEmployeeBtn = new QPushButton("Delete Employee");  // NOLINT
    buttonLayout->addWidget(addEmployeeBtn);
    buttonLayout->addWidget(editEmployeeBtn);
    buttonLayout->addWidget(deleteEmployeeBtn);
    mainLayout->addLayout(buttonLayout);

    connect(addEmployeeBtn, &QPushButton::clicked, this,
            &MainWindow::addEmployee);
    connect(editEmployeeBtn, &QPushButton::clicked, this,
            &MainWindow::editEmployee);
    connect(deleteEmployeeBtn, &QPushButton::clicked, this,
            &MainWindow::deleteEmployee);
    connect(searchEmployeeBtn, &QPushButton::clicked, this,
            &MainWindow::searchEmployee);

    tabWidget->addTab(employeeTab, "Employees");
}

void MainWindow::setupProjectTab() {
    projectTab = new QWidget();  // NOLINT

    auto* mainLayout = new QVBoxLayout(projectTab);
    mainLayout->setSpacing(kLayoutSpacing);
    mainLayout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                                    kLayoutMargins);

    // Search bar
    auto* searchLayout = new QHBoxLayout();
    searchProjectEdit = new QLineEdit();  // NOLINT
    searchProjectEdit->setPlaceholderText(
        "Search projects by name, status, or client...");
    searchProjectEdit->setMinimumHeight(kSearchEditMinHeight);
    searchProjectBtn = new QPushButton("Search");  // NOLINT
    searchLayout->addWidget(searchProjectEdit);
    searchLayout->addWidget(searchProjectBtn);
    mainLayout->addLayout(searchLayout);

    // Table
    projectTable = new QTableWidget();  // NOLINT
    projectTable->setColumnCount(5);
    QStringList headers = {"ID", "Name", "Status", "Budget", "Client"};
    projectTable->setHorizontalHeaderLabels(headers);
    projectTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    projectTable->setSelectionMode(QAbstractItemView::SingleSelection);
    projectTable->horizontalHeader()->setStretchLastSection(true);
    projectTable->setAlternatingRowColors(true);

    // Set column widths for better visibility
    projectTable->setColumnWidth(0, kTableColumnWidthId);           // ID
    projectTable->setColumnWidth(1, kTableColumnWidthProjectName);    // Name
    projectTable->setColumnWidth(2, kTableColumnWidthStatus);         // Status
    projectTable->setColumnWidth(3, kTableColumnWidthBudget);         // Budget
    projectTable->setColumnWidth(4, kTableColumnWidthClient);         // Client

    // Set row height for better readability
    projectTable->verticalHeader()->setDefaultSectionSize(kTableRowHeight);

    mainLayout->addWidget(projectTable);

    // Buttons
    auto* buttonLayout = new QHBoxLayout();
    addProjectBtn = new QPushButton("Add Project");        // NOLINT
    editProjectBtn = new QPushButton("Edit Project");      // NOLINT
    deleteProjectBtn = new QPushButton("Delete Project");  // NOLINT
    buttonLayout->addWidget(addProjectBtn);
    buttonLayout->addWidget(editProjectBtn);
    buttonLayout->addWidget(deleteProjectBtn);
    mainLayout->addLayout(buttonLayout);

    connect(addProjectBtn, &QPushButton::clicked, this,
            &MainWindow::addProject);
    connect(editProjectBtn, &QPushButton::clicked, this,
            &MainWindow::editProject);
    connect(deleteProjectBtn, &QPushButton::clicked, this,
            &MainWindow::deleteProject);
    connect(searchProjectBtn, &QPushButton::clicked, this,
            &MainWindow::searchProject);

    tabWidget->addTab(projectTab, "Projects");
}

void MainWindow::setupCompanyInfoTab() {
    infoTab = new QWidget();  // NOLINT
    auto* layout = new QVBoxLayout(infoTab);
    layout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                                kLayoutMargins);

    companyInfoText = new QTextEdit();  // NOLINT
    companyInfoText->setReadOnly(true);
    companyInfoText->setStyleSheet("font-size: 14px; line-height: 1.6;");
    layout->addWidget(companyInfoText);

    tabWidget->addTab(infoTab, "Company Info");
}

void MainWindow::setupStatisticsTab() {
    statsTab = new QWidget();  // NOLINT
    auto* layout = new QVBoxLayout(statsTab);
    layout->setContentsMargins(kLayoutMargins, kLayoutMargins, kLayoutMargins,
                                kLayoutMargins);

    statisticsText = new QTextEdit();  // NOLINT
    statisticsText->setReadOnly(true);
    statisticsText->setStyleSheet(
        "font-size: 14px; line-height: 1.6; background-color: white; color: "
        "black;");
    layout->addWidget(statisticsText);

    refreshStatsBtn = new QPushButton("Refresh Statistics");  // NOLINT
    layout->addWidget(refreshStatsBtn);

    connect(refreshStatsBtn, &QPushButton::clicked, this,
            &MainWindow::showStatistics);

    tabWidget->addTab(statsTab, "Statistics");
}

void MainWindow::displayEmployees() {
    if (currentCompany == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    employeeTable->setRowCount(employees.size());

    for (size_t index = 0; index < employees.size(); ++index) {
        const auto& employee = employees[index];
        employeeTable->setItem(
            index, 0, new QTableWidgetItem(QString::number(employee->getId())));
        employeeTable->setItem(index, 1,
                               new QTableWidgetItem(employee->getName()));
        employeeTable->setItem(index, 2,
                               new QTableWidgetItem(employee->getPosition()));
        employeeTable->setItem(index, 3,
                               new QTableWidgetItem(employee->getDepartment()));
        employeeTable->setItem(index, 4,
                               new QTableWidgetItem(QString::number(
                                   employee->getSalary(), 'f', 2)));
        employeeTable->setItem(
            index, 5, new QTableWidgetItem(employee->getEmployeeType()));
    }

    if (!employees.empty()) {
        nextEmployeeId = employees.back()->getId() + 1;
    }
}

void MainWindow::displayProjects() {
    if (currentCompany == nullptr) return;

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
        projectTable->setItem(index, 4,
                              new QTableWidgetItem(project.getClientName()));
    }

    if (!projects.empty()) {
        nextProjectId = projects.back().getId() + 1;
    }
}

int MainWindow::getSelectedEmployeeId() {
    int rowIndex = employeeTable->currentRow();
    if (rowIndex >= 0) {
        QTableWidgetItem* tableItem = employeeTable->item(rowIndex, 0);
        if (tableItem != nullptr) {
            bool conversionSuccess = false;
            int employeeId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? employeeId : -1;
        }
    }
    return -1;
}

int MainWindow::getSelectedProjectId() {
    int rowIndex = projectTable->currentRow();
    if (rowIndex >= 0) {
        QTableWidgetItem* tableItem = projectTable->item(rowIndex, 0);
        if (tableItem != nullptr) {
            bool conversionSuccess = false;
            int projectId = tableItem->text().toInt(&conversionSuccess);
            return conversionSuccess ? projectId : -1;
        }
    }
    return -1;
}

void MainWindow::addEmployee() {
    if (currentCompany == nullptr) return;

    QDialog dialog(this);
    dialog.setWindowTitle("Add Employee");
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);

    auto* typeCombo = new QComboBox();
    typeCombo->addItems({"Manager", "Developer", "Designer", "QA"});
    typeCombo->setStyleSheet("background-color: white;");
    form->addRow("Type:", typeCombo);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., John Doe");
    form->addRow("Name:", nameEdit);

    auto* salaryEdit = new QLineEdit();
    salaryEdit->setPlaceholderText("e.g., 5000");
    form->addRow("Salary ($):", salaryEdit);

    auto* deptEdit = new QLineEdit();
    deptEdit->setPlaceholderText("e.g., Development, Design");
    form->addRow("Department:", deptEdit);

    // Type-specific fields with separate labels for proper visibility control
    auto* managerProject = new QLineEdit();
    managerProject->setPlaceholderText("e.g., Mobile App Project");
    auto* managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(managerProjectLabel, managerProject);
    managerProjectLabel->setVisible(false);
    managerProject->setVisible(false);

    auto* managerTeamSize = new QLineEdit();
    managerTeamSize->setPlaceholderText("e.g., 5");
    auto* managerTeamSizeLabel = new QLabel("Team Size:");
    form->addRow(managerTeamSizeLabel, managerTeamSize);
    managerTeamSizeLabel->setVisible(false);
    managerTeamSize->setVisible(false);

    auto* devLanguage = new QLineEdit();
    devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    auto* devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(devLanguageLabel, devLanguage);
    devLanguageLabel->setVisible(false);
    devLanguage->setVisible(false);

    auto* devExperience = new QLineEdit();
    devExperience->setPlaceholderText("e.g., 3");
    auto* devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(devExperienceLabel, devExperience);
    devExperienceLabel->setVisible(false);
    devExperience->setVisible(false);

    auto* designerTool = new QLineEdit();
    designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    auto* designerToolLabel = new QLabel("Design Tool:");
    form->addRow(designerToolLabel, designerTool);
    designerToolLabel->setVisible(false);
    designerTool->setVisible(false);

    auto* designerProjects = new QLineEdit();
    designerProjects->setPlaceholderText("e.g., 10");
    auto* designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(designerProjectsLabel, designerProjects);
    designerProjectsLabel->setVisible(false);
    designerProjects->setVisible(false);

    auto* qaTestType = new QLineEdit();
    qaTestType->setPlaceholderText("e.g., Manual, Automated");
    auto* qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(qaTestTypeLabel, qaTestType);
    qaTestTypeLabel->setVisible(false);
    qaTestType->setVisible(false);

    auto* qaBugs = new QLineEdit();
    qaBugs->setPlaceholderText("e.g., 25");
    auto* qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(qaBugsLabel, qaBugs);
    qaBugsLabel->setVisible(false);
    qaBugs->setVisible(false);

    // Show/hide fields and labels based on type
    auto updateFields = [=](int index) {
        // Manager fields
        bool showManager = (index == 0);
        managerProjectLabel->setVisible(showManager);
        managerProject->setVisible(showManager);
        managerTeamSizeLabel->setVisible(showManager);
        managerTeamSize->setVisible(showManager);

        // Developer fields
        bool showDev = (index == 1);
        devLanguageLabel->setVisible(showDev);
        devLanguage->setVisible(showDev);
        devExperienceLabel->setVisible(showDev);
        devExperience->setVisible(showDev);

        // Designer fields
        bool showDesigner = (index == 2);
        designerToolLabel->setVisible(showDesigner);
        designerTool->setVisible(showDesigner);
        designerProjectsLabel->setVisible(showDesigner);
        designerProjects->setVisible(showDesigner);

        // QA fields
        bool showQA = (index == 3);
        qaTestTypeLabel->setVisible(showQA);
        qaTestType->setVisible(showQA);
        qaBugsLabel->setVisible(showQA);
        qaBugs->setVisible(showQA);
    };

    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            updateFields);

    // Initialize fields based on default selection (Manager = 0)
    updateFields(0);

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            QString name = nameEdit->text().trimmed();
            double salary = salaryEdit->text().toDouble();
            QString department = deptEdit->text().trimmed();

            // Validation
            if (name.isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Name cannot be empty!");
                return;
            }

            // Check for duplicate employee by name
            auto existingEmployees = currentCompany->getAllEmployees();
            for (const auto& employee : existingEmployees) {
                if (employee &&
                    employee->getName().toLower() == name.toLower()) {
                    QMessageBox::warning(
                        this, "Duplicate Error",
                        "An employee with this name already exists!");
                    return;
                }
            }

            // Validate salary range
            if (salary < kMinSalary) {
                QMessageBox::warning(
                    this, "Validation Error",
                    QString("Salary must be at least $%1!").arg(kMinSalary));
                return;
            }
            if (salary > kMaxSalary) {
                QMessageBox::warning(
                    this, "Validation Error",
                    QString("Salary cannot exceed $%1!").arg(kMaxSalary));
                return;
            }

            if (department.isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Department cannot be empty!");
                return;
            }

            // Generate ID only after all validation passes
            int employeeId = nextEmployeeId++;

            QString employeeType = typeCombo->currentText();
            std::shared_ptr<Employee> employee;

            if (employeeType == "Manager") {
                if (managerProject->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Managed project cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int managerTeamSizeValue =
                    managerTeamSize->text().toInt(&conversionSuccess);
                if (!conversionSuccess || managerTeamSizeValue < 0) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Please enter a valid team size!");
                    return;
                }
                employee = std::make_shared<Manager>(
                    employeeId, name, salary, department, managerTeamSizeValue,
                    managerProject->text().trimmed());
            } else if (employeeType == "Developer") {
                if (devLanguage->text().trimmed().isEmpty()) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Programming language cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int developerYearsOfExperience =
                    devExperience->text().toInt(&conversionSuccess);
                if (!conversionSuccess || developerYearsOfExperience < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter valid years of experience!");
                    return;
                }
                employee = std::make_shared<Developer>(
                    employeeId, name, salary, department,
                    devLanguage->text().trimmed(), developerYearsOfExperience);
            } else if (employeeType == "Designer") {
                if (designerTool->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Design tool cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int designerNumberOfProjects =
                    designerProjects->text().toInt(&conversionSuccess);
                if (!conversionSuccess || designerNumberOfProjects < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter a valid number of projects!");
                    return;
                }
                employee = std::make_shared<Designer>(
                    employeeId, name, salary, department,
                    designerTool->text().trimmed(), designerNumberOfProjects);
            } else if (employeeType == "QA") {
                if (qaTestType->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Testing type cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int qaBugsFound = qaBugs->text().toInt(&conversionSuccess);
                if (!conversionSuccess || qaBugsFound < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter a valid number of bugs!");
                    return;
                }
                employee = std::make_shared<QA>(
                    employeeId, name, salary, department,
                    qaTestType->text().trimmed(), qaBugsFound);
            }

            currentCompany->addEmployee(employee);
            displayEmployees();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after adding
            QMessageBox::information(this, "Success",
                                     "Employee added successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(
                this, "Error", QString("Failed to add employee: ") + e.what());
        }
    }
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
    if (!employee) {
        QMessageBox::warning(this, "Error", "Employee not found!");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("Edit Employee");
    dialog.setStyleSheet("QDialog { background-color: white; }");
    dialog.setMinimumWidth(400);

    auto* form = new QFormLayout(&dialog);

    // Type is read-only
    auto* typeLabel = new QLabel("Type:");
    auto* typeDisplay = new QLineEdit();
    typeDisplay->setText(employee->getEmployeeType());
    typeDisplay->setReadOnly(true);
    typeDisplay->setStyleSheet("QLineEdit { background-color: #f5f5f5; }");
    form->addRow(typeLabel, typeDisplay);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., John Doe");
    nameEdit->setText(employee->getName());
    form->addRow("Name:", nameEdit);

    auto* salaryEdit = new QLineEdit();
    salaryEdit->setPlaceholderText("e.g., 5000");
    salaryEdit->setText(QString::number(employee->getSalary(), 'f', 2));
    form->addRow("Salary ($):", salaryEdit);

    auto* deptEdit = new QLineEdit();
    deptEdit->setPlaceholderText("e.g., Development, Design");
    deptEdit->setText(employee->getDepartment());
    form->addRow("Department:", deptEdit);

    // Type-specific fields
    auto* managerProject = new QLineEdit();
    managerProject->setPlaceholderText("e.g., Mobile App Project");
    auto* managerProjectLabel = new QLabel("Managed Project:");
    form->addRow(managerProjectLabel, managerProject);

    auto* managerTeamSize = new QLineEdit();
    managerTeamSize->setPlaceholderText("e.g., 5");
    auto* managerTeamSizeLabel = new QLabel("Team Size:");
    form->addRow(managerTeamSizeLabel, managerTeamSize);

    auto* devLanguage = new QLineEdit();
    devLanguage->setPlaceholderText("e.g., C++, Java, Python");
    auto* devLanguageLabel = new QLabel("Programming Language:");
    form->addRow(devLanguageLabel, devLanguage);

    auto* devExperience = new QLineEdit();
    devExperience->setPlaceholderText("e.g., 3");
    auto* devExperienceLabel = new QLabel("Years of Experience:");
    form->addRow(devExperienceLabel, devExperience);

    auto* designerTool = new QLineEdit();
    designerTool->setPlaceholderText("e.g., Figma, Adobe XD");
    auto* designerToolLabel = new QLabel("Design Tool:");
    form->addRow(designerToolLabel, designerTool);

    auto* designerProjects = new QLineEdit();
    designerProjects->setPlaceholderText("e.g., 10");
    auto* designerProjectsLabel = new QLabel("Number of Projects:");
    form->addRow(designerProjectsLabel, designerProjects);

    auto* qaTestType = new QLineEdit();
    qaTestType->setPlaceholderText("e.g., Manual, Automated");
    auto* qaTestTypeLabel = new QLabel("Testing Type:");
    form->addRow(qaTestTypeLabel, qaTestType);

    auto* qaBugs = new QLineEdit();
    qaBugs->setPlaceholderText("e.g., 25");
    auto* qaBugsLabel = new QLabel("Bugs Found:");
    form->addRow(qaBugsLabel, qaBugs);

    // Populate and show/hide fields based on employee type
    QString currentType = employee->getEmployeeType();
    if (currentType == "Manager") {
        auto* manager = dynamic_cast<Manager*>(employee.get());
        if (manager != nullptr) {
            managerProject->setText(manager->getProjectManaged());
            managerTeamSize->setText(QString::number(manager->getTeamSize()));
        }
        managerProjectLabel->setVisible(true);
        managerProject->setVisible(true);
        managerTeamSizeLabel->setVisible(true);
        managerTeamSize->setVisible(true);

        devLanguageLabel->setVisible(false);
        devLanguage->setVisible(false);
        devExperienceLabel->setVisible(false);
        devExperience->setVisible(false);
        designerToolLabel->setVisible(false);
        designerTool->setVisible(false);
        designerProjectsLabel->setVisible(false);
        designerProjects->setVisible(false);
        qaTestTypeLabel->setVisible(false);
        qaTestType->setVisible(false);
        qaBugsLabel->setVisible(false);
        qaBugs->setVisible(false);
    } else if (currentType == "Developer") {
        auto* developer = dynamic_cast<Developer*>(employee.get());
        if (developer != nullptr) {
            devLanguage->setText(developer->getProgrammingLanguage());
            devExperience->setText(
                QString::number(developer->getYearsOfExperience()));
        }
        devLanguageLabel->setVisible(true);
        devLanguage->setVisible(true);
        devExperienceLabel->setVisible(true);
        devExperience->setVisible(true);

        managerProjectLabel->setVisible(false);
        managerProject->setVisible(false);
        managerTeamSizeLabel->setVisible(false);
        managerTeamSize->setVisible(false);
        designerToolLabel->setVisible(false);
        designerTool->setVisible(false);
        designerProjectsLabel->setVisible(false);
        designerProjects->setVisible(false);
        qaTestTypeLabel->setVisible(false);
        qaTestType->setVisible(false);
        qaBugsLabel->setVisible(false);
        qaBugs->setVisible(false);
    } else if (currentType == "Designer") {
        auto* designer = dynamic_cast<Designer*>(employee.get());
        if (designer != nullptr) {
            designerTool->setText(designer->getDesignTool());
            designerProjects->setText(
                QString::number(designer->getNumberOfProjects()));
        }
        designerToolLabel->setVisible(true);
        designerTool->setVisible(true);
        designerProjectsLabel->setVisible(true);
        designerProjects->setVisible(true);

        managerProjectLabel->setVisible(false);
        managerProject->setVisible(false);
        managerTeamSizeLabel->setVisible(false);
        managerTeamSize->setVisible(false);
        devLanguageLabel->setVisible(false);
        devLanguage->setVisible(false);
        devExperienceLabel->setVisible(false);
        devExperience->setVisible(false);
        qaTestTypeLabel->setVisible(false);
        qaTestType->setVisible(false);
        qaBugsLabel->setVisible(false);
        qaBugs->setVisible(false);
    } else if (currentType == "QA") {
        auto* qaEmployee = dynamic_cast<QA*>(employee.get());
        if (qaEmployee != nullptr) {
            qaTestType->setText(qaEmployee->getTestingType());
            qaBugs->setText(QString::number(qaEmployee->getBugsFound()));
        }
        qaTestTypeLabel->setVisible(true);
        qaTestType->setVisible(true);
        qaBugsLabel->setVisible(true);
        qaBugs->setVisible(true);

        managerProjectLabel->setVisible(false);
        managerProject->setVisible(false);
        managerTeamSizeLabel->setVisible(false);
        managerTeamSize->setVisible(false);
        devLanguageLabel->setVisible(false);
        devLanguage->setVisible(false);
        devExperienceLabel->setVisible(false);
        devExperience->setVisible(false);
        designerToolLabel->setVisible(false);
        designerTool->setVisible(false);
        designerProjectsLabel->setVisible(false);
        designerProjects->setVisible(false);
    }

    auto* okButton = new QPushButton("OK");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        try {
            QString name = nameEdit->text().trimmed();
            double salary = salaryEdit->text().toDouble();
            QString department = deptEdit->text().trimmed();

            // Validation
            if (name.isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Name cannot be empty!");
                return;
            }

            // Check for duplicate name (excluding current employee)
            auto existingEmployees = currentCompany->getAllEmployees();
            for (const auto& employee : existingEmployees) {
                if (employee && employee->getId() != employeeId &&
                    employee->getName().toLower() == name.toLower()) {
                    QMessageBox::warning(
                        this, "Duplicate Error",
                        "An employee with this name already exists!");
                    return;
                }
            }

            // Validate salary range
            if (salary < kMinSalary) {
                QMessageBox::warning(
                    this, "Validation Error",
                    QString("Salary must be at least $%1!").arg(kMinSalary));
                return;
            }
            if (salary > kMaxSalary) {
                QMessageBox::warning(
                    this, "Validation Error",
                    QString("Salary cannot exceed $%1!").arg(kMaxSalary));
                return;
            }

            if (department.isEmpty()) {
                QMessageBox::warning(this, "Validation Error",
                                     "Department cannot be empty!");
                return;
            }

            std::shared_ptr<Employee> updatedEmployee;

            if (currentType == "Manager") {
                if (managerProject->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Managed project cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int managerTeamSizeValue =
                    managerTeamSize->text().toInt(&conversionSuccess);
                if (!conversionSuccess || managerTeamSizeValue < 0) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Please enter a valid team size!");
                    return;
                }
                updatedEmployee = std::make_shared<Manager>(
                    employeeId, name, salary, department, managerTeamSizeValue,
                    managerProject->text().trimmed());
            } else if (currentType == "Developer") {
                if (devLanguage->text().trimmed().isEmpty()) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Programming language cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int developerYearsOfExperience =
                    devExperience->text().toInt(&conversionSuccess);
                if (!conversionSuccess || developerYearsOfExperience < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter valid years of experience!");
                    return;
                }
                updatedEmployee = std::make_shared<Developer>(
                    employeeId, name, salary, department,
                    devLanguage->text().trimmed(), developerYearsOfExperience);
            } else if (currentType == "Designer") {
                if (designerTool->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Design tool cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int designerNumberOfProjects =
                    designerProjects->text().toInt(&conversionSuccess);
                if (!conversionSuccess || designerNumberOfProjects < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter a valid number of projects!");
                    return;
                }
                updatedEmployee = std::make_shared<Designer>(
                    employeeId, name, salary, department,
                    designerTool->text().trimmed(), designerNumberOfProjects);
            } else if (currentType == "QA") {
                if (qaTestType->text().trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Validation Error",
                                         "Testing type cannot be empty!");
                    return;
                }
                bool conversionSuccess = false;
                int qaBugsFound = qaBugs->text().toInt(&conversionSuccess);
                if (!conversionSuccess || qaBugsFound < 0) {
                    QMessageBox::warning(
                        this, "Validation Error",
                        "Please enter a valid number of bugs!");
                    return;
                }
                updatedEmployee = std::make_shared<QA>(
                    employeeId, name, salary, department,
                    qaTestType->text().trimmed(), qaBugsFound);
            }

            // Remove old employee and add updated one
            currentCompany->removeEmployee(employeeId);
            currentCompany->addEmployee(updatedEmployee);

            displayEmployees();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after editing
            QMessageBox::information(this, "Success",
                                     "Employee updated successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to update employee: ") + e.what());
        }
    }
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
            displayEmployees();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after deletion
            QMessageBox::information(this, "Success",
                                     "Employee deleted successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete employee: ") + e.what());
        }
    }
}

void MainWindow::searchEmployee() {
    if (currentCompany == nullptr) return;

    QString searchTerm = searchEmployeeEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        displayEmployees();
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
                rowIndex, 2, new QTableWidgetItem(employee->getPosition()));
            employeeTable->setItem(
                rowIndex, 3, new QTableWidgetItem(employee->getDepartment()));
            employeeTable->setItem(rowIndex, 4,
                                   new QTableWidgetItem(QString::number(
                                       employee->getSalary(), 'f', 2)));
            employeeTable->setItem(
                rowIndex, 5, new QTableWidgetItem(employee->getEmployeeType()));
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
            [=](int index) {
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
            auto existingProjects = currentCompany->getAllProjects();
            for (const auto& project : existingProjects) {
                if (project.getName().toLower() == projectName.toLower()) {
                    QMessageBox::warning(
                        this, "Duplicate Error",
                        "A project with this name already exists!");
                    return;
                }
            }

            // Generate ID only after all validation passes
            int projectId = nextProjectId++;
            Project project(projectId, nameEdit->text().trimmed(),
                            descEdit->toPlainText().trimmed(),
                            statusCombo->currentText(), startDate->date(),
                            endDate->date(), projectBudget,
                            clientNameEdit->text().trimmed());

            currentCompany->addProject(project);
            displayProjects();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after adding
            QMessageBox::information(this, "Success",
                                     "Project added successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(this, "Error",
                                 QString("Failed to add project: ") + e.what());
        }
    }
}

void MainWindow::editProject() {
    if (currentCompany == nullptr) return;

    int projectId = getSelectedProjectId();
    if (projectId < 0) {
        QMessageBox::warning(this, "Error", "Please select a project to edit.");
        return;
    }

    auto* project = currentCompany->getProject(projectId);
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
            auto existingProjects = currentCompany->getAllProjects();
            for (const auto& existingProject : existingProjects) {
                if (existingProject.getId() != projectId &&
                    existingProject.getName().toLower() ==
                        projectName.toLower()) {
                    QMessageBox::warning(
                        this, "Duplicate Error",
                        "A project with this name already exists!");
                    return;
                }
            }

            // Create updated project
            Project updatedProject(
                projectId, nameEdit->text().trimmed(),
                descEdit->toPlainText().trimmed(), statusCombo->currentText(),
                startDate->date(), endDate->date(), projectBudget,
                clientNameEdit->text().trimmed());

            // Remove old project and add updated one
            currentCompany->removeProject(projectId);
            currentCompany->addProject(updatedProject);

            displayProjects();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after editing
            QMessageBox::information(this, "Success",
                                     "Project updated successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to update project: ") + e.what());
        }
    }
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
            displayProjects();
            showCompanyInfo();
            showStatistics();
            autoSave();  // Automatically save after deletion
            QMessageBox::information(this, "Success",
                                     "Project deleted successfully!");
        } catch (const std::exception& e) {
            QMessageBox::warning(
                this, "Error",
                QString("Failed to delete project: ") + e.what());
        }
    }
}

void MainWindow::searchProject() {
    if (currentCompany == nullptr) return;

    QString searchTerm = searchProjectEdit->text().toLower();
    if (searchTerm.isEmpty()) {
        displayProjects();
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
            projectTable->setItem(
                rowIndex, 4, new QTableWidgetItem(project.getClientName()));
            rowIndex++;
        }
    }
}

void MainWindow::saveData() {
    if (companies.empty()) return;

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
        FileManager::saveCompanies(companies, filepath);
        QMessageBox::information(
            this, "Success",
            QString("Data saved successfully to:\n") + filepath +
                QString("\n\nCompanies saved: %1").arg(companies.size()));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to save: ") + e.what());
    }
}

void MainWindow::autoSave() {
    if (companies.empty()) return;

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
        FileManager::saveCompanies(companies, filepath);
        // Silent save - no message box
    } catch (const std::exception& e) {
        // Silently fail - don't interrupt user workflow
        // Could optionally log to console in debug mode
        (void)e;  // Suppress unused variable warning
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
        for (auto* company : companies) {
            delete company;
        }
        companies.clear();

        // Set loaded companies
        companies = loadedCompanies;

        // Set first company as current if available
        if (!companies.empty()) {
            currentCompanyIndex = 0;
            currentCompany = companies[0];
        } else {
            currentCompany = nullptr;
            currentCompanyIndex = -1;
        }

        // Update company selector
        refreshCompanyList();

        if (currentCompany != nullptr) {
            displayEmployees();
            displayProjects();
            showCompanyInfo();
            showStatistics();
        }

        QMessageBox::information(
            this, "Success",
            QString("Data loaded successfully!\nCompanies loaded: %1")
                .arg(companies.size()));
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Error",
                             QString("Failed to load: ") + e.what());
    }
}

void MainWindow::showCompanyInfo() {
    if (currentCompany == nullptr) return;

    companyInfoText->setPlainText(currentCompany->getCompanyInfo());
}

void MainWindow::showStatistics() {
    if (currentCompany == nullptr) return;

    auto employees = currentCompany->getAllEmployees();
    double totalSalaries = currentCompany->getTotalSalaries();
    double totalBudget = currentCompany->getTotalBudget();
    double avgSalary =
        !employees.empty() ? totalSalaries / employees.size() : 0;

    QString stats = QString("╔═══════════════════════════════════╗\n")
                        .append(
                            "║                          COMPANY STATISTICS     "
                            "                  ║\n")
                        .append("╚═══════════════════════════════════╝\n\n")
                        .append(QString("%1: %2\n")
                                    .arg("Total Employees", -18)
                                    .arg(currentCompany->getEmployeeCount()))
                        .append(QString("%1: %2\n")
                                    .arg("Total Projects", -18)
                                    .arg(currentCompany->getProjectCount()))
                        .append(QString("%1: $%2\n")
                                    .arg("Total Salaries", -18)
                                    .arg(totalSalaries, 0, 'f', 2))
                        .append(QString("%1: $%2\n")
                                    .arg("Average Salary", -18)
                                    .arg(avgSalary, 0, 'f', 2))
                        .append(QString("%1: $%2\n\n")
                                    .arg("Total Budget", -18)
                                    .arg(totalBudget, 0, 'f', 2))
                        .append("═══════════════════════════\n")
                        .append("Employees by Type:\n")
                        .append("═══════════════════════════\n");

    std::map<QString, int> employeeTypeCount;
    for (const auto& employee : employees) {
        employeeTypeCount[employee->getEmployeeType()]++;
    }

    for (const auto& typeCountPair : employeeTypeCount) {
        stats.append(QString("\n%1: %2")
                         .arg(typeCountPair.first, -18)
                         .arg(typeCountPair.second));
    }

    statisticsText->setPlainText(stats);
}

void MainWindow::refreshEmployeeTable() { displayEmployees(); }

void MainWindow::refreshProjectTable() { displayProjects(); }

void MainWindow::refreshCompanyList() {
    if (companySelector != nullptr) {
        companySelector->clear();
        for (auto* company : companies) {
            if (company != nullptr) {
                companySelector->addItem(company->getName());
            }
        }
        if (currentCompanyIndex >= 0 &&
            currentCompanyIndex < (int)companies.size()) {
            companySelector->setCurrentIndex(currentCompanyIndex);
        }
    }
}

void MainWindow::addCompany() {
    QDialog dialog(this);
    dialog.setWindowTitle("Add New Company");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., Google");
    nameEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Company Name:", nameEdit);

    auto* industryEdit = new QLineEdit();
    industryEdit->setPlaceholderText("e.g., Software Development");
    industryEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Industry:", industryEdit);

    auto* locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText("e.g., Mountain View, USA");
    locationEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Location:", locationEdit);

    auto* yearEdit = new QLineEdit();
    yearEdit->setPlaceholderText("e.g., 1998");
    yearEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Founded Year:", yearEdit);

    auto* okButton = new QPushButton("Create");
    form->addRow(okButton);
    connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        // Validation
        if (companyName.isEmpty()) {
            QMessageBox::warning(this, "Error",
                                 "Company name cannot be empty!");
            return;
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(this, "Error", "Industry cannot be empty!");
            return;
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(this, "Error", "Location cannot be empty!");
            return;
        }
        if (!conversionSuccess || foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(this, "Error", "Please enter a valid year!");
            return;
        }

        currentCompany = new Company(companyName, companyIndustry,
                                     companyLocation, foundedYear);
        companies.push_back(currentCompany);
        currentCompanyIndex = companies.size() - 1;

        refreshCompanyList();
        displayEmployees();
        displayProjects();
        showCompanyInfo();
        showStatistics();

        autoSave();  // Save new company to file

        QMessageBox::information(this, "Success",
                                 "Company added successfully!");
    }
}

void MainWindow::switchCompany() {
    if (companySelector != nullptr && currentCompanyIndex >= 0) {
        // Save previous company before switching
        if (currentCompany != nullptr) {
            autoSave();
        }

        int newIndex = companySelector->currentIndex();
        if (newIndex >= 0 && newIndex < (int)companies.size()) {
            currentCompany = companies[newIndex];
            currentCompanyIndex = newIndex;
            displayEmployees();
            displayProjects();
            showCompanyInfo();
            showStatistics();
        }
    }
}

void MainWindow::deleteCompany() {
    if (companies.size() <= 1) {
        QMessageBox::warning(this, "Error",
                             "Cannot delete the only remaining company!");
        return;
    }

    int ret = QMessageBox::question(
        this, "Confirm Delete", "Are you sure you want to delete this company?",
        QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        delete currentCompany;
        companies.erase(companies.begin() + currentCompanyIndex);

        // Switch to first company if available
        if (!companies.empty()) {
            currentCompanyIndex = 0;
            currentCompany = companies[0];
        } else {
            currentCompany = nullptr;
            currentCompanyIndex = -1;
        }

        refreshCompanyList();
        displayEmployees();
        displayProjects();
        showCompanyInfo();
        showStatistics();

        // Save remaining company after deletion
        if (currentCompany != nullptr) {
            autoSave();
        }

        QMessageBox::information(this, "Success",
                                 "Company deleted successfully!");
    }
}
