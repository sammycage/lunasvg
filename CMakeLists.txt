cmake_minimum_required(VERSION 3.15)

set(LUNASVG_VERSION_MAJOR 3)
set(LUNASVG_VERSION_MINOR 2)
set(LUNASVG_VERSION_MICRO 1)

project(lunasvg LANGUAGES CXX VERSION ${LUNASVG_VERSION_MAJOR}.${LUNASVG_VERSION_MINOR}.${LUNASVG_VERSION_MICRO})

find_package(plutovg 0.0.4 QUIET)
if(NOT plutovg_FOUND)
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

target_link_libraries(lunasvg PRIVATE plutovg::plutovg)
target_compile_definitions(lunasvg PRIVATE LUNASVG_BUILD)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(lunasvg PUBLIC LUNASVG_BUILD_STATIC)
endif()

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

include(GNUInstallDirs)
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

option(LUNASVG_BUILD_EXAMPLES "Build examples" ON)
if(LUNASVG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
