#include "exceptions/exception_handler.h"

#include <QMessageBox>

#include "exceptions/exceptions.h"

void ExceptionHandler::handleCompanyException(const CompanyException& e,
                                              QDialog* dialog,
                                              const QString& action) {
    QWidget* parent = dialog ? dialog->parentWidget() : nullptr;
    if (!parent && dialog) {
        parent = dialog;
    }
    QMessageBox::warning(parent, "Error",
                         QString("Failed to %1!\n\nError details: %2\n\nPlease "
                                 "check the input data and try again.")
                             .arg(action, e.what()));
}

void ExceptionHandler::handleFileManagerException(const FileManagerException& e,
                                                  QDialog* dialog,
                                                  const QString& action) {
    QWidget* parent = dialog ? dialog->parentWidget() : nullptr;
    if (!parent && dialog) {
        parent = dialog;
    }
    QMessageBox::warning(
        parent, "Auto-save Error",
        QString("Failed to auto-save changes!\n\nError details: %1\n\n"
                "The %2 was completed but the data could not be saved "
                "automatically.\n"
                "Please check file permissions and try again.")
            .arg(e.what(), action));
}

void ExceptionHandler::handleGenericException(const std::exception& e,
                                              QDialog* dialog) {
    QWidget* parent = dialog ? dialog->parentWidget() : nullptr;
    if (!parent && dialog) {
        parent = dialog;
    }
    QMessageBox::critical(
        parent, "Unexpected Error",
        QString("An unexpected error occurred!\n\nError details: %1\n\n"
                "Please try again or contact support if the problem persists.")
            .arg(e.what()));
}
