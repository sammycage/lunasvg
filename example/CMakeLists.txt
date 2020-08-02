add_executable(svg2png "svg2png.cpp")

target_link_libraries(svg2png lunasvg)

target_include_directories(svg2png
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../include/"
)

