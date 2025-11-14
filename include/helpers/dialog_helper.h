#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>

class DialogHelper {
   public:
    static void createHtmlDialog(QDialog* dialog, const QString& title,
                                 const QString& html, int minWidth = 800,
                                 int minHeight = 600);
    static void createTableDialog(QDialog* dialog, const QString& title,
                                  QTableWidget* table, int minWidth = 900,
                                  int minHeight = 600);
};
