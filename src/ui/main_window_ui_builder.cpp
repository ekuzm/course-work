#include "ui/main_window_ui_builder.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSizePolicy>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "ui/main_window.h"
#include "ui/main_window_helpers.h"
#include "ui/main_window_operations.h"
#include "utils/app_styles.h"
#include "utils/consts.h"

void MainWindowUIBuilder::setupMainUI(MainWindow* window) {
    if (!window) return;

    window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    window->setMinimumSize(900, 700);
    window->setStyleSheet(UIStyleHelper::getMainWindowStylesheet());

    window->companyUI.widget = new QWidget();
    auto* companyLayout = new QHBoxLayout(window->companyUI.widget);
    companyLayout->setContentsMargins(
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins,
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins);

    auto* companyLabel = new QLabel("Current Company:");
    window->companyUI.selector = new QComboBox();
    window->companyUI.selector->setMinimumWidth(kCompanySelectorMinWidth);
    window->companyUI.selector->setStyleSheet(
        UIStyleHelper::getCompanyComboBoxStylesheet());

    window->companyUI.addBtn = new QPushButton("Add");
    window->companyUI.deleteBtn = new QPushButton("Delete");

    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(window->companyUI.selector);
    companyLayout->addWidget(window->companyUI.addBtn);
    companyLayout->addWidget(window->companyUI.deleteBtn);
    companyLayout->addStretch();

    QObject::connect(
        window->companyUI.selector,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
        [window](int) { CompanyOperations::switchCompany(window); });
    QObject::connect(window->companyUI.addBtn, &QPushButton::clicked,
                     [window]() { CompanyOperations::addCompany(window); });
    QObject::connect(window->companyUI.deleteBtn, &QPushButton::clicked,
                     [window]() { CompanyOperations::deleteCompany(window); });

    window->mainTabWidget = new QTabWidget(window);

    auto* centralWidget = new QWidget();
    auto* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(window->companyUI.widget);
    mainLayout->addWidget(window->mainTabWidget);

    window->setCentralWidget(centralWidget);

    setupEmployeeTab(window, window->mainTabWidget);
    setupProjectTab(window, window->mainTabWidget);
    setupStatisticsTab(window, window->mainTabWidget);
}

QWidget* MainWindowUIBuilder::createCompanyWidget(MainWindow* window) {
    return window ? window->companyUI.widget : nullptr;
}

void MainWindowUIBuilder::setupEmployeeTab(MainWindow* window,
                                           QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->employeeUI.tab = new QWidget();
    auto* mainLayout = new QVBoxLayout(window->employeeUI.tab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);

    window->employeeUI.searchEdit = new QLineEdit();
    window->employeeUI.searchEdit->setPlaceholderText("🔍 Search...");
    window->employeeUI.searchEdit->setMinimumHeight(40);

    window->employeeUI.addBtn = new QPushButton("➕ Add Employee");
    window->employeeUI.addBtn->setMinimumWidth(150);
    window->employeeUI.addBtn->setMinimumHeight(40);

    toolbarLayout->addWidget(window->employeeUI.searchEdit, 1);
    toolbarLayout->addWidget(window->employeeUI.addBtn);

    mainLayout->addLayout(toolbarLayout);

    window->employeeUI.table = new QTableWidget();
    auto headers = QStringList{"ID",   "Name",    "Department", "Salary",
                           "Type", "Project", "Actions"};
    auto columnWidths = QList<int>{110, 280, 250, 210, 250, 590, 10};
    MainWindowUIHelper::setupTableWidget(window->employeeUI.table, headers,
                                         columnWidths);

    mainLayout->addWidget(window->employeeUI.table);

    QObject::connect(window->employeeUI.addBtn, &QPushButton::clicked,
                     [window]() { EmployeeOperations::addEmployee(window); });
    QObject::connect(window->employeeUI.searchEdit, &QLineEdit::textChanged,
                     [window](const QString&) {
                         EmployeeOperations::searchEmployee(window);
                     });

    tabWidget->addTab(window->employeeUI.tab, "Employees");
}

