file(GLOB LUNASVG_EXAMPLE_FILES "*.cpp")

foreach(EXAMPLE_FILENAME ${LUNASVG_EXAMPLE_FILES})
    get_filename_component(EXAMPLE_TARGET ${EXAMPLE_FILENAME} NAME_WE)
    add_executable(${EXAMPLE_TARGET} ${EXAMPLE_FILENAME})
    target_link_libraries(${EXAMPLE_TARGET} lunasvg)
    target_include_directories(${EXAMPLE_TARGET}    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../include/"
                                                    PRIVATE "${CMAKE_CURRENT_LIST_DIR}/../example_nodeps/")
    target_compile_definitions(${EXAMPLE_TARGET}    PRIVATE _CRT_SECURE_NO_WARNINGS)
                                                    
endforeach()
