set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(svg2png svg2png.cpp)
target_link_libraries(svg2png lunasvg)
