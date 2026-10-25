cmake_minimum_required(VERSION 3.16)
project(DMM_Diag LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets SerialPort Charts)

add_executable(DMM_Diag
    main.cpp
    dmm_maindiag.cpp
    dmm_maindiag.h
    dmm_maindiag.ui
    M2550/m2550_access.cpp
    M2550/m2550_access.h
    Serial/serial_access.cpp
    Serial/serial_access.h
)

target_include_directories(DMM_Diag PRIVATE M2550)
target_link_libraries(DMM_Diag PRIVATE Qt6::Widgets Qt6::SerialPort Qt6::Charts)

install(TARGETS DMM_Diag RUNTIME DESTINATION bin)
