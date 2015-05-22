#
# Find the MongoDB includes and client library
# This module defines
# MongoDB_INCLUDE_DIR, where to find mongo/client/dbclient.h
# MongoDB_LIBRARIES, the libraries needed to use MongoDB.
# MongoDB_FOUND, If false, do not try to use MongoDB.
# MongoDB_LIBS_DIR
#
# var environment MongoDB_ROOT

find_path(MongoDB_INCLUDE_DIR mongo/client/dbclient.h
               ${IOTAGENT_DEPENDENCIES_PATHS}
               $ENV{MongoDB_ROOT}/include
               ${MongoDB_ROOT}/include
               NO_DEFAULT_PATH)

if (MONGODB_USE_STATIC_LIBS STREQUAL "ON")
set(_mongodb_LIB_NAME "libmongoclient.a")
else()
set(_mongodb_LIB_NAME "mongoclient")
endif()
find_library(_MongoDB_LIBRARIES NAMES ${_mongodb_LIB_NAME}
               PATHS
               ${IOTAGENT_DEPENDENCIES_PATHS}
               $ENV{MongoDB_ROOT}/lib
               ${MongoDB_ROOT}/lib
               NO_DEFAULT_PATH
)
if(MongoDB_INCLUDE_DIR AND _MongoDB_LIBRARIES)
 get_filename_component(MongoDB_LIBS_DIR ${_MongoDB_LIBRARIES} DIRECTORY)
 if (NOT MONGODB_USE_STATIC_LIBS)
  set(MongoDB_LIBRARIES "${_mongodb_LIB_NAME}")
 else()
  set(MongoDB_LIBRARIES "${_MongoDB_LIBRARIES}")
 endif()
 set(MongoDB_FOUND TRUE)
else(MongoDB_INCLUDE_DIR AND _MongoDB_LIBRARIES)
 set(MongoDB_FOUND FALSE)
 if (MongoDB_FIND_REQUIRED)
  message(FATAL_ERROR "MongoDB not found.")
 else (MongoDB_FIND_REQUIRED)
message(STATUS "MongoDB not found.")
endif (MongoDB_FIND_REQUIRED)
endif(MongoDB_INCLUDE_DIR AND _MongoDB_LIBRARIES)
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MongoDB DEFAULT_MSG MongoDB_LIBRARIES MongoDB_INCLUDE_DIR MongoDB_LIBS_DIR)
mark_as_advanced(MongoDB_INCLUDE_DIR MongoDB_LIBRARIES MongoDB_LIBS_DIR)
