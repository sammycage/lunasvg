cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 11)

project(lunasvg)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(Cairo REQUIRED)

project(lunasvg)

add_library(lunasvg)

add_subdirectory(include)
add_subdirectory(source)
add_subdirectory(example)

target_include_directories(lunasvg PRIVATE ${CAIRO_INCLUDE_DIR})
target_link_libraries(lunasvg ${CAIRO_LIBRARIES})

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
