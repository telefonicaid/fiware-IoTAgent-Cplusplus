#-------------------------------------------------------------------------------------
# Locate Log4cplus library
# This module defines
# LOG4CPP_FOUND, if false, do not try to link to Log4cplus
# LOG4CPP_LIBRARY
# LOG4CPP_INCLUDE_DIR, where to find log4cplus.hppa
# LOG4CPP_LIBS_DIR,
#
#
# environment var LOG4CPP_ROOT
#
#-------------------------------------------------------------------------------------
set(LOG4CPP_FIND_OPTIONS ${IOT_FIND_OPTIONS})
if (LOG4CPP_ROOT)
set(LOG4CPP_FIND_OPTIONS NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
endif()


find_path(LOG4CPP_INCLUDE_DIR
           NAMES
           log4cpp/config.h
           PATHS
           ${_LOG4CPP_IOT}/include
           $ENV{LOG4CPP_ROOT}/include
           ${LOG4CPP_ROOT}/include
           ${LOG4CPP_FIND_OPTIONS} 
)
if(Log4Cpp_USE_STATIC_LIBS)
 set(_log4cpp_LIB_NAME "liblog4cpp.a")
else()
 set(_log4cpp_LIB_NAME "log4cpp")
endif()

find_library(_LOG4CPP_LIBRARIES
              NAMES
              ${_log4cpp_LIB_NAME}
              PATHS
              ${_LOG4CPP_IOT}/lib
              $ENV{LOG4CPP_ROOT}/lib
              ${LOG4CPP_ROOT}/lib
              ${LOG4CPP_FIND_OPTIONS} 
)

if(LOG4CPP_INCLUDE_DIR)
 get_filename_component(LOG4CPP_LIBS_DIR ${_LOG4CPP_LIBRARIES} DIRECTORY)
endif()


if(Log4Cplus_USE_STATIC_LIBS)
 set(LOG4CPP_LIBRARIES ${_LOG4CPP_LIBRARIES})
else()
 set(LOG4CPP_LIBRARIES ${_log4cpp_LIB_NAME})
endif()

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set LOG4CPP_FOUND to TRUE if
# all listed variables are TRUE
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Log4cpp DEFAULT_MSG LOG4CPP_LIBRARIES LOG4CPP_INCLUDE_DIR LOG4CPP_LIBS_DIR)
MARK_AS_ADVANCED(LOG4CPP_INCLUDE_DIR LOG4CPP_LIBRARIES LOG4CPP_LIBS_DIR)
if(LOG4CPP_FOUND)
message("-- Log4cpp: ${LOG4CPP_INCLUDE_DIR} ${LOG4CPP_LIBRARIES}")
endif()
