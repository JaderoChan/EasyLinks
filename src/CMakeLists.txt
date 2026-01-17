cmake_minimum_required(VERSION 3.17)

file(GLOB_RECURSE HEADER *.h *.hpp)
file(GLOB_RECURSE SRC *.cpp)
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

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    # WIN32_EXECUTABLE ON
)

option(UPDATE_TRANSLATIONS_FILES ON)

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
