target_sources(lunasvg 
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_vcgen_stroke.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_vcgen_dash.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_vcgen_contour.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_trans_affine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_curves.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/src/agg_sqrt_tables.cpp"
)

target_include_directories(lunasvg
PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

