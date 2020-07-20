cmake_minimum_required(VERSION 3.3)

project(lunasvg VERSION 0.1.0 LANGUAGES C CXX)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(LUNASVG_BUILD_EXAMPLES "Build examples" ON)

add_library(lunasvg)

set_target_properties(lunasvg PROPERTIES VERSION 0.1.0 SONAME 0)

target_link_libraries(lunasvg cairo)

target_compile_options(lunasvg
PRIVATE
    -std=c++11 -fPIC -fno-rtti -fno-exceptions -Wall
)

add_subdirectory(include)
add_subdirectory(source)

if(LUNASVG_BUILD_EXAMPLES)
add_subdirectory(example)
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
