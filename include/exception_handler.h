#pragma once

#include <QDialog>
#include <QString>
#include <exception>

class CompanyException;
class FileManagerException;

class ExceptionHandler {
   public:
    static void handleCompanyException(const CompanyException& e, QDialog* dialog, const QString& action);
    static void handleFileManagerException(const FileManagerException& e, QDialog* dialog, const QString& action);
    static void handleGenericException(const std::exception& e, QDialog* dialog);
};





