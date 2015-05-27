#-------------------------------------------------------------------------------------
# Locate Log4cplus library
# This module defines
# LOG4CPLUS_FOUND, if false, do not try to link to Log4cplus
# LOG4CPLUS_LIBRARY
# LOG4CPLUS_INCLUDE_DIR, where to find log4cplus.hppa
# LOG4CPLUS_LIBS_DIR,
#
#
# environment var LOG4CPLUS_ROOT
#
#-------------------------------------------------------------------------------------
# get version macro
# first param - path to include
macro(log4cplus_get_version _include_PATH vmajor vminor vpatch)
 file(STRINGS "${_include_PATH}/log4cplus/version.h" _log4cplus_VER_STRING_AUX REGEX ".*#define[ ]+LOG4CPLUS_VERSION[ ]+")
 string(REGEX MATCHALL "[0-9]+" _log4clpus_VER_LIST "${_log4cplus_VER_STRING_AUX}")
 list(LENGTH _log4clpus_VER_LIST _log4cplus_VER_LIST_LEN)

 if(_log4cplus_VER_LIST_LEN EQUAL 5)
  list(GET _log4clpus_VER_LIST 2 ${vmajor})
  list(GET _log4clpus_VER_LIST 3 ${vminor})
  list(GET _log4clpus_VER_LIST 4 ${vpatch})
 endif()
endmacro()

find_path(LOG4CPLUS_INCLUDE_DIR
           NAMES
           log4cplus/logger.h log4cplus/version.h
           PATHS
           ${IOTAGENT_DEPENDENCIES_PATHS}
           $ENV{LOG4CPLUS_ROOT}/include
           ${LOG4CPLUS_ROOT}/include
           NO_DEFAULT_PATH
)

if(Log4Cplus_USE_STATIC_LIBS)
 set(_log4cplus_LIB_NAME "liblog4cplus.a")
else()
 set(_log4cplus_LIB_NAME "log4cplus")
endif()

find_library(_LOG4CPLUS_LIBRARIES
              NAMES
              ${_log4cplus_LIB_NAME}
              PATHS
              ${IOTAGENT_DEPENDENCIES_PATHS}
              $ENV{LOG4CPLUS_ROOT}/lib
              ${LOG4CPLUS_ROOT}/lib
              NO_DEFAULT_PATH
)

if(LOG4CPLUS_INCLUDE_DIR)
 log4cplus_get_version(${LOG4CPLUS_INCLUDE_DIR}
                       LOG4CPLUS_VER_MAJOR
                       LOG4CPLUS_VER_MINOR
                       LOG4CPLUS_VER_PATCH)
 set(_log4cplus_VERSION "${LOG4CPLUS_VER_MAJOR}.${LOG4CPLUS_VER_MINOR}.${LOG4CPLUS_VER_PATCH}")
 get_filename_component(LOG4CPLUS_LIBS_DIR ${_LOG4CPLUS_LIBRARIES} DIRECTORY)
endif()


if(Log4Cplus_USE_STATIC_LIBS)
 set(LOG4CPLUS_LIBRARIES ${_LOG4CPLUS_LIBRARIES})
else()
 set(LOG4CPLUS_LIBRARIES ${_log4cplus_LIB_NAME})
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LOG4CPLUS_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cplus DEFAULT_MSG LOG4CPLUS_LIBRARIES LOG4CPLUS_INCLUDE_DIR LOG4CPLUS_LIBS_DIR)
MARK_AS_ADVANCED(LOG4CPLUS_INCLUDE_DIR LOG4CPLUS_LIBRARIES LOG4CPLUS_LIBS_DIR
                   LOG4CPLUS_VER_MAJOR LOG4CPLUS_VER_MINOR LOG4CPLUS_VER_PATCH)
if(LOG4CPLUS_FOUND)
message("-- Log4cplus: ${LOG4CPLUS_INCLUDE_DIR} ${LOG4CPLUS_LIBRARIES} ${LOG4CPLUS_VER_MAJOR}.${LOG4CPLUS_VER_MINOR}.${LOG4CPLUS_VER_PATCH}")
endif()
