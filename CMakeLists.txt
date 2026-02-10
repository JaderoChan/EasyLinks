cmake_minimum_required(VERSION 3.17)

project(EasyLinks VERSION 1.1.7.4 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include(FetchContent)
set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
FetchContent_Declare(
    easy_translate
    SOURCE_DIR ${3RDPARTY_DIR}/easy_translate
)
FetchContent_MakeAvailable(easy_translate)

FetchContent_Declare(
    global_hotkey
    SOURCE_DIR ${3RDPARTY_DIR}/global_hotkey
)
set(GLOBAL_HOTKEY_DISABLE_REGISTER ON)
FetchContent_MakeAvailable(global_hotkey)

include(GNUInstallDirs)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

set(APP_ORGANIZATION "JaderoChan")
set(APP_TITLE ${PROJECT_NAME})
set(APP_AUTHOR "頔珞 JaderoChan")
set(APP_AUTHOR_EMAIL "c_dl_cn@outlook.com")
set(APP_COPYRIGHT_TEXT "Copyright (c) 2025 JaderoChan")
set(APP_UUID "53c90f08-cf31-4998-83f0-2eeb408406f9")

add_subdirectory(src)
