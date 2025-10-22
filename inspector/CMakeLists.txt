cmake_minimum_required(VERSION 3.16)

project(inspector LANGUAGES CXX VERSION 0.1.0 DESCRIPTION "XPBuddy Character Inspector")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Multimedia)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS TextToSpeech QUIET)

if (Qt${QT_VERSION_MAJOR}TextToSpeech_FOUND)
    message(STATUS "Qt${QT_VERSION_MAJOR}Speech found — enabling speech support")
    add_compile_definitions(HAVE_QTSPEECH)
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

set(PROJECT_SOURCES
    xpbapp.cpp
    propertieswindow.h
    propertieswindow.cpp
    renderer.h
    renderer.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Widgets)

    qt_add_executable(inspector
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        loggerwindow.h loggerwindow.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET inspector APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(inspector SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(inspector
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(inspector PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(inspector
    PRIVATE
    ${CMAKE_SOURCE_DIR}/libacsfile
)

target_link_libraries(inspector PRIVATE libacsfile)

if (Qt${QT_VERSION_MAJOR}TextToSpeech_FOUND)
    target_link_libraries(inspector PRIVATE Qt${QT_VERSION_MAJOR}::TextToSpeech)
target_link_libraries(inspector PRIVATE Qt6::Widgets)
endif()

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.catsworks.xpbinspector)
endif()
set_target_properties(inspector PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS inspector
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(inspector)
endif()

