# This module defines
# CPPUNIT_INCLUDE_DIR, where to find tiff.h, etc.
# CPPUNIT_LIBRARIES, the libraries to link against to use CppUnit.
# CPPUNIT_FOUND, If false, do not try to use CppUnit.
# CPPUNIT_LIBS_DIR


 set(CPPUNIT_FIND_OPTIONS ${IOT_FIND_OPTIONS})
 if (CPPUNIT_ROOT)
 set(CPPUNIT_FIND_OPTIONS NO_DEFAULT_PATH NO_CMAKE_FIND_ROOT_PATH)
 endif()

FIND_PATH(CPPUNIT_INCLUDE_DIR cppunit/TestCase.h
	PATHS
	${CPPUNIT_ROOT}/include
	$ENV{CPPUNIT_ROOT}/include
  ${_CPPUNIT_IOT}/include
  ${CPPUNIT_FIND_OPTIONS} 
)

if (CPPUNIT_USE_STATIC_LIBS)
set(_cppunit_LIB_NAME "libcppunit.a")
else()
set(_cppunit_LIB_NAME "cppunit")
endif()
FIND_LIBRARY(_CPPUNIT_LIBRARIES NAMES ${_cppunit_LIB_NAME}
	             PATHS
	             ${CPPUNIT_ROOT}/lib
	             $ENV{CPPUNIT_ROOT}/lib
               ${_CPPUNIT_IOT}/lib
               ${CPPUNIT_FIND_OPTIONS} 
            )

IF(CPPUNIT_INCLUDE_DIR AND _CPPUNIT_LIBRARIES)
    SET(CPPUNIT_FOUND TRUE)
    get_filename_component(CPPUNIT_LIBS_DIR ${_CPPUNIT_LIBRARIES} DIRECTORY)
		if (CPPUNIT_USE_STATIC_LIBS)
			set(CPPUNIT_LIBRARIES ${_CPPUNIT_LIBRARIES})
		else()
			set(CPPUNIT_LIBRARIES ${_cppunit_LIB_NAME})
		endif()
ELSE()
  message (STATUS  "not found cppunit")
  SET(CPPUNIT_FOUND FALSE)
ENDIF()
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CppUnit DEFAULT_MSG CPPUNIT_LIBRARIES CPPUNIT_INCLUDE_DIR CPPUNIT_LIBS_DIR)
mark_as_advanced(CPPUNIT_LIBRARIES CPPUNIT_INCLUDE_DIR CPPUNIT_LIBS_DIR)
