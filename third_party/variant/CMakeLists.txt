project(LibVariant)

cmake_minimum_required(VERSION 2.6)

enable_testing()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

include(CheckCXXSourceCompiles)
include(CheckIncludeFileCXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	set(EXAMPLES_AND_TESTS on)
	set(LIBVARIANT_DOC_TARGET doc)
	option(BUILD_SHARED_LIBS "Build shared libraries." OFF)
else()
	set(EXAMPLES_AND_TESTS off)
	set(LIBVARIANT_DOC_TARGET libvariant-doc)
endif()

option(LIBVARIANT_BUILD_WALL "build Variant with the -Wall flag" ON)
option(LIBVARIANT_BUILD_DEBUG "build Variant with the additional debug flags" OFF)
option(LIBVARIANT_BUILD_TESTS "build the Variant tests" ${EXAMPLES_AND_TESTS})
option(LIBVARIANT_BUILD_EXAMPLES "build the Variant examples" ${EXAMPLES_AND_TESTS})
option(LIBVARIANT_BUILD_PROFILE "build Variant with profiling flags" OFF)
option(LIBVARIANT_ENABLE_YAML "Enable YAML encoding" ON)
option(LIBVARIANT_ENABLE_XML "Enable XML encoding" ON)
option(LIBVARIANT_ENABLE_MSGPACK "Enable msgpack encoding" OFF)
option(LIBVARIANT_ENABLE_CURL "Enable cURL related functionality in the SchemaLoader" ON)

set(LIBVARIANT_MAJOR_VERSION 1)
set(LIBVARIANT_MINOR_VERSION 0)
set(LIBVARIANT_PATCH_VERSION 1)
set(LIBVARIANT_VERSION
	  ${LIBVARIANT_MAJOR_VERSION}.${LIBVARIANT_MINOR_VERSION}.${LIBVARIANT_PATCH_VERSION})
   
# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE STRING "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE STRING "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include CACHE STRING "Installation directory for header files")

set(DEF_INSTALL_CMAKE_DIR "${INSTALL_LIB_DIR}/cmake/LibVariant")
set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE STRING
	"Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(p LIB BIN INCLUDE CMAKE)
	set(var INSTALL_${p}_DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
	endif()
endforeach()

if(LIBVARIANT_BUILD_WALL)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if (LIBVARIANT_BUILD_DEBUG)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winline -Wfatal-errors")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Winline -Wfatal-errors")
endif()

if (LIBVARIANT_BUILD_PROFILE)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fprofile-arcs -ftest-coverage")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fprofile-arcs -ftest-coverage")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DENABLE_STACKTRACE")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_STACKTRACE")

# setup include directories
include_directories("include")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# libraries to depends upon
if(LIBVARIANT_ENABLE_XML)
	find_package(LibXml2)
	if(LIBXML2_FOUND)
		include_directories(${LIBXML2_INCLUDE_DIR})
		add_definitions(${LIBXML2_DEFINITIONS})
		add_definitions("-DENABLE_XML")
		list(APPEND EXTRA_LIBS ${LIBXML2_LIBRARIES})
	else()
		message(WARNING " Could not find libxml2, disabling.")
		set(LIBVARIANT_ENABLE_XML OFF)
	endif()
endif()

if(LIBVARIANT_ENABLE_YAML)
	find_package(LibYAML)
	if(LIBYAML_FOUND)
		include_directories(${LIBYAML_INCLUDE_DIR})
		add_definitions("-DENABLE_YAML")
		list(APPEND EXTRA_LIBS ${LIBYAML_LIBRARIES})
	else()
		message(WARNING " Could not find libyaml, disabling.")
		set(LIBVARIANT_ENABLE_YAML OFF)
	endif()
endif()

if(LIBVARIANT_ENABLE_MSGPACK)
	find_package(MsgPack)
	if(MSGPACK_FOUND)
		include_directories(${MSGPACK_INCLUDE_DIR})
		add_definitions("-DENABLE_MSGPACK")
		list(APPEND EXTRA_LIBS ${MSGPACK_LIBRARIES})
	else()
		message(WARNING " Could not find msgpack, disabling")
		set(LIBVARIANT_ENABLE_MSGPACK OFF)
	endif()
endif()

if (LIBVARIANT_ENABLE_CURL)
	find_package(CURL)
	if (CURL_FOUND)
		include_directories(${CURL_INCLUDE_DIR})
		add_definitions("-DENABLE_CURL")
		list(APPEND EXTRA_LIBS ${CURL_LIBRARIES})
	else()
		message(INFO " CURL not found, disabling CURL functionality.")
		set(LIBVARIANT_ENABLE_CURL OFF)
	endif()
