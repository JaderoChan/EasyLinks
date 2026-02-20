cmake_minimum_required(VERSION 3.17)

#############
# 收集源文件 #
#############

# 配置config头文件
configure_file(
    ${CMAKE_TEMPLATE_DIR}/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
    @ONLY
)

# 头文件（可用于在Visual Studio IDE的项目文件树中显示头文件）
file(GLOB_RECURSE HEADER ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
# 源文件
file(
    GLOB_RECURSE SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/app/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/filelink/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils/*.cpp
)
# 平台相关的代码
if(WIN32)
    file(GLOB PLATFORMS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platforms/*_win.cpp)
elseif(APPLE)
    file(GLOB PLATFORMS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platforms/*_mac.cpp)
else()
    file(GLOB PLATFORMS_SRC ${CMAKE_CURRENT_SOURCE_DIR}/platforms/*_linux.cpp)
endif()
list(APPEND SRC ${PLATFORMS_SRC})
# UI文件
file(GLOB_RECURSE UI ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)
# QRC文件
set(QRC ${RESOURCES_DIR}/app.qrc)
set(PROJECT_SOURCE ${HEADER} ${SRC} ${UI} ${QRC})

###########
# 构建程序 #
###########

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCE})

###########
# 配置依赖 #
###########

# 配置头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# 链接依赖库
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    global_hotkey::global_hotkey
    easy_translate::easy_translate
)

if(APPLE)
    find_library(APPLICATION_SERVICES_LIBRARY ApplicationServices)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${APPLICATION_SERVICES_LIBRARY})
endif()

###############
# 配置生成目标 #
###############

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 指定为GUI应用程序
if(WIN32)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus /permissive-)
endif()

# 程序是否自动更新语言文件（开发时使用）
option(UPDATE_TRANSLATIONS_FILES OFF)
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<BOOL:UPDATE_TRANSLATIONS_FILES>:EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES>
)

# 设置应用文件图标
set(APP_ICON ${ICONS_DIR}/app.ico)
if(WIN32)
    set(RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
    configure_file(
        ${CMAKE_TEMPLATE_DIR}/app.rc.in
        ${RC_FILE}
        @ONLY
    )
    target_sources(${PROJECT_NAME} PRIVATE ${RC_FILE})
elseif(APPLE)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "app.ico"
    )
    set_source_files_properties(
        ${APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON})
else()
    # TODO
endif()

# 配置MacOS下Bundle属性
if(APPLE)
    set(IS_MACOSX_BUNDLE TRUE)
    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ${IS_MACOSX_BUNDLE}
        MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_ORGANIZATION_DOMAIN}.${APP_TITLE}
        MACOSX_BUNDLE_BUNDLE_NAME ${APP_TITLE}
        MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_TITLE}
        MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION}
        MACOSX_BUNDLE_COPYRIGHT ${APP_COPYRIGHT_TEXT}
    )
endif()

# 配置MacOS下脚本文件
if(APPLE)
    file(GLOB SCRIPT_FILES ${SCRIPTS_DIR}/mac/*)
    set_source_files_properties(
        ${SCRIPT_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/scripts"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${SCRIPT_FILES})
endif()

# 配置MacOS下语言文件
if(APPLE AND IS_MACOSX_BUNDLE)
    file(GLOB LANG_FILES ${LANGUAGES_DIR}/*)
    set_source_files_properties(
        ${LANG_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/languages"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${LANG_FILES})

    target_compile_definitions(${PROJECT_NAME} PRIVATE IS_MACOSX_BUNDLE)
endif()

#################
# 生成后额外命令 #
#################

# 复制语言文件至输出目录
if(NOT APPLE OR NOT IS_MACOSX_BUNDLE)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${LANGUAGES_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages
    )
endif()
