cmake_minimum_required(VERSION 3.5)

project(SenderUdp LANGUAGES CXX)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
endif()

# Path to Qt
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

set(PROJECT_SOURCES
        main.cpp
        Dialog.cpp
        Dialog.h
        Dialog.ui
)

add_executable(SenderUdp ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
        Qt5::WebSockets
        Qt5::SerialPort
)
