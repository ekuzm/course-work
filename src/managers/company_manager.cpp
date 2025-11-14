#include "managers/company_manager.h"

#include <QCoreApplication>
#include <QDate>
#include <QFormLayout>
#include <QObject>
#include <QPushButton>

#include "utils/consts.h"

void CompanyManager::initializeCompany(std::vector<Company*>& companies,
                                       Company*& currentCompany,
                                       int& currentCompanyIndex,
                                       int& nextEmployeeId, int& nextProjectId,
                                       QComboBox* selector) {
    while (true) {
        QDialog dialog(selector);
        dialog.setWindowTitle("Setup Your IT Company");
        dialog.setMinimumWidth(kDefaultDialogMinWidth);
        dialog.setStyleSheet("QDialog { background-color: white; }");

        auto* form = new QFormLayout(&dialog);

        auto* nameEdit = new QLineEdit();
        nameEdit->setPlaceholderText("e.g., Yandex");
        nameEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Company Name:", nameEdit);

        auto* industryEdit = new QLineEdit();
        industryEdit->setPlaceholderText("e.g., Software Development");
        industryEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Industry:", industryEdit);

        auto* locationEdit = new QLineEdit();
        locationEdit->setPlaceholderText("e.g., Minsk, Belarus");
        locationEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Location:", locationEdit);

        auto* yearEdit = new QLineEdit();
        yearEdit->setPlaceholderText("e.g., 2020");
        yearEdit->setStyleSheet("QLineEdit { color: black; }");
        form->addRow("Founded Year:", yearEdit);

        auto* okButton = new QPushButton("Create Company");
        form->addRow(okButton);
        QObject::connect(okButton, &QPushButton::clicked, &dialog,
                         &QDialog::accept);

        if (int result = dialog.exec(); result != QDialog::Accepted) {
            QCoreApplication::quit();
            return;
        }

        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        if (companyName.isEmpty()) {
            QMessageBox::warning(&dialog, "Error",
                                 "Company name cannot be empty!");
            continue;
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "Industry cannot be empty!");
            continue;
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "Location cannot be empty!");
            continue;
        }
        if (!conversionSuccess || foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(&dialog, "Error",
                                 "Please enter a valid year!");
            continue;
        }

        currentCompany = new Company(companyName, companyIndustry,
                                     companyLocation, foundedYear);
        companies.push_back(currentCompany);
        currentCompanyIndex = companies.size() - 1;

        if (selector != nullptr) {
            selector->addItem(companyName);
            selector->setCurrentIndex(currentCompanyIndex);
        }

        return;
    }
}

