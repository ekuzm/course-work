#pragma once

#include <QString>
#include <exception>

class BaseException : public std::exception {
   protected:
    QString message;

   public:
    explicit BaseException(QString msg);
    const char* what() const noexcept override;
};

class EmployeeException : public BaseException {
   public:
    explicit EmployeeException(QString msg);
};

class CompanyException : public BaseException {
   public:
    explicit CompanyException(QString msg);
};

class ProjectException : public BaseException {
   public:
    explicit ProjectException(QString msg);
};

class TaskException : public BaseException {
   public:
    explicit TaskException(QString msg);
};

class FileManagerException : public BaseException {
   public:
    explicit FileManagerException(QString msg);
};
