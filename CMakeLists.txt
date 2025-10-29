cmake_minimum_required(VERSION 3.16)
project(CompanyManagement)

set(CMAKE_CXX_STANDARD 20)
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
    src/file_manager.cpp
    src/employee_dialog_helper.cpp
)

set(HEADERS
    include/main_window.h
    include/company.h
    include/employee.h
    include/derived_employees.h
    include/project.h
    include/file_manager.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} 
    Qt6::Core
    Qt6::Widgets
)

