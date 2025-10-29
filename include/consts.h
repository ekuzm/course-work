#ifndef CONSTS_H
#define CONSTS_H

// Salary validation constants
inline const int kMinSalary = 100;
inline const int kMaxSalary = 500000;

// Year validation constants
inline const int kMinYear = 1800;

// UI size constants
inline const int kDefaultDialogMinWidth = 400;
inline const int kCompanySelectorMinWidth = 200;
inline const int kSearchEditMinHeight = 35;
inline const int kDateEditMinWidth = 200;
inline const int kDescEditMaxHeight = 100;

// Table column widths
inline const int kTableColumnWidthId = 50;
inline const int kTableColumnWidthName = 150;
inline const int kTableColumnWidthPosition = 120;
inline const int kTableColumnWidthDepartment = 150;
inline const int kTableColumnWidthSalary = 100;
inline const int kTableColumnWidthType = 120;
inline const int kTableColumnWidthProjectName = 200;
inline const int kTableColumnWidthStatus = 120;
inline const int kTableColumnWidthBudget = 120;
inline const int kTableColumnWidthClient = 200;

// Table row height
inline const int kTableRowHeight = 50;

// Layout spacing and margins
inline const int kLayoutSpacing = 15;
inline const int kLayoutMargins = 15;
inline const int kCompanyLayoutMargins = 10;
inline const int kCompanyLayoutVerticalMargins = 5;

// Project defaults
inline const int kDefaultProjectEndDateDays = 90;

// Employee bonus constants
inline const double kManagerSalaryMultiplier = 0.25;
inline const int kManagerTeamBonus = 1000;
inline const double kDeveloperSalaryMultiplier = 0.20;
inline const int kDeveloperExperienceBonus = 500;
inline const double kDesignerSalaryMultiplier = 0.15;
inline const int kDesignerProjectBonus = 800;
inline const double kQaSalaryMultiplier = 0.10;
inline const int kQaBugBonus = 100;

#endif  // CONSTS_H

