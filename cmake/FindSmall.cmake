# - Find libsmall header-only library
# The module defines the following variables:
#
#  SMALL_FOUND - true if small was found
#  SMALL_INCLUDE_DIRS - the directory of the small headers
#  SMALL_LIBRARIES - the small static library needed for linking
#

find_path(SMALL_INCLUDE_DIR small/small.h)
find_library(SMALL_LIBRARY NAMES small)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Small
    REQUIRED_VARS SMALL_INCLUDE_DIR SMALL_LIBRARY)
set(SMALL_INCLUDE_DIRS ${SMALL_INCLUDE_DIR} ${SMALL_INCLUDE_DIR}/small/third_party)
set(SMALL_LIBRARIES ${SMALL_LIBRARY})
mark_as_advanced(SMALL_INCLUDE_DIR SMALL_INCLUDE_DIRS
                 SMALL_LIBRARY SMALL_LIBRARIES)
