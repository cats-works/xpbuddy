cmake_minimum_required(VERSION 3.16)

project(libacsfile LANGUAGES CXX VERSION 0.1.0 DESCRIPTION "Agent Character File Routines")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set_target_properties(libacsfile PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(libacsfile PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
# set_target_properties(libacsfile PROPERTIES PUBLIC_HEADER acsfile.h)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

add_library(libacsfile
    acs_private.h acs_private.cpp

    acsfile.h acsfile.cpp
    README.md
    acs_wintypes.h)

target_include_directories(libacsfile PRIVATE
    ${CMAKE_SOURCE_DIR}/libacsfile
)

include(GNUInstallDirs)
install(TARGETS libacsfile
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
