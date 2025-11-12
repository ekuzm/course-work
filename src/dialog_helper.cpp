#include "dialog_helper.h"

#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

void DialogHelper::createHtmlDialog(QDialog* dialog, const QString& title,
                                    const QString& html, int minWidth,
                                    int minHeight) {
    if (!dialog) return;

    dialog->setWindowTitle(title);
    dialog->setMinimumSize(minWidth, minHeight);
    dialog->setStyleSheet("QDialog { background-color: white; }");

    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(15);

    auto* textEdit = new QTextEdit();
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet("font-size: 14px; border: none;");
    textEdit->setHtml(html);
    layout->addWidget(textEdit);

    auto* closeButton = new QPushButton("Close");
    closeButton->setStyleSheet(
        "QPushButton { background-color: #1976d2; color: white; padding: 10px "
        "30px; "
        "border-radius: 5px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1565c0; }");
    QObject::connect(closeButton, &QPushButton::clicked, dialog,
                     &QDialog::accept);

    auto* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);
}
