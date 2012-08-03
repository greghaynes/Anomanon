# Try to find librrd

find_path(LIBRRD_INCLUDE_DIR rrd.h
          HINTS /usr/include /usr/local/include)


find_library(LIBRRD_LIBRARY NAMES rrd librrd
             HINTS /usr/lib /usr/local/lib)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(LibRrd  DEFAULT_MSG
                                  LIBRRD_LIBRARY LIBRRD_INCLUDE_DIR)


mark_as_advanced(LIBRRD_INCLUDE_DIR LIBRRD_LIBRARY )
