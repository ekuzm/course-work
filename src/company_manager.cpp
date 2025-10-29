#include "../include/company_manager.h"

#include <QCoreApplication>
#include <QDate>
#include <QFormLayout>
#include <QObject>
#include <QPushButton>

#include "../include/consts.h"

void CompanyManager::initializeCompany(CompanyManager::CompanyData& companyData,
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

        int result = dialog.exec();

        // If user closed the dialog without creating company
        if (result != QDialog::Accepted) {
            QCoreApplication::quit();
            return;
        }

        // If dialog was accepted, validate input
        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        // Validation
        if (companyName.isEmpty()) {
            QMessageBox::warning(&dialog, "Error",
                                 "Company name cannot be empty!");
            continue;  // Restart dialog
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "Industry cannot be empty!");
            continue;  // Restart dialog
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "Location cannot be empty!");
            continue;  // Restart dialog
        }
        if (!conversionSuccess || foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(&dialog, "Error",
                                 "Please enter a valid year!");
            continue;  // Restart dialog
        }

        // All validation passed, create company
        companyData.currentCompany =
            new Company(companyName, companyIndustry, companyLocation, foundedYear);
        companyData.companies.push_back(companyData.currentCompany);
        companyData.currentCompanyIndex = companyData.companies.size() - 1;

        // Update company selector
        if (selector != nullptr) {
            selector->addItem(companyName);
            selector->setCurrentIndex(companyData.currentCompanyIndex);
        }

        // Successfully created company, exit the loop
        return;
    }
}

void CompanyManager::addCompany(CompanyManager::CompanyData& companyData,
                                 QComboBox* selector, QWidget* parent) {
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
    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted) {
        QString companyName = nameEdit->text().trimmed();
        QString companyIndustry = industryEdit->text().trimmed();
        QString companyLocation = locationEdit->text().trimmed();
        bool conversionSuccess = false;
        int foundedYear = yearEdit->text().trimmed().toInt(&conversionSuccess);

        // Validation
        if (companyName.isEmpty()) {
            QMessageBox::warning(parent, "Error",
                                 "Company name cannot be empty!");
            return;
        }
        if (companyIndustry.isEmpty()) {
            QMessageBox::warning(parent, "Error", "Industry cannot be empty!");
            return;
        }
        if (companyLocation.isEmpty()) {
            QMessageBox::warning(parent, "Error", "Location cannot be empty!");
            return;
        }
        if (!conversionSuccess || foundedYear < kMinYear ||
            foundedYear > QDate::currentDate().year()) {
            QMessageBox::warning(parent, "Error", "Please enter a valid year!");
            return;
        }

        // Create new company
        companyData.currentCompany =
            new Company(companyName, companyIndustry, companyLocation, foundedYear);
        companyData.companies.push_back(companyData.currentCompany);
        companyData.currentCompanyIndex = companyData.companies.size() - 1;

        // Update selector
        if (selector != nullptr) {
            selector->addItem(companyName);
            selector->setCurrentIndex(companyData.currentCompanyIndex);
        }

        QMessageBox::information(parent, "Success",
                                 "Company added successfully!");
    }
}

void CompanyManager::switchCompany(CompanyManager::CompanyData& companyData,
                                    QComboBox* selector, int newIndex) {
    if (selector != nullptr && companyData.currentCompanyIndex >= 0) {
        if (newIndex >= 0 && newIndex < (int)companyData.companies.size()) {
            companyData.currentCompany = companyData.companies[newIndex];
            companyData.currentCompanyIndex = newIndex;
            if (selector->currentIndex() != newIndex) {
                selector->setCurrentIndex(newIndex);
            }
        }
    }
}

void CompanyManager::deleteCompany(CompanyManager::CompanyData& companyData,
                                    QComboBox* selector, QWidget* parent) {
    if (companyData.companies.empty()) {
        QMessageBox::warning(parent, "Error", "No companies to delete!");
        return;
    }
    if (companyData.companies.size() <= 1) {
        QMessageBox::warning(parent, "Error",
                             "Cannot delete the last remaining company!");
        return;
    }

    int ret = QMessageBox::question(
        parent, "Confirm Delete",
        "Are you sure you want to delete the current company?",
        QMessageBox::Yes | QMessageBox::No);

    if (ret == QMessageBox::Yes) {
        delete companyData.currentCompany;
        companyData.companies.erase(companyData.companies.begin() +
                                    companyData.currentCompanyIndex);

        // Switch to first company if available
        if (!companyData.companies.empty()) {
            companyData.currentCompanyIndex = 0;
            companyData.currentCompany = companyData.companies[0];
            if (selector != nullptr) {
                selector->setCurrentIndex(0);
            }
        } else {
            companyData.currentCompany = nullptr;
            companyData.currentCompanyIndex = -1;
            if (selector != nullptr) {
                selector->clear();
            }
        }
    }
}

void CompanyManager::refreshCompanyList(CompanyManager::CompanyData& companyData,
                                         QComboBox* selector) {
    if (selector != nullptr) {
        selector->clear();
        for (auto* company : companyData.companies) {
            if (company != nullptr) {
                selector->addItem(company->getName());
            }
        }
        if (companyData.currentCompanyIndex >= 0 &&
            companyData.currentCompanyIndex < (int)companyData.companies.size()) {
            selector->setCurrentIndex(companyData.currentCompanyIndex);
        }
    }
}

