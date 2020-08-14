target_sources(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/canvas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/strokedata.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/rgb.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/pattern.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gradient.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/paint.cpp"
)

target_include_directories(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
