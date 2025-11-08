#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

class DialogHelper {
   public:
    static void createHtmlDialog(QDialog* dialog, const QString& title, const QString& html,
                                 int minWidth = 800, int minHeight = 600);
};





