cmake_minimum_required(VERSION 3.13)

project(lunasvg VERSION 2.3.5 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# Default to hidden visibility for symbols
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

option(BUILD_SHARED_LIBS "Builds as shared library" OFF)
option(LUNASVG_BUILD_EXAMPLES "Builds examples" OFF)

add_library(lunasvg)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(3rdparty/plutovg)

set_target_properties(lunasvg
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_library(lunasvg::lunasvg ALIAS lunasvg)

if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(lunasvg PUBLIC LUNASVG_STATIC_DEFINE)
endif()

if(LUNASVG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

#
# Installation
#
include(GNUInstallDirs)
install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lunasvg.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/lunasvg_export.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lunasvg
)
install(
    TARGETS lunasvg
    EXPORT lunasvg-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(
    EXPORT lunasvg-targets
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg NAMESPACE lunasvg:: FILE lunasvgTargets.cmake
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/lunasvgConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfig.cmake
    INSTALL_DESTINATION
    ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg
)

write_basic_package_version_file(lunasvgConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg
)

export(EXPORT lunasvg-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/lunasvgTargets.cmake NAMESPACE lunasvg::)