void CompanyManager::addCompany(std::vector<Company*>& companies,
                                Company*& currentCompany,
                                int& currentCompanyIndex, QComboBox* selector,
                                QWidget* parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Add New Company");
    dialog.setMinimumWidth(400);
    dialog.setStyleSheet("QDialog { background-color: white; }");

    auto* form = new QFormLayout(&dialog);

    auto* nameEdit = new QLineEdit();
    nameEdit->setPlaceholderText("e.g., Google");
    nameEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Company Name:", nameEdit);

    auto* industryEdit = new QLineEdit();
    industryEdit->setPlaceholderText("e.g., Software Development");
    industryEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Industry:", industryEdit);

    auto* locationEdit = new QLineEdit();
    locationEdit->setPlaceholderText("e.g., Mountain View, USA");
    locationEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Location:", locationEdit);

    auto* yearEdit = new QLineEdit();
    yearEdit->setPlaceholderText("e.g., 1998");
    yearEdit->setStyleSheet("QLineEdit { color: black; }");
    form->addRow("Founded Year:", yearEdit);

    auto* okButton = new QPushButton("Create");
    form->addRow(okButton);

    QObject::connect(okButton, &QPushButton::clicked, [&]() {
        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        if (companyName.isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error",
                                 "Company name cannot be empty!\n\n"
                                 "Please enter a name for the company.");
            return;
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error",
                                 "Industry cannot be empty!\n\n"
                                 "Please enter the industry of the company.");
            return;
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(&dialog, "Validation Error",
                                 "Location cannot be empty!\n\n"
                                 "Please enter the location of the company.");
            return;
        }
        if (!conversionSuccess) {
            QMessageBox::warning(&dialog, "Validation Error",
                                 "Invalid year format!\n\n"
                                 "Please enter a valid number.\n"
                                 "Current value: \"" +
                                     yearEdit->text() + "\"");
            return;
        }
        if (foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(
                &dialog, "Validation Error",
                "Year out of valid range!\n\n"
                "Current value: " +
                    QString::number(foundedYear) +
                    "\n"
                    "Valid range: " +
                    QString::number(kMinYear) + " to " +
                    QString::number(QDate::currentDate().year()));
            return;
        }

        for (Company* existingCompany : companies) {
            if (existingCompany != nullptr &&
                existingCompany->getName().toLower() == companyName.toLower()) {
                QMessageBox::warning(
                    &dialog, "Duplicate Error",
                    "A company with this name already exists!\n\n"
                    "Company name: \"" +
                        companyName +
                        "\"\n"
                        "Please choose a different name.");
                return;
            }
        }

        currentCompany = new Company(companyName, companyIndustry,
                                     companyLocation, foundedYear);
        companies.push_back(currentCompany);
        currentCompanyIndex = companies.size() - 1;

        if (selector != nullptr) {
            selector->addItem(companyName);
            selector->setCurrentIndex(currentCompanyIndex);
        }

        QMessageBox::information(&dialog, "Success",
                                 "Company added successfully!\n\n"
                                 "Name: " +
                                     companyName +
                                     "\n"
                                     "Industry: " +
                                     companyIndustry +
                                     "\n"
                                     "Location: " +
                                     companyLocation +
                                     "\n"
                                     "Founded: " +
                                     QString::number(foundedYear));
        dialog.accept();
    });

    dialog.exec();
}

void CompanyManager::switchCompany(std::vector<Company*>& companies,
                                   Company*& currentCompany,
                                   int& currentCompanyIndex,
                                   QComboBox* selector, int newIndex) {
    if (selector != nullptr && currentCompanyIndex >= 0 && newIndex >= 0 &&
        static_cast<size_t>(newIndex) < companies.size()) {
        currentCompany = companies[newIndex];
        currentCompanyIndex = newIndex;
        if (selector->currentIndex() != newIndex) {
            selector->setCurrentIndex(newIndex);
        }
    }
}

void CompanyManager::deleteCompany(std::vector<Company*>& companies,
                                   Company*& currentCompany,
                                   int& currentCompanyIndex,
                                   QComboBox* selector, QWidget* parent) {
    if (companies.empty()) {
        QMessageBox::warning(parent, "Error", "No companies to delete!");
        return;
    }
    if (companies.size() <= 1) {
        QMessageBox::warning(parent, "Error",
                             "Cannot delete the last remaining company!");
        return;
    }

    int ret = QMessageBox::question(
        parent, "Confirm Delete",
        "Are you sure you want to delete the current company?",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        delete currentCompany;
        companies.erase(companies.begin() + currentCompanyIndex);

        if (!companies.empty()) {
            currentCompanyIndex = 0;
            currentCompany = companies[0];
            if (selector != nullptr) {
                selector->setCurrentIndex(0);
            }
        } else {
            currentCompany = nullptr;
            currentCompanyIndex = -1;
            if (selector != nullptr) {
                selector->clear();
            }
        }
    }
}

void CompanyManager::refreshCompanyList(const std::vector<Company*>& companies,
                                        QComboBox* selector) {
    if (selector != nullptr) {
        selector->clear();
        for (const auto* company : companies) {
            if (company != nullptr) {
                selector->addItem(company->getName());
            }
        }
    }
}
