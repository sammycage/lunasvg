target_sources(lunasvg 
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/plutovg.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-paint.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-geometry.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-blend.c"
    "${CMAKE_CURRENT_LIST_DIR}/plutovg-rle.c"
)

target_include_directories(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
