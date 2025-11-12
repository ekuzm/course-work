#include "file_helper.h"

#include <QDir>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>
#include <exception>

#include "main_window.h"

void FileHelper::clearAllDataFiles(QWidget* parent) {
    if (!parent) return;

    int confirmation = QMessageBox::question(
        parent, "Confirm Clear Data",
        "Are you sure you want to clear all data files?\n\n"
        "This action cannot be undone!",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (confirmation != QMessageBox::Yes) {
        return;
    }

    try {
        MainWindow* mainWindow = qobject_cast<MainWindow*>(parent);
        if (!mainWindow) return;

        QString dataDirPath = mainWindow->getDataDirectory();
        QDir dataDir(dataDirPath);

        if (dataDir.exists()) {
            QStringList dirs = {"companies", "employees", "projects"};
            QStringList filters{"*.txt"};

            for (const QString& dirName : dirs) {
                QDir subDir(dataDirPath + "/" + dirName);
                if (subDir.exists()) {
                    subDir.setNameFilters(filters);
                    subDir.setFilter(QDir::Files);
                    for (const QString& fileName : subDir.entryList()) {
                        subDir.remove(fileName);
                    }
                }
            }

            QMessageBox::information(
                parent, "Success",
                "All data files have been cleared successfully.");
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(
            parent, "Error",
            QString("Failed to clear data files: %1").arg(e.what()));
    }
}
