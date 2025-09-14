cmake_minimum_required(VERSION 3.15)

set(LUNASVG_VERSION_MAJOR 3)
set(LUNASVG_VERSION_MINOR 5)
set(LUNASVG_VERSION_MICRO 0)

project(lunasvg LANGUAGES CXX VERSION ${LUNASVG_VERSION_MAJOR}.${LUNASVG_VERSION_MINOR}.${LUNASVG_VERSION_MICRO})

option(USE_SYSTEM_PLUTOVG "Use system plutovg library" OFF)
if(USE_SYSTEM_PLUTOVG)
    find_package(plutovg 1.3.0 QUIET)
    if(NOT plutovg_FOUND)
        message(WARNING "Could not find: plutovg>=1.3.0. Falling back to plutovg submodule.")
    endif()
endif()

if(NOT plutovg_FOUND)
    message(STATUS "Using plutovg submodule.")
    set(PLUTOVG_BUILD_EXAMPLES OFF CACHE BOOL "Disable plutovg examples" FORCE)
    add_subdirectory(plutovg)
endif()

set(lunasvg_sources
    source/lunasvg.cpp
    source/graphics.cpp
    source/svgelement.cpp
    source/svggeometryelement.cpp
    source/svglayoutstate.cpp
    source/svgpaintelement.cpp
    source/svgparser.cpp
    source/svgproperty.cpp
    source/svgrenderstate.cpp
    source/svgtextelement.cpp
)

set(lunasvg_headers
    include/lunasvg.h
    source/graphics.h
    source/svgelement.h
    source/svggeometryelement.h
    source/svglayoutstate.h
    source/svgpaintelement.h
    source/svgparserutils.h
    source/svgproperty.h
    source/svgrenderstate.h
    source/svgtextelement.h
)

add_library(lunasvg ${lunasvg_sources} ${lunasvg_headers})
add_library(lunasvg::lunasvg ALIAS lunasvg)

target_link_libraries(lunasvg PRIVATE plutovg::plutovg)
set_target_properties(lunasvg PROPERTIES
    SOVERSION ${LUNASVG_VERSION_MAJOR}
    CXX_VISIBILITY_PRESET hidden
    CXX_STANDARD_REQUIRED ON
    CXX_STANDARD 17
)

target_include_directories(lunasvg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_include_directories(lunasvg PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/lunasvg>
)

target_compile_definitions(lunasvg PRIVATE LUNASVG_BUILD)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(lunasvg PUBLIC LUNASVG_BUILD_STATIC)
endif()

option(LUNASVG_DISABLE_LOAD_SYSTEM_FONTS "Disable automatic loading of fonts from system directories" OFF)
if(LUNASVG_DISABLE_LOAD_SYSTEM_FONTS)
    target_compile_definitions(lunasvg PRIVATE LUNASVG_DISABLE_LOAD_SYSTEM_FONTS)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/lunasvgConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg
)

write_basic_package_version_file(lunasvgConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/lunasvg.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lunasvg
)

install(TARGETS lunasvg
    EXPORT lunasvgTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT lunasvgTargets
    FILE lunasvgTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg
    NAMESPACE lunasvg::
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/lunasvgConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lunasvg
)

export(EXPORT lunasvgTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/lunasvgTargets.cmake
    NAMESPACE lunasvg::
)

file(RELATIVE_PATH lunasvg_pc_prefix_relative
    "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig"
    "${CMAKE_INSTALL_PREFIX}"
)

set(lunasvg_pc_cflags "")
if(NOT BUILD_SHARED_LIBS)
    string(APPEND lunasvg_pc_cflags " -DLUNASVG_BUILD_STATIC")
endif()

string(CONFIGURE [[
prefix=${pcfiledir}/@lunasvg_pc_prefix_relative@
includedir=${prefix}/@CMAKE_INSTALL_INCLUDEDIR@
libdir=${prefix}/@CMAKE_INSTALL_LIBDIR@

Name: LunaSVG
Description: SVG rendering and manipulation library
Version: @PROJECT_VERSION@

Requires.private: plutovg
Cflags: -I${includedir}/lunasvg@lunasvg_pc_cflags@
Libs: -L${libdir} -llunasvg
]] lunasvg_pc @ONLY)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/lunasvg.pc" "${lunasvg_pc}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/lunasvg.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

option(LUNASVG_BUILD_EXAMPLES "Build examples" ON)
if(LUNASVG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
