cmake_minimum_required(VERSION 3.3)

project(lunasvg VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_SHARED_LIBS "Builds shared libraries" ON)
option(LUNASVG_BUILD_EXAMPLES "Builds examples" OFF)
option(LUNASVG_ENABLE_CAIRO "Enable Cairo 2D Graphics"  OFF)

add_library(lunasvg)

add_subdirectory(include)
add_subdirectory(source)

if(LUNASVG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(LUNASVG_ENABLE_CAIRO)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CAIRO REQUIRED cairo)
    target_include_directories(lunasvg PRIVATE ${CAIRO_INCLUDE_DIRS})
    target_link_libraries(lunasvg ${CAIRO_LIBRARIES})
    add_subdirectory(source/backends/cairo)
else()
    add_subdirectory(3rdparty/agg)
    add_subdirectory(source/backends/agg)
endif()

set(LUNASVG_LIBDIR ${CMAKE_INSTALL_PREFIX}/lib)
set(LUNASVG_INCDIR ${CMAKE_INSTALL_PREFIX}/include/lunasvg)

install(FILES
    include/svgdocument.h
    include/svgelement.h
    include/svgelementiter.h
    DESTINATION ${LUNASVG_INCDIR}
)

install(TARGETS lunasvg
    LIBRARY     DESTINATION    ${LUNASVG_LIBDIR}
    ARCHIVE     DESTINATION    ${LUNASVG_LIBDIR}
    INCLUDES    DESTINATION    ${LUNASVG_INCDIR}
)
