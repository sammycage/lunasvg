target_include_directories(lunasvg
PUBLIC
    # When building a project that uses the lunasvg library,
    # we need to look in the installed include directory
    $<INSTALL_INTERFACE:include>

    # When building the lunasvg library we need to look in the
    # build dir for the lunasvg_export.h header and in the source
    # dir for other headers
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BUILD_DIR}>
)
