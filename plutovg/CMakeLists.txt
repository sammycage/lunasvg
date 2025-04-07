cmake_minimum_required(VERSION 3.15)

set(PLUTOVG_VERSION_MAJOR 1)
set(PLUTOVG_VERSION_MINOR 0)
set(PLUTOVG_VERSION_MICRO 0)

project(plutovg LANGUAGES C VERSION ${PLUTOVG_VERSION_MAJOR}.${PLUTOVG_VERSION_MINOR}.${PLUTOVG_VERSION_MICRO})

set(plutovg_sources
    source/plutovg-blend.c
    source/plutovg-canvas.c
    source/plutovg-font.c
    source/plutovg-matrix.c
    source/plutovg-paint.c
    source/plutovg-path.c
    source/plutovg-rasterize.c
    source/plutovg-surface.c
    source/plutovg-ft-math.c
    source/plutovg-ft-raster.c
    source/plutovg-ft-stroker.c
)

set(plutovg_headers
    include/plutovg.h
    source/plutovg-private.h
    source/plutovg-utils.h
    source/plutovg-ft-math.h
    source/plutovg-ft-raster.h
    source/plutovg-ft-stroker.h
    source/plutovg-ft-types.h
    source/plutovg-stb-image-write.h
    source/plutovg-stb-image.h
    source/plutovg-stb-truetype.h
)

add_library(plutovg ${plutovg_sources} ${plutovg_headers})
add_library(plutovg::plutovg ALIAS plutovg)

set_target_properties(plutovg PROPERTIES
    SOVERSION ${PLUTOVG_VERSION_MAJOR}
    C_VISIBILITY_PRESET hidden
    C_STANDARD_REQUIRED ON
    C_STANDARD 99
)

target_include_directories(plutovg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_include_directories(plutovg PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/plutovg>
)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(plutovg PRIVATE ${MATH_LIBRARY})
endif()

target_compile_definitions(plutovg PRIVATE PLUTOVG_BUILD)
if(NOT BUILD_SHARED_LIBS)
    target_compile_definitions(plutovg PUBLIC PLUTOVG_BUILD_STATIC)
endif()

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/plutovgConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/plutovgConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plutovg
)

write_basic_package_version_file(plutovgConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

include(GNUInstallDirs)
install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/plutovg.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/plutovg
)

install(TARGETS plutovg
    EXPORT plutovgTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT plutovgTargets
    FILE plutovgTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plutovg
    NAMESPACE plutovg::
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plutovgConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/plutovgConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/plutovg
)

export(EXPORT plutovgTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/plutovgTargets.cmake
    NAMESPACE plutovg::
)

option(PLUTOVG_BUILD_EXAMPLES "Build examples" ON)
if(PLUTOVG_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
