cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 14)

project(svg2png CXX)

add_executable(example svg2png.cpp)
target_link_libraries(example lunasvg)

foreach(EXAMPLE_FILENAME ${LUNASVG_EXAMPLE_FILES})
    get_filename_component(EXAMPLE_TARGET ${EXAMPLE_FILENAME} NAME_WE)
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_FILENAME})
    target_link_libraries(${EXAMPLE_TARGET} lunasvg sfml-system sfml-window sfml-graphics)
    target_include_directories(${EXAMPLE_TARGET} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/")
endforeach()

if(LUNASVG_BUILD_EXTRAS)
    add_subdirectory(SDL2) 
    add_subdirectory(WxWidgets)
    add_subdirectory(X11)
endif()
