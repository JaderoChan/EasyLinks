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
    file(
        GLOB PLATFORMS_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/*_mac.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/platforms/*_mac.mm
    )
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
target_include_directories(
    ${PROJECT_NAME} PRIVATE
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
    find_library(APP_KIT_LIBRARY AppKit)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(SERVICE_MANAGEMENT_LIBRARY ServiceManagement)
    target_link_libraries(
        ${PROJECT_NAME} PRIVATE
        ${APPLICATION_SERVICES_LIBRARY}
        ${APP_KIT_LIBRARY}
        ${COCOA_LIBRARY}
        ${SERVICE_MANAGEMENT_LIBRARY}
    )
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

# 配置应用资源/属性
if(WIN32)
    # Windows
    set(APP_ICON ${APP_ICONS_DIR}/app.ico)

    set(RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
    configure_file(
        ${CMAKE_TEMPLATE_DIR}/app.rc.in
        ${RC_FILE}
        @ONLY
    )
    target_sources(${PROJECT_NAME} PRIVATE ${RC_FILE})
elseif(APPLE)
    # Apple
    set(APP_ICON ${APP_ICONS_DIR}/app.icns)

    # Info Plist Strings
    set(
        INFO_PLIST_STRINGS_NAMES
        en.lproj
        zh_CN.lproj
    )
    foreach(INFO_PLIST_STRINGS_NAME IN LISTS INFO_PLIST_STRINGS_NAMES)
        configure_file(
            ${CMAKE_TEMPLATE_DIR}/InfoPlistStrings/${INFO_PLIST_STRINGS_NAME}/InfoPlist.strings.in
            ${CMAKE_CURRENT_BINARY_DIR}/${INFO_PLIST_STRINGS_NAME}/InfoPlist.strings
            @ONLY
        )
    endforeach()
    foreach(INFO_PLIST_STRINGS_NAME IN LISTS INFO_PLIST_STRINGS_NAMES)
        set(INFO_PLIST_STRINGS_FILE ${CMAKE_CURRENT_BINARY_DIR}/${INFO_PLIST_STRINGS_NAME}/InfoPlist.strings)
        set_source_files_properties(
            ${INFO_PLIST_STRINGS_FILE} PROPERTIES
            MACOSX_PACKAGE_LOCATION "Resources/${INFO_PLIST_STRINGS_NAME}"
        )
        target_sources(${PROJECT_NAME} PRIVATE ${INFO_PLIST_STRINGS_FILE})
    endforeach()

    # Info Plist
    set(MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_BINARY_DIR}/Info.plist)
    configure_file(
        ${CMAKE_TEMPLATE_DIR}/Info.plist.in
        ${MACOSX_BUNDLE_INFO_PLIST}
        @ONLY
    )

    # 应用图标
    set_source_files_properties(
        ${APP_ICON} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${APP_ICON})

    set_target_properties(
        ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_INFO_PLIST ${MACOSX_BUNDLE_INFO_PLIST}
    )

    # 语言文件
    file(GLOB LANG_FILES ${LANGUAGES_DIR}/*)
    set_source_files_properties(
        ${LANG_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/languages"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${LANG_FILES})

    # 脚本文件
    file(GLOB SCRIPT_FILES ${RESOURCES_DIR}/scripts/mac/*)
    set_source_files_properties(
        ${SCRIPT_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/scripts"
    )
    target_sources(${PROJECT_NAME} PRIVATE ${SCRIPT_FILES})
else()
    # TODO
endif()

#################
# 生成后额外命令 #
#################

# 复制语言文件至输出目录
if(NOT APPLE)
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${LANGUAGES_DIR} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages
    )
endif()

# Qt部署（需要保证*deployqt可见）
if(WIN32)
    set(DEPLOYQT windeployqt)
    set(OUTPUT_SUFFIX "exe")
elseif(APPLE)
    set(DEPLOYQT macdeployqt)
    set(OUTPUT_SUFFIX "app")
else()
    # TODO
endif()
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${DEPLOYQT}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.${OUTPUT_SUFFIX}
    $<$<PLATFORM_ID:Darwin>:-codesign=->
)

# MacOS DMG打包
option(CREATE_DMG "Create DMG for MacOS, create-dmg is required." OFF)
if(APPLE AND CREATE_DMG)
    # Qt部署与DMG打包
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/create_dmg.sh
        ${PROJECT_NAME}
        ${APP_ICON}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )
endif()
