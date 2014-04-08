project(DMM_Diag)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 REQUIRED)
#find_package(Qt5 REQUIRED)


#set(DMM_Diag_SRCS main.cpp dmm_maindiag.cpp rigol_access.cpp)
#set(DMM_Diag_HEADERS dmm_maindiag.h rigol_access.h)

set(DMM_Diag_SRCS Serial/serial_access.cpp M2550/m2550_access.cpp main.cpp dmm_maindiag.cpp)
set(DMM_Diag_HEADERS dmm_maindiag.h M2550/m2550_access.h Serial/serial_access.h)


SET(DMM_Diag_FORMS dmm_maindiag.ui)

INCLUDE(${QT_USE_FILE})
ADD_DEFINITIONS(${QT_DEFINITIONS})

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} M2550/ /usr/include/qwt)


QT4_WRAP_CPP(DMM_Diag_HEADERS_MOC ${DMM_Diag_HEADERS})
QT4_ADD_RESOURCES(DMM_Diag_RESOURCES_RCC ${DMM_Diag_RESOURCES})
QT4_WRAP_UI(DMM_Diag_FORMS_HEADERS ${DMM_Diag_FORMS})

qt4_automoc(${DMM_Diag_SRCS})

add_executable(DMM_Diag ${DMM_Diag_SRCS}
    ${DMM_Diag_HEADERS_MOC} 
    ${DMM_Diag_FORMS_HEADERS}
    ${DMM_Diag_RESOURCES_RCC}
)
    
target_link_libraries(DMM_Diag ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_LIBRARIES} qwt qextserialport)

install(TARGETS DMM_Diag RUNTIME DESTINATION bin)
