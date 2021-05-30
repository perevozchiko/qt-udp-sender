cmake_minimum_required(VERSION 3.5)
project(SenderUdp LANGUAGES CXX)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
endif()

# Путь к  Qt
if(UNIX)
    set(CMAKE_PREFIX_PATH /opt/Qt/5.12.5/gcc_64/)
elseif(WIN32)
    set(CMAKE_PREFIX_PATH C:/Qt/5.12.5/mingw492_32/)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED Core Widgets Xml SerialPort WebSockets)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        Dialog.cpp
        Dialog.h
        Dialog.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SenderUdp
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(SenderUdp SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(SenderUdp
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::WebSockets
        Qt5::SerialPort
)
