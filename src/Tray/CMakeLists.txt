cmake_minimum_required(VERSION 3.16)

project(sast-evento-tray VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6Widgets CONFIG REQUIRED)
find_package(Qt6Network CONFIG REQUIRED)

set(PROJECT_SOURCES
    main.cc
    TcpServer.h
    TcpServer.cc
    res.qrc
)

qt_add_executable(sast-evento-tray
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(sast-evento-tray PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)

set_target_properties(sast-evento-tray PROPERTIES
    MACOSX_BUNDLE FALSE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS sast-evento-tray
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(sast-evento-tray)
endif()
