# - Find libmosquitto
# Find the native libmosquitto includes and libraries
#
# MOSQUITTO_LIBRARIES - List of libraries when using libmosquitto.
# MOSQUITTO_FOUND - True if libmosquitto found.
# var environment MOSQUITTO_ROOT
set(MOSQUITTO_FIND_OPTIONS ${IOT_FIND_OPTIONS})
if (MOSQUITTO_ROOT)
set(MOSQUITTO_FIND_OPTIONS NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
endif()

find_library(MOSQUITTO_LIBRARY
              PATHS
              ${_MOSQUITTO_IOT}/lib
              $ENV{MOSQUITTO_ROOT}/lib
              ${MOSQUITTO_ROOT}/lib
              NAMES mosquittopp mosquitto ${MOSQUITTO_FIND_OPTIONS})
include(FindPackageHandleStandardArgs)

if(MOSQUITTO_LIBRARY)
 get_filename_component(MOSQUITTO_LIBS_DIR ${MOSQUITTO_LIBRARY} DIRECTORY)
 set(MOSQUITTO_FOUND "YES")
 set(MOSQUITTO_LIBRARIES ${MOSQUITTO_LIBRARY})
endif(MOSQUITTO_LIBRARY)
find_package_handle_standard_args(MOSQUITTO DEFAULT_MSG MOSQUITTO_LIBS_DIR MOSQUITTO_LIBRARIES)
mark_as_advanced(MOSQUITTO_LIBRARIES MOSQUITTO_LIBS_DIR)

