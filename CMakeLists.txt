cmake_minimum_required(VERSION 3.3)

project(lunasvg CXX)

set(CMAKE_CXX_STANDARD 11)

option(BUILD_SHARED_LIBS "Builds shared libraries" ON)
option(LUNASVG_BUILD_EXAMPLES "Builds examples" ON)
option(LUNASVG_ENABLE_CAIRO "Enable Cairo 2D Graphics"  OFF)

if(LUNASVG_ENABLE_CAIRO)
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
    find_package(Cairo REQUIRED)
endif()

add_library(lunasvg)

add_subdirectory(include)
add_subdirectory(source)

if(LUNASVG_BUILD_EXAMPLES)
    add_subdirectory(example)
endif()

if(LUNASVG_ENABLE_CAIRO)
    target_include_directories(lunasvg PRIVATE ${CAIRO_INCLUDE_DIR})
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
