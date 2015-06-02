# - Find msgpack
# Find the native Message Pack headers and libraries.
#
#  MSGPACK_INCLUDE_DIRS - where to find msgpack.hpp, etc.
#  MSGPACK_LIBRARIES    - List of libraries when using MsgPack.
#  MSGPACK_FOUND        - True if MsgPack found.


# Look for the header file.
FIND_PATH(MSGPACK_INCLUDE_DIR NAMES msgpack.hpp)

# Look for the library.
FIND_LIBRARY(MSGPACK_LIBRARY NAMES msgpack libmsgpack)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(MsgPack DEFAULT_MSG MSGPACK_LIBRARY MSGPACK_INCLUDE_DIR)
MARK_AS_ADVANCED(MSGPACK_INCLUDE_DIR MSGPACK_LIBRARY)
