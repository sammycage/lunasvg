file(GLOB LUNASVG_EXAMPLE_FILES "*.cpp")

foreach(EXAMPLE_FILENAME ${LUNASVG_EXAMPLE_FILES})
    get_filename_component(EXAMPLE_TARGET ${EXAMPLE_FILENAME} NAME_WE)
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_FILENAME})
    target_link_libraries(${EXAMPLE_TARGET} lunasvg sfml-system sfml-window sfml-graphics)
    target_include_directories(${EXAMPLE_TARGET} PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/")
endforeach()
