cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_STANDARD 14)

project(svg2png CXX)

add_executable(svg2png svg2png.cpp)
target_link_libraries(svg2png lunasvg)
