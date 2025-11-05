#include "exceptions.h"

#include <utility>

BaseException::BaseException(QString msg) : message(std::move(msg)) {}

const char* BaseException::what() const noexcept {
    return message.toLocal8Bit().constData();
}

EmployeeException::EmployeeException(QString msg)
    : BaseException(std::move(msg)) {}

CompanyException::CompanyException(QString msg)
    : BaseException(std::move(msg)) {}

ProjectException::ProjectException(QString msg)
    : BaseException(std::move(msg)) {}

TaskException::TaskException(QString msg) : BaseException(std::move(msg)) {}

FileManagerException::FileManagerException(QString msg)
    : BaseException(std::move(msg)) {}
