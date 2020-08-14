target_sources(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/canvasimpl.cpp"
)

target_include_directories(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}"
)
