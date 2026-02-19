cmake_minimum_required(VERSION 3.17)

project(EasyLinks VERSION 1.3.10 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 默认使用Release构建
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# 查找Qt库
find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# 配置Easy Translate库
include(FetchContent)
set(3RDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
FetchContent_Declare(
    easy_translate
    SOURCE_DIR ${3RDPARTY_DIR}/easy_translate
)
FetchContent_MakeAvailable(easy_translate)

# 配置Global Hotkey库
FetchContent_Declare(
    global_hotkey
    SOURCE_DIR ${3RDPARTY_DIR}/global_hotkey
)
set(GLOBAL_HOTKEY_DISABLE_REGISTER ON)
FetchContent_MakeAvailable(global_hotkey)

# 重定向目标文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

# 设置程序信息
set(APP_ORGANIZATION "JaderoChan")
set(APP_ORGANIZATION_DOMAIN "io.github.jaderochan")
set(APP_TITLE ${PROJECT_NAME})
set(APP_VERSION ${PROJECT_VERSION})
set(APP_AUTHOR "頔珞 JaderoChan")
set(APP_AUTHOR_EMAIL "c_dl_cn@outlook.com")
set(APP_COPYRIGHT_TEXT "Copyright (c) 2025 JaderoChan")
set(APP_UUID "53c90f08-cf31-4998-83f0-2eeb408406f9")

set(DEFAULT_RENAME_PATTERN "@ (#)")
set(MAX_LOG_COUNT 1000)

###############
# 定义常用路径 #
###############

# CMake文件夹
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
# CMake模板文件夹
set(CMAKE_TEMPLATE_DIR ${CMAKE_DIR}/template)
# 语言文件夹路径
set(LANGUAGES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/languages)
# 资源文件夹路径
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
# 图标文件夹路径
set(ICONS_DIR ${RESOURCES_DIR}/icons)
# 脚本文件夹路径
set(SCRIPTS_DIR ${RESOURCES_DIR}/scripts)

add_subdirectory(src)
