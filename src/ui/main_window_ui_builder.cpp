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

#include "utils/app_styles.h"
#include "utils/consts.h"
#include "ui/main_window.h"

void MainWindowUIBuilder::setupMainUI(MainWindow* window) {
    if (!window) return;

    window->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    window->setMinimumSize(900, 700);
    window->setStyleSheet(UIStyleHelper::getMainWindowStylesheet());

    window->companyWidget = new QWidget();
    QHBoxLayout* companyLayout = new QHBoxLayout(window->companyWidget);
    companyLayout->setContentsMargins(
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins,
        kCompanyLayoutMargins, kCompanyLayoutVerticalMargins);

    QLabel* companyLabel = new QLabel("Current Company:");
    window->companySelector = new QComboBox();
    window->companySelector->setMinimumWidth(kCompanySelectorMinWidth);
    window->companySelector->setStyleSheet(
        UIStyleHelper::getCompanyComboBoxStylesheet());

    window->companyAddBtn = new QPushButton("Add");
    window->companyDeleteBtn = new QPushButton("Delete");

    companyLayout->addWidget(companyLabel);
    companyLayout->addWidget(window->companySelector);
    companyLayout->addWidget(window->companyAddBtn);
    companyLayout->addWidget(window->companyDeleteBtn);
    companyLayout->addStretch();

    QObject::connect(window->companySelector,
                     QOverload<int>::of(&QComboBox::currentIndexChanged),
                     window, &MainWindow::switchCompany);
    QObject::connect(window->companyAddBtn, &QPushButton::clicked, window,
                     &MainWindow::addCompany);
    QObject::connect(window->companyDeleteBtn, &QPushButton::clicked, window,
                     &MainWindow::deleteCompany);

    window->mainTabWidget = new QTabWidget(window);

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(window->companyWidget);
    mainLayout->addWidget(window->mainTabWidget);

    window->setCentralWidget(centralWidget);

    setupEmployeeTab(window, window->mainTabWidget);
    setupProjectTab(window, window->mainTabWidget);
    setupStatisticsTab(window, window->mainTabWidget);
}

QWidget* MainWindowUIBuilder::createCompanyWidget(MainWindow* window) {
    return window ? window->companyWidget : nullptr;
}

void MainWindowUIBuilder::setupEmployeeTab(MainWindow* window,
                                           QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->employeeTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(window->employeeTab);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QHBoxLayout* toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(10);

    window->employeeSearchEdit = new QLineEdit();
    window->employeeSearchEdit->setPlaceholderText("🔍 Search...");
    window->employeeSearchEdit->setMinimumHeight(40);

    window->employeeAddBtn = new QPushButton("➕ Add Employee");
    window->employeeAddBtn->setMinimumWidth(150);
    window->employeeAddBtn->setMinimumHeight(40);

    toolbarLayout->addWidget(window->employeeSearchEdit, 1);
    toolbarLayout->addWidget(window->employeeAddBtn);

    mainLayout->addLayout(toolbarLayout);

    window->employeeTable = new QTableWidget();
    QStringList headers = {"ID",   "Name",    "Department", "Salary",
                           "Type", "Project", "Actions"};
    QList<int> columnWidths = {110, 280, 250, 210, 250, 590, 10};
    window->setupTableWidget(window->employeeTable, headers, columnWidths);

    mainLayout->addWidget(window->employeeTable);

    QObject::connect(window->employeeAddBtn, &QPushButton::clicked, window,
                     &MainWindow::addEmployee);
    QObject::connect(window->employeeSearchEdit, &QLineEdit::textChanged, window,
                     &MainWindow::searchEmployee);

    tabWidget->addTab(window->employeeTab, "Employees");
}

