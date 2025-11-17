#include "helpers/file_helper.h"

#include <QDir>
#include <QMessageBox>
#include <QStringList>
#include <QWidget>
#include <exception>

#include "ui/main_window.h"

void FileHelper::clearAllDataFiles(QWidget* parent) {
    if (!parent) return;

    if (int confirmation = QMessageBox::question(
            parent, "Confirm Clear Data",
            "Are you sure you want to clear all data files?\n\n"
            "This action cannot be undone!",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        confirmation != QMessageBox::Yes) {
        return;
    }

    try {
        if (const MainWindow* mainWindow = qobject_cast<MainWindow*>(parent);
            !mainWindow) {
            return;
        }

        QString dataDirPath = MainWindow::getDataDirectory();
        QDir dataDir(dataDirPath);

        if (dataDir.exists()) {
            QStringList dirs = {"companies", "employees", "projects"};
            QStringList filters{"*.txt"};

            for (const QString& dirName : dirs) {
                QDir subDir(dataDirPath + "/" + dirName);
                if (!subDir.exists()) {
                    continue;
                }
                subDir.setNameFilters(filters);
                subDir.setFilter(QDir::Files);
                for (const QString& fileName : subDir.entryList()) {
                    subDir.remove(fileName);
                }
            }

            QMessageBox::information(
                parent, "Success",
                "All data files have been cleared successfully.");
        }
    } catch (const FileManagerException& e) {
        QMessageBox::warning(
            parent, "Error",
            QString("Failed to clear data files: %1").arg(e.what()));
    }
}
