#include "managers/company_manager.h"

#include <QComboBox>
#include <QDate>
#include <QDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <span>
#include <vector>

#include "entities/company.h"
#include "utils/consts.h"

struct CreateCompanyParams {
    QDialog& dialog;
    std::vector<Company*>& companies;
    Company*& currentCompany;
    int& currentCompanyIndex;
    QLineEdit* nameEdit;
    QLineEdit* industryEdit;
    QLineEdit* locationEdit;
    QLineEdit* yearEdit;
    QComboBox* selector;
};

static void handleCreateCompanyButtonClick(const CreateCompanyParams& params) {
    QString companyName = params.nameEdit->text().trimmed();
    QString companyIndustry = params.industryEdit->text().trimmed();
    QString companyLocation = params.locationEdit->text().trimmed();
    bool conversionSuccess = false;
    int foundedYear =
        params.yearEdit->text().trimmed().toInt(&conversionSuccess);

    QWidget* parent = &params.dialog;

    if (companyName.isEmpty()) {
        QMessageBox::warning(parent, "Validation Error",
                             "Company name cannot be empty!\n\n"
                             "Please enter a name for the company.");
        return;
    }
    if (companyIndustry.isEmpty()) {
        QMessageBox::warning(parent, "Validation Error",
                             "Industry cannot be empty!\n\n"
                             "Please enter the industry of the company.");
        return;
    }
    if (companyLocation.isEmpty()) {
        QMessageBox::warning(parent, "Validation Error",
                             "Location cannot be empty!\n\n"
                             "Please enter the location of the company.");
        return;
    }
    if (!conversionSuccess) {
        QMessageBox::warning(parent, "Validation Error",
                             "Invalid year format!\n\n"
                             "Please enter a valid number.\n"
                             "Current value: \"" +
                                 params.yearEdit->text() + "\"");
        return;
    }
    if (foundedYear < kMinYear || foundedYear > QDate::currentDate().year()) {
        QMessageBox::warning(parent, "Validation Error",
                             "Year out of valid range!\n\n"
                             "Current value: " +
                                 QString::number(foundedYear) +
                                 "\n"
                                 "Valid range: " +
                                 QString::number(kMinYear) + " to " +
                                 QString::number(QDate::currentDate().year()));
        return;
    }

    for (const Company* existingCompany : params.companies) {
        if (existingCompany != nullptr &&
            existingCompany->getName().toLower() == companyName.toLower()) {
            QMessageBox::warning(parent, "Duplicate Error",
                                 "A company with this name already exists!\n\n"
                                 "Company name: \"" +
                                     companyName +
                                     "\"\n"
                                     "Please choose a different name.");
            return;
        }
    }

    params.currentCompany =
        new Company(companyName, companyIndustry, companyLocation, foundedYear);
    params.companies.push_back(params.currentCompany);
    params.currentCompanyIndex = params.companies.size() - 1;

    if (params.selector != nullptr) {
        params.selector->addItem(companyName);
        params.selector->setCurrentIndex(params.currentCompanyIndex);
    }

    QMessageBox::information(&params.dialog, "Success",
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
    params.dialog.accept();
}

void CompanyManager::initializeCompany(
    std::span<Company* const> companies,
    [[maybe_unused]] Company*& currentCompany, const int& currentCompanyIndex,
    [[maybe_unused]] int&, [[maybe_unused]] int&, QComboBox* selector) {
    if (!companies.empty() && selector != nullptr) {
        selector->clear();
        for (const auto* company : companies) {
            if (company != nullptr) {
                selector->addItem(company->getName());
            }
        }
        if (currentCompanyIndex >= 0 &&
            static_cast<size_t>(currentCompanyIndex) < companies.size()) {
            selector->setCurrentIndex(currentCompanyIndex);
        }
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

    QObject::connect(
        okButton, &QPushButton::clicked,
        [&dialog, &companies, &currentCompany, &currentCompanyIndex, nameEdit,
         industryEdit, locationEdit, yearEdit, selector]() {
            CreateCompanyParams params{
                dialog,   companies,    currentCompany, currentCompanyIndex,
                nameEdit, industryEdit, locationEdit,   yearEdit,
                selector};
            handleCreateCompanyButtonClick(params);
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

void CompanyManager::refreshCompanyList(std::span<Company* const> companies,
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
