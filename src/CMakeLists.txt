cmake_minimum_required(VERSION 3.17)

file(GLOB_RECURSE HEADER *.h *.hpp)
file(GLOB_RECURSE SRC app/*.cpp filelink/*.cpp)
if(WIN32)
    list(APPEND SRC shell/shell_utility_win.cpp)
elseif(APPLE)
else()
endif()
file(GLOB_RECURSE UI *.ui)
file(GLOB_RECURSE QRC *qrc)
set(PROJECT_SOURCE ${HEADER} ${SRC} ${UI} ${QRC})

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCE})

configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/template/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/config.h
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    global_hotkey::global_hotkey
    easy_translate::easy_translate
)

set(APP_ICON ${CMAKE_CURRENT_SOURCE_DIR}/app/icons/app.ico)
if(WIN32)
    set(RC_FILE ${CMAKE_CURRENT_BINARY_DIR}/app.rc)
    set(RC_CONTENT "APP_ICON ICON \"${APP_ICON}\"")
    file(WRITE ${RC_FILE} ${RC_CONTENT})
    target_sources(${PROJECT_NAME} PRIVATE ${RC_FILE})
endif()

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    WIN32_EXECUTABLE ON
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /Zc:__cplusplus /permissive-)
endif()

option(UPDATE_TRANSLATIONS_FILES OFF)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<BOOL:UPDATE_TRANSLATIONS_FILES>:EASY_TRANSLATE_UPDATE_TRANSLATIONS_FILES>
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/languages ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/languages
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/languages DESTINATION ${CMAKE_INSTALL_BINDIR})
