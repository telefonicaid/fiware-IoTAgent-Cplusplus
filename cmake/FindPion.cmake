#-------------------------------------------------------------------------------------
# Locate pion library
# This module defines
# PION_FOUND,
# Pion_LIBRARIES
# Pion_INCLUDE_DIR
# Pion_LIBS_DIR
#
# Location hint can be provided through
# environment var PION_ROOT
#
#-------------------------------------------------------------------------------------

# get version macro
# first param - path to include
macro(pion_get_version _include_PATH vmajor vminor vpatch)
 file(STRINGS "${_include_PATH}/pion/config.hpp" _pion_VER_STRING_AUX REGEX ".*#define[ ]+PION_VERSION[ ]+")
 string(REGEX MATCHALL "[0-9]+" _pion_VERSION "${_pion_VER_STRING_AUX}")
 list(LENGTH _pion_VERSION _pion_VER_LIST_LEN)
 if(_pion_VER_LIST_LEN EQUAL 3)
  list(GET _pion_VERSION 0 ${vmajor})
  list(GET _pion_VERSION 1 ${vminor})
  list(GET _pion_VERSION 2 ${vpatch})
 endif()
endmacro()

find_path(Pion_INCLUDE_DIR
           NAMES
           pion/config.hpp
           PATHS
           ${_PION_IOT}/include
           $ENV{PION_ROOT}/include
           ${PION_ROOT}/include
					 ${IOT_FIND_OPTIONS}
)
if(Pion_USE_STATIC_LIBS STREQUAL "ON")
 set(_Pion_LIB_NAME "libpion.a")
else()
 set(_Pion_LIB_NAME "pion")
endif()

find_library(_Pion_LIBRARIES
               NAMES
               ${_Pion_LIB_NAME}
               PATHS
               ${_PION_IOT}/lib
               $ENV{PION_ROOT}/lib
               ${PION_ROOT}/lib
					     ${IOT_FIND_OPTIONS}
)

if(Pion_INCLUDE_DIR)
  pion_get_version(
  ${Pion_INCLUDE_DIR}
  Pion_VER_MAJOR
  Pion_VER_MINOR
  Pion_VER_PATCH)
endif()

if(_Pion_LIBRARIES)
 if (Pion_USE_STATIC_LIBS STREQUAL "ON")
  set(Pion_LIBRARIES ${_Pion_LIBRARIES})
 else()
  set(Pion_LIBRARIES ${_Pion_LIB_NAME})
 endif()
 get_filename_component(Pion_LIBS_DIR ${_Pion_LIBRARIES} DIRECTORY)
endif()
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Pion DEFAULT_MSG Pion_LIBRARIES Pion_INCLUDE_DIR Pion_LIBS_DIR)
MARK_AS_ADVANCED(Pion_INCLUDE_DIR Pion_LIBRARIES Pion_LIBS_DIR
Pion_VER_MAJOR Pion_VER_MINOR Pion_VER_PATCH)
if(PION_FOUND)
 message("-- Pion: ${Pion_INCLUDE_DIR} ${Pion_LIBRARIES} ${Pion_VER_MAJOR}.${Pion_VER_MINOR}.${Pion_VER_PATCH}")
endif()
