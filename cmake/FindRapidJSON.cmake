# - Find RapidJSON
# Find the RapidJSON includes.
# Once done, this will define
#
#   RapidJSON_INCLUDE_DIRS  - where to find the RapidJSON headers.
#   RapidJSON_FOUND         - True if RapidJSON is found.
#

FIND_PATH (RapidJSON_INCLUDE_DIR rapidjson PATHS ${_JSON_IOT}/include ${IOT_FIND_OPTIONS})

INCLUDE (FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(RapidJSON REQUIRED_VARS RapidJSON_INCLUDE_DIR)

IF (RAPIDJSON_FOUND)
    SET (RapidJSON_INCLUDE_DIRS ${RapidJSON_INCLUDE_DIR})
ENDIF (RAPIDJSON_FOUND)
mark_as_advanced(RapidJSON_INCLUDE_DIRS)