void MainWindowUIBuilder::setupProjectTab(MainWindow* window,
                                          QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->projectUI.tab = new QWidget();
    auto* mainLayout = new QVBoxLayout(window->projectUI.tab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    auto* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(10);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->addStretch();

    window->projectUI.addBtn = new QPushButton("➕ Add Project");
    window->projectUI.addBtn->setMinimumWidth(160);
    window->projectUI.addBtn->setMinimumHeight(42);
    actionsLayout->addWidget(window->projectUI.addBtn);

    window->projectUI.table = new QTableWidget();
    auto headers =
        QStringList{"ID",         "Name",         "Phase",  "Budget",
                           "Est. Hours", "Alloc. Hours", "Client", "Actions"};
    auto columnWidths = QList<int>{110, 340, 240, 220, 240, 240, 300, 100};
    MainWindowUIHelper::setupTableWidget(window->projectUI.table, headers,
                                         columnWidths);

    window->projectUI.listContainer = new QWidget();
    auto* projectListLayout = new QVBoxLayout(window->projectUI.listContainer);
    projectListLayout->setSpacing(20);
    projectListLayout->setContentsMargins(0, 0, 0, 0);
    projectListLayout->addLayout(actionsLayout);
    projectListLayout->addWidget(window->projectUI.table);

    mainLayout->addWidget(window->projectUI.listContainer);

    window->projectUI.detailContainer = new QWidget();
    auto* projectDetailLayout =
        new QVBoxLayout(window->projectUI.detailContainer);
    projectDetailLayout->setSpacing(18);
    projectDetailLayout->setContentsMargins(0, 0, 0, 0);

    window->projectUI.detailHeaderContainer =
        new QWidget(window->projectUI.detailContainer);
    auto* detailHeaderLayout =
        new QHBoxLayout(window->projectUI.detailHeaderContainer);
    detailHeaderLayout->setSpacing(12);
    detailHeaderLayout->setContentsMargins(0, 0, 0, 0);

    window->projectUI.detailTitle = new QLabel("Current Project");
    window->projectUI.detailTitle->setStyleSheet(
        "font-size: 20px; font-weight: 700; color: #1a1a1a;");
    detailHeaderLayout->addWidget(window->projectUI.detailTitle);

    detailHeaderLayout->addStretch();

    window->projectUI.detailAutoAssignBtn = new QPushButton("Auto Assign");
    window->projectUI.detailAutoAssignBtn->setMinimumWidth(150);
    window->projectUI.detailAutoAssignBtn->setMinimumHeight(42);
    detailHeaderLayout->addWidget(window->projectUI.detailAutoAssignBtn);

    window->projectUI.detailCloseBtn = new QPushButton("Close");
    window->projectUI.detailCloseBtn->setMinimumWidth(110);
    window->projectUI.detailCloseBtn->setMinimumHeight(40);
    detailHeaderLayout->addWidget(window->projectUI.detailCloseBtn);

    projectDetailLayout->addWidget(window->projectUI.detailHeaderContainer);

    window->projectUI.detailInfoText = new QTextEdit();
    window->projectUI.detailInfoText->setReadOnly(true);
    window->projectUI.detailInfoText->setStyleSheet(
        "QTextEdit { font-size: 14px; line-height: 1.65; padding: 0; border: "
        "none; "
        "background-color: transparent; }");
    window->projectUI.detailInfoText->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    window->projectUI.detailInfoText->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    window->projectUI.detailInfoText->setMinimumHeight(400);
    window->projectUI.detailInfoText->setFrameShape(QFrame::NoFrame);
    window->projectUI.detailInfoText->setVisible(false);
    projectDetailLayout->addWidget(window->projectUI.detailInfoText);

    window->projectUI.tasksTable = new QTableWidget();
    auto taskHeaders = QStringList{
        "Name",  "Task Type", "Priority", "Estimated Hours", "Allocated Hours",
        "Assign"};
    auto taskColumnWidths = QList<int>{340, 260, 200, 260, 260, 100};
    MainWindowUIHelper::setupTableWidget(window->projectUI.tasksTable,
                                         taskHeaders, taskColumnWidths);
    window->projectUI.tasksTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    window->projectUI.tasksTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    window->projectUI.tasksTable->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
    window->projectUI.tasksTable->verticalHeader()->setDefaultSectionSize(56);

    projectDetailLayout->addWidget(window->projectUI.tasksTable);

    window->projectUI.detailContainer->setVisible(false);

    mainLayout->addWidget(window->projectUI.detailContainer);

    QObject::connect(window->projectUI.addBtn, &QPushButton::clicked,
                     [window]() { ProjectOperations::addProject(window); });
    QObject::connect(
        window->projectUI.detailCloseBtn, &QPushButton::clicked,
        [window]() { ProjectOperations::closeProjectDetails(window); });
    QObject::connect(
        window->projectUI.detailAutoAssignBtn, &QPushButton::clicked,
        [window]() { ProjectOperations::autoAssignDetailedProject(window); });

    tabWidget->addTab(window->projectUI.tab, "Projects");
}

void MainWindowUIBuilder::setupStatisticsTab(MainWindow* window,
                                             QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->statisticsUI.tab = new QWidget();
    auto* layout = new QVBoxLayout(window->statisticsUI.tab);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(25);

    auto* titleLabel = new QLabel("📊 Company Statistics");
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; padding: 10px; color: #000000;");
    layout->addWidget(titleLabel);

    window->statisticsUI.chartWidget = new QWidget();
    window->statisticsUI.chartWidget->setMinimumHeight(400);
    window->statisticsUI.chartWidget->setStyleSheet(
        "background-color: white; border: 1px solid #e3f2fd; border-radius: "
        "10px;");
    layout->addWidget(window->statisticsUI.chartWidget, 1);

    window->statisticsUI.text = new QTextEdit();
    window->statisticsUI.text->setReadOnly(true);
    window->statisticsUI.text->setStyleSheet(
        "font-size: 15px; line-height: 2.0; padding: 25px; border-radius: "
        "10px;");
    layout->addWidget(window->statisticsUI.text, 1);

    tabWidget->addTab(window->statisticsUI.tab, "Statistics");

    QObject::connect(tabWidget, &QTabWidget::currentChanged,
                     [window, tabWidget](int index) {
                         if (window && tabWidget) {
                             QString tabText = tabWidget->tabText(index);
                             if (tabText == "Statistics") {
                                 ProjectOperations::showStatistics(window);
                             }
                         }
                     });
}
