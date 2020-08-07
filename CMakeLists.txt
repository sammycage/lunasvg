cmake_minimum_required(VERSION 3.3)

set(LUNASVG_MAJOR_VERSION 0)
set(LUNASVG_MINOR_VERSION 1)
set(LUNASVG_PATCH_VERSION 0)
set(LUNASVG_VERSION ${LUNASVG_MAJOR_VERSION}.${LUNASVG_MINOR_VERSION}.${LUNASVG_PATCH_VERSION})

project(lunasvg VERSION ${LUNASVG_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_SHARED_LIBS "Builds shared libraries" ON)
option(LUNASVG_BUILD_EXAMPLES "Builds examples" ON)
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

