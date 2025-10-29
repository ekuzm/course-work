cmake_minimum_required(VERSION 3.16)
project(CompanyManagement)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Enable automatic MOC, UIC, and RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/Company.cpp
    src/Employee.cpp
    src/DerivedEmployees.cpp
    src/Project.cpp
    src/FileManager.cpp
)

# Header files
set(HEADERS
    include/MainWindow.h
    include/Company.h
    include/Employee.h
    include/DerivedEmployees.h
    include/Project.h
    include/FileManager.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt6 libraries
target_link_libraries(${PROJECT_NAME} 
    Qt6::Core
    Qt6::Widgets
)

