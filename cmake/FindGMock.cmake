# Find the Google Mock headers and libraries
# GMOCK_INCLUDE_DIR where to find gmock.h
set(GMOCK_FIND_OPTIONS ${IOT_FIND_OPTIONS})
if (GMOCK_ROOT)
set(GMOCK_FIND_OPTIONS NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
endif()

find_path ( GMOCK_INCLUDE_DIR gmock/gmock.h
	${GMOCK_ROOT}/include
	$ENV{GMOCK_ROOT}/include
${_GMOCK_IOT}/include
${GMOCK_FIND_OPTIONS})
find_library(GMOCK_LIBRARY
	NAMES gmock
	PATHS ${_GMOCK_IOT}/lib
	${GMOCK_ROOT}/lib
	$ENV{GMOCK_ROOT}/lib
	${GMOCK_FIND_OPTIONS})
if (GMOCK_LIBRARY)
	message(STATUS "${GMOCK_LIBRARY}")
  get_filename_component(GMOCK_LIBS_DIR ${GMOCK_LIBRARY} DIRECTORY)
	set(GMOCK_LIBRARIES "gmock")
	set(GMOCK_FOUND TRUE)
endif()
include ( EmbeddedGTest )
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args( GMOCK DEFAULT_MSG GMOCK_INCLUDE_DIR
GMOCK_LIBS_DIR
GMOCK_LIBRARIES
)
mark_as_advanced ( GMOCK_INCLUDE_DIR GMOCK_LIBRARIES GMOCK_LIBS_DIR )
