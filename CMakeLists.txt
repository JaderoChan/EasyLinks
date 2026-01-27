cmake_minimum_required(VERSION 3.17)

project(EasyLink VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

list(APPEND CMAKE_PREFIX_PATH $ENV{MSVC_LIBS})

find_package(global_hotkey REQUIRED)
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include(FetchContent)
FetchContent_Declare(
    easy_translate
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/easy_translate
)
FetchContent_MakeAvailable(easy_translate)

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

set(APP_ORGANIZATION "JaderoChan")
set(APP_TITLE ${PROJECT_NAME})
set(APP_COPYRIGHT_TEXT "Copyright © 2025 JaderoChan")
set(APP_UUID "53c90f08-cf31-4998-83f0-2eeb408406f9")

add_subdirectory(src)
