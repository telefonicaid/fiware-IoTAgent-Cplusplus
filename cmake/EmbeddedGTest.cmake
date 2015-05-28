# Find the Google Test headers and libraries
# GTEST_INCLUDE_DIR where to find gtest.h
# GTEST_FOUND If false, do not try to use Google Test
set(GMOCK_FIND_OPTIONS ${IOT_FIND_OPTIONS})
if (GMOCK_ROOT)
set(GMOCK_FIND_OPTIONS NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
endif()

find_path ( GTEST_INCLUDE_DIR gtest/gtest.h
PATHS ${GMOCK_ROOT}/include $ENV{GMOCK_ROOT}/include ${_GMOCK_IOT}/include
${GMOCK_FIND_OPTIONS})
# handle the QUIETLY and REQUIRED arguments and set GTEST_FOUND to TRUE if
# all listed variables are TRUE
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( GTest DEFAULT_MSG GTEST_INCLUDE_DIR )
mark_as_advanced ( GTEST_INCLUDE_DIR )
