cmake_minimum_required(VERSION 3.28)
project(course-work)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCES
    src/main.cpp
    src/main_window.cpp
    src/company.cpp
    src/employee.cpp
    src/derived_employees.cpp
    src/project.cpp
    src/task.cpp
    src/file_manager.cpp
    src/employee_dialog_helper.cpp
    src/company_manager.cpp
    src/display_helper.cpp
    src/exceptions.cpp
    src/validation_helper.cpp
    src/exception_handler.cpp
    src/employee_validator.cpp
    src/project_dialog_helper.cpp
    src/html_generator.cpp
    src/statistics_chart_widget.cpp
    src/task_assignment_helper.cpp
    src/project_helper.cpp
    src/dialog_helper.cpp
    src/app_styles.cpp
    src/auto_save_loader.cpp
    src/employee_dialog_handler.cpp
    src/main_window_ui_builder.cpp
    src/action_button_helper.cpp
    src/task_dialog_helper.cpp
    src/file_helper.cpp
)

set(HEADERS
    include/main_window.h
    include/company.h
    include/company_manager.h
    include/consts.h
    include/container_utils.h
    include/display_helper.h
    include/employee.h
    include/employee_dialog_helper.h
    include/employee_validator.h
    include/exception_handler.h
    include/project_dialog_helper.h
    include/validation_helper.h
    include/html_generator.h
    include/statistics_chart_widget.h
    include/task_assignment_helper.h
    include/project_helper.h
    include/dialog_helper.h
    include/app_styles.h
    include/auto_save_loader.h
    include/employee_dialog_handler.h
    include/main_window_ui_builder.h
    include/action_button_helper.h
    include/task_dialog_helper.h
    include/file_helper.h
    include/derived_employees.h
    include/project.h
    include/task.h
    include/file_manager.h
    include/exceptions.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} 
    Qt6::Core
    Qt6::Widgets
)

