#pragma once

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>

struct EmployeeTabUI {
    QWidget* tab = nullptr;
    QTableWidget* table = nullptr;
    QPushButton* addBtn = nullptr;
    QLineEdit* searchEdit = nullptr;
};

struct ProjectTabUI {
    QWidget* tab = nullptr;
    QWidget* listContainer = nullptr;
    QWidget* detailHeaderContainer = nullptr;
    QWidget* detailContainer = nullptr;
    QTableWidget* table = nullptr;
    QTableWidget* tasksTable = nullptr;
    QPushButton* addBtn = nullptr;
    QPushButton* detailCloseBtn = nullptr;
    QPushButton* detailAutoAssignBtn = nullptr;
    QLabel* detailTitle = nullptr;
    QTextEdit* detailInfoText = nullptr;
};

struct StatisticsTabUI {
    QWidget* tab = nullptr;
    QWidget* chartWidget = nullptr;
    QWidget* chartInnerWidget = nullptr;
    QTextEdit* text = nullptr;
};

struct CompanyUI {
    QWidget* widget = nullptr;
    QComboBox* selector = nullptr;
    QPushButton* addBtn = nullptr;
    QPushButton* deleteBtn = nullptr;
};