void MainWindowUIBuilder::setupProjectTab(MainWindow* window,
                                          QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->projectTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(window->projectTab);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QHBoxLayout* actionsLayout = new QHBoxLayout();
    actionsLayout->setSpacing(10);
    actionsLayout->setContentsMargins(0, 0, 0, 0);
    actionsLayout->addStretch();

    window->projectAddBtn = new QPushButton("➕ Add Project");
    window->projectAddBtn->setMinimumWidth(160);
    window->projectAddBtn->setMinimumHeight(42);
    actionsLayout->addWidget(window->projectAddBtn);

    window->projectTable = new QTableWidget();
    QStringList headers = {"ID",         "Name",         "Phase",  "Budget",
                           "Est. Hours", "Alloc. Hours", "Client", "Actions"};
    QList<int> columnWidths = {110, 340, 240, 220, 240, 240, 300, 100};
    window->setupTableWidget(window->projectTable, headers, columnWidths);

    window->projectListContainer = new QWidget();
    QVBoxLayout* projectListLayout =
        new QVBoxLayout(window->projectListContainer);
    projectListLayout->setSpacing(20);
    projectListLayout->setContentsMargins(0, 0, 0, 0);
    projectListLayout->addLayout(actionsLayout);
    projectListLayout->addWidget(window->projectTable);

    mainLayout->addWidget(window->projectListContainer);

    window->projectDetailContainer = new QWidget();
    QVBoxLayout* projectDetailLayout =
        new QVBoxLayout(window->projectDetailContainer);
    projectDetailLayout->setSpacing(18);
    projectDetailLayout->setContentsMargins(0, 0, 0, 0);

    window->projectDetailHeaderContainer =
        new QWidget(window->projectDetailContainer);
    QHBoxLayout* detailHeaderLayout =
        new QHBoxLayout(window->projectDetailHeaderContainer);
    detailHeaderLayout->setSpacing(12);
    detailHeaderLayout->setContentsMargins(0, 0, 0, 0);

    window->projectDetailTitle = new QLabel("Current Project");
    window->projectDetailTitle->setStyleSheet(
        "font-size: 20px; font-weight: 700; color: #1a1a1a;");
    detailHeaderLayout->addWidget(window->projectDetailTitle);

    detailHeaderLayout->addStretch();

    window->projectDetailAutoAssignBtn = new QPushButton("Auto Assign");
    window->projectDetailAutoAssignBtn->setMinimumWidth(150);
    window->projectDetailAutoAssignBtn->setMinimumHeight(42);
    detailHeaderLayout->addWidget(window->projectDetailAutoAssignBtn);

    window->projectDetailCloseBtn = new QPushButton("Close");
    window->projectDetailCloseBtn->setMinimumWidth(110);
    window->projectDetailCloseBtn->setMinimumHeight(40);
    detailHeaderLayout->addWidget(window->projectDetailCloseBtn);

    projectDetailLayout->addWidget(window->projectDetailHeaderContainer);

    window->projectDetailInfoText = new QTextEdit();
    window->projectDetailInfoText->setReadOnly(true);
    window->projectDetailInfoText->setStyleSheet(
        "QTextEdit { font-size: 14px; line-height: 1.65; padding: 0; border: "
        "none; "
        "background-color: transparent; }");
    window->projectDetailInfoText->setVerticalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    window->projectDetailInfoText->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAsNeeded);
    window->projectDetailInfoText->setMinimumHeight(400);
    window->projectDetailInfoText->setFrameShape(QFrame::NoFrame);
    window->projectDetailInfoText->setVisible(false);
    projectDetailLayout->addWidget(window->projectDetailInfoText);

    window->projectTasksTable = new QTableWidget();
    QStringList taskHeaders = {
        "Name",  "Task Type", "Priority", "Estimated Hours", "Allocated Hours",
        "Assign"};
    QList<int> taskColumnWidths = {340, 260, 200, 260, 260, 100};
    window->setupTableWidget(window->projectTasksTable, taskHeaders,
                             taskColumnWidths);
    window->projectTasksTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers);
    window->projectTasksTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch);
    window->projectTasksTable->setSizePolicy(QSizePolicy::Expanding,
                                             QSizePolicy::Expanding);
    window->projectTasksTable->verticalHeader()->setDefaultSectionSize(56);

    projectDetailLayout->addWidget(window->projectTasksTable);

    window->projectDetailContainer->setVisible(false);

    mainLayout->addWidget(window->projectDetailContainer);

    QObject::connect(window->projectAddBtn, &QPushButton::clicked, window,
                     &MainWindow::addProject);
    QObject::connect(window->projectDetailCloseBtn, &QPushButton::clicked,
                     window, &MainWindow::closeProjectDetails);
    QObject::connect(window->projectDetailAutoAssignBtn, &QPushButton::clicked,
                     window, &MainWindow::autoAssignDetailedProject);

    tabWidget->addTab(window->projectTab, "Projects");
}

void MainWindowUIBuilder::setupStatisticsTab(MainWindow* window,
                                             QTabWidget* tabWidget) {
    if (!window || !tabWidget) return;

    window->statsTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(window->statsTab);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(25);

    QLabel* titleLabel = new QLabel("📊 Company Statistics");
    titleLabel->setStyleSheet(
        "font-size: 20px; font-weight: bold; padding: 10px; color: #000000;");
    layout->addWidget(titleLabel);

    window->statisticsChartWidget = new QWidget();
    window->statisticsChartWidget->setMinimumHeight(400);
    window->statisticsChartWidget->setStyleSheet(
        "background-color: white; border: 1px solid #e3f2fd; border-radius: "
        "10px;");
    layout->addWidget(window->statisticsChartWidget, 1);

    window->statisticsText = new QTextEdit();
    window->statisticsText->setReadOnly(true);
    window->statisticsText->setStyleSheet(
        "font-size: 15px; line-height: 2.0; padding: 25px; border-radius: "
        "10px;");
    layout->addWidget(window->statisticsText, 1);

    tabWidget->addTab(window->statsTab, "Statistics");

    QObject::connect(tabWidget, &QTabWidget::currentChanged, window,
                     [window, tabWidget](int index) {
                         if (window && tabWidget) {
                             QString tabText = tabWidget->tabText(index);
                             if (tabText == "Statistics") {
                                 window->showStatistics();
                             }
                         }
                     });
}