endif()

# We want to following checks to try compiling with the same flags that we
# have already setup
set(CMAKE_REQUIRED_FLAGS ${CMAKE_CXX_FLAGS})

check_cxx_source_compiles( "
\#include <memory>\n
int main() {
	std::shared_ptr<int> i;
	return 0;
}
"
	HAVE_CXX11_SHAREDPTR )

if (NOT HAVE_CXX11_SHAREDPTR)
	check_include_file_cxx("tr1/memory" HAVE_TR1_SHAREDPTR)

	if (NOT HAVE_TR1_SHAREDPTR)
		message(STATUS "Fallback to using boost shared_ptr")
		find_package(Boost REQUIRED)
		include_directories(${Boost_INCLUDE_DIRS})
		set(USE_BOOST_SHAREDPTR 1)
	endif()
endif()

check_cxx_source_compiles("
int main() {
	long long i = 0;
	unsigned long long u = 0;
	return i + u;
}
"
	HAVE_LONG_LONG)

if (HAVE_LONG_LONG)
	add_definitions("-DHAVE_LONG_LONG=1")
endif()

# add sub directories
add_subdirectory(src)
if (LIBVARIANT_BUILD_TESTS)
	add_subdirectory(test)
endif()
if (LIBVARIANT_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()


# Build documentation
find_package(Doxygen)
if (DOXYGEN_FOUND)
	configure_file(Doxyfile.in ${PROJECT_BINARY_DIR}/Doxyfile @ONLY IMMEDIATE)
	add_custom_target(${LIBVARIANT_DOC_TARGET}
	   	COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_BINARY_DIR}/Doxyfile
		SOURCES ${PROJECT_BINARY_DIR}/Doxyfile)
endif()


# Add targets to the build-tree export set
set(LIBVARIANT_EXPORTED_LIBS Variant)

#export(TARGETS ${LIBVARIANT_EXPORTED_LIBS}
#	FILE "${PROJECT_BINARY_DIR}/LibVariantLibraryDepends.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
#export(PACKAGE LibVariant) # NOT IMPLEMENTED IN cmake 2.6 on RHEL6


# Create a LibVariantBuildTreeSettings.cmake file for the use from the build tree
configure_file(LibVariantBuildTreeSettings.cmake.in
	"${PROJECT_BINARY_DIR}/LibVariantBuildTreeSettings.cmake" @ONLY)

# Create the LibVariantConfig.cmake and LibVariantConfigVersion files
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
	"${INSTALL_INCLUDE_DIR}")
configure_file(LibVariantConfig.cmake.in
	"${PROJECT_BINARY_DIR}/LibVariantConfig.cmake" @ONLY)
configure_file(LibVariantConfigVersion.cmake.in
	"${PROJECT_BINARY_DIR}/LibVariantConfigVersion.cmake" @ONLY)

install(FILES 
	"${PROJECT_BINARY_DIR}/LibVariantConfig.cmake"
	"${PROJECT_BINARY_DIR}/LibVariantConfigVersion.cmake"
	DESTINATION "${INSTALL_CMAKE_DIR}")

# Install the export set for use with the install-tree
#install(EXPORT LibVariantLibraryDepends
#DESTINATION "${INSTALL_CMAKE_DIR}"
#COMPONENT dev)

# Create the package config file
configure_file(libvariant.pc.in
	"${PROJECT_BINARY_DIR}/libvariant.pc" @ONLY)

install(FILES
	"${PROJECT_BINARY_DIR}/libvariant.pc"
	DESTINATION "${INSTALL_LIB_DIR}/pkgconfig")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/include/Variant/Config.h)

FILE(GLOB install_headers RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
	include/Variant/*.h)
#install(FILES ${install_headers} DESTINATION ${INSTALL_INCLUDE_DIR}/Variant/)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/include/Variant/Config.h DESTINATION
#${INSTALL_INCLUDE_DIR}/Variant/)

# Am I the root CMakeLists.txt?
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
	include(MakeTBZ2)
	add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)
else()
	# set parent variables as if it used find_package(testlib2)
	set(LIBVARIANT_LIBRARIES Variant PARENT_SCOPE)
	get_directory_property(LIBVARIANT_INCLUDE_DIRS INCLUDE_DIRECTORIES)
	set(LIBVARIANT_INCLUDE_DIRS ${LIBVARIANT_INCLUDE_DIRS} PARENT_SCOPE)
	set(LIBVARIANT_FOUND TRUE PARENT_SCOPE)
endif()
