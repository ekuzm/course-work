#include "ui_style_helper.h"

QString UIStyleHelper::getMainWindowStylesheet() {
    return R"(
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
    )";
}

QString UIStyleHelper::getCompanyComboBoxStylesheet() {
    return R"(
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
    )";
}

