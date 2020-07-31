find_path(CAIRO_INCLUDE_DIR cairo.h PATH_SUFFIXES cairo)

set(CAIRO_NAMES ${CAIRO_NAMES} cairo libcairo cairo2 libcairo2)
find_library(CAIRO_LIBRARY NAMES ${CAIRO_NAMES})
set(CAIRO_LIBRARIES ${CAIRO_LIBRARY})

mark_as_advanced(CAIRO_INCLUDE_DIR CAIRO_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CAIRO DEFAULT_MSG CAIRO_LIBRARY CAIRO_INCLUDE_DIR)
