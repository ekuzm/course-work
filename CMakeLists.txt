cmake_minimum_required(VERSION 3.28)
project(course-work)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(ENTITY_SOURCES
    src/entities/company.cpp
    src/entities/company_managers.cpp
    src/entities/employee.cpp
    src/entities/derived_employees.cpp
    src/entities/project.cpp
    src/entities/task.cpp
)

set(ENTITY_HEADERS
    include/entities/company.h
    include/entities/company_managers.h
    include/entities/employee.h
    include/entities/derived_employees.h
    include/entities/project.h
    include/entities/task.h
)

set(SERVICE_SOURCES
    src/services/cost_calculation_service.cpp
    src/services/task_assignment_service.cpp
    src/services/employee_service.cpp
    src/services/project_service.cpp
)

set(SERVICE_HEADERS
    include/services/cost_calculation_service.h
    include/services/task_assignment_service.h
    include/services/employee_service.h
    include/services/project_service.h
)

set(UI_SOURCES
    src/ui/main_window.cpp
    src/ui/main_window_operations.cpp
    src/ui/main_window_ui_builder.cpp
    src/ui/main_window_helpers.cpp
    src/ui/statistics_chart_widget.cpp
)

set(UI_HEADERS
    include/ui/main_window.h
    include/ui/main_window_operations.h
    include/ui/main_window_ui_builder.h
    include/ui/main_window_helpers.h
    include/ui/statistics_chart_widget.h
)

set(HELPER_SOURCES
    src/helpers/action_button_helper.cpp
    src/helpers/dialog_helper.cpp
    src/helpers/display_helper.cpp
    src/helpers/employee_dialog_helper.cpp
    src/helpers/employee_dialog_handler.cpp
    src/helpers/file_helper.cpp
    src/helpers/html_generator.cpp
    src/helpers/id_helper.cpp
    src/helpers/project_dialog_helper.cpp
    src/helpers/project_helper.cpp
    src/helpers/task_assignment_helper.cpp
    src/helpers/task_dialog_helper.cpp
    src/helpers/validation_helper.cpp
    src/helpers/employee_validator.cpp
)

set(HELPER_HEADERS
    include/helpers/action_button_helper.h
    include/helpers/dialog_helper.h
    include/helpers/display_helper.h
    include/helpers/employee_dialog_helper.h
    include/helpers/employee_dialog_handler.h
    include/helpers/file_helper.h
    include/helpers/html_generator.h
    include/helpers/id_helper.h
    include/helpers/project_dialog_helper.h
    include/helpers/project_helper.h
    include/helpers/task_assignment_helper.h
    include/helpers/task_dialog_helper.h
    include/helpers/validation_helper.h
    include/helpers/employee_validator.h
)

set(MANAGER_SOURCES
    src/managers/company_manager.cpp
    src/managers/file_manager.cpp
    src/managers/auto_save_loader.cpp
)

set(MANAGER_HEADERS
    include/managers/company_manager.h
    include/managers/file_manager.h
    include/managers/auto_save_loader.h
)

set(EXCEPTION_SOURCES
    src/exceptions/exceptions.cpp
    src/exceptions/exception_handler.cpp
)

set(EXCEPTION_HEADERS
    include/exceptions/exceptions.h
    include/exceptions/exception_handler.h
)

set(UTIL_SOURCES
    src/utils/app_styles.cpp
)

set(UTIL_HEADERS
    include/utils/app_styles.h
    include/utils/consts.h
    include/utils/container_utils.h
)

set(SOURCES
    src/main.cpp
    ${ENTITY_SOURCES}
    ${SERVICE_SOURCES}
    ${UI_SOURCES}
    ${HELPER_SOURCES}
    ${MANAGER_SOURCES}
    ${EXCEPTION_SOURCES}
    ${UTIL_SOURCES}
)

set(HEADERS
    ${ENTITY_HEADERS}
    ${SERVICE_HEADERS}
    ${UI_HEADERS}
    ${HELPER_HEADERS}
    ${MANAGER_HEADERS}
    ${EXCEPTION_HEADERS}
    ${UTIL_HEADERS}
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} 
    Qt6::Core
    Qt6::Widgets
)
