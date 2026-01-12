cmake_minimum_required(VERSION 3.17)

file(GLOB_RECURSE HEADER *.h *.hpp)
file(GLOB_RECURSE SRC *.cpp)
file(GLOB_RECURSE UI *.ui)
file(GLOB_RECURSE QRC *qrc)
set(PROJECT_SOURCE ${HEADER} ${SRC} ${UI} ${QRC})

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCE})

target_include_directories(${PROJECT_NAME} PRIVATE "E:/05_Data/Libs/OnlyHeader")
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    global_hotkey::global_hotkey
)

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
    # WIN32_EXECUTABLE ON
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/languages DESTINATION ${CMAKE_INSTALL_BINDIR})
