cmake_minimum_required(VERSION 2.8.12)
project (iot-agent)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/include/version.h)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set (CMAKE_SHARED_MODULE_PREFIX "")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SPIRIT_THREADSAFE -fPIC")
set (CMAKE_CXX_FLAGS_DEBUGCOVERAGE "-g -O0 --coverage")
if (CMAKE_BUILD_TYPE STREQUAL "DebugCoverage")
 set (TEST_LIBS cppunit gcov)
endif()
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGE_NAME iot-agent)

# Version has X.Y.Z format
set(CPACK_PACKAGE_VERSION ${GIT_VERSION})

# Commit is number_of_commit.commit_identifier
set(CPACK_PACKAGE_RELEASE ${GIT_COMMIT})

set(CPACK_PACKAGE_CONTACT "")
set(CPACK_PACKAGE_VENDOR "PDI")
set(CPACK_PACKAGING_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")

# Where cmake modules are defined
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# LIBS
set(IOTAGENT_LIBS "")
# View cmake directory. NO_DEFAULT_PATH is defined in cmake modules to find
# dependencies

if(IOT_ROOT)

file(GLOB_RECURSE _PION_I_  ${IOT_ROOT}/libpion-5.0.so)
if(_PION_I_)
list(GET _PION_I_ 0 _PION_I_)
get_filename_component(_PION_IOT ${_PION_I_} PATH)
set(_PION_IOT ${_PION_IOT}/..)
endif(_PION_I_)

file(GLOB_RECURSE _LOG4CPLUS_I_  ${IOT_ROOT}/liblog4cplus.so)
if(_LOG4CPLUS_I_)
list(GET _LOG4CPLUS_I_ 0 _LOG4CPLUS_I_)
get_filename_component(_LOG4CPLUS_IOT ${_LOG4CPLUS_I_} PATH)
set(_LOG4CPLUS_IOT ${_LOG4CPLUS_IOT}/..)
endif(_LOG4CPLUS_I_)

file(GLOB_RECURSE _MONGOCLIENT_I_  ${IOT_ROOT}/libmongoclient.so)
if(_MONGOCLIENT_I_)
list(GET _MONGOCLIENT_I_ 0 _MONGOCLIENT_I_)
get_filename_component(_MONGOCLIENT_IOT ${_MONGOCLIENT_I_} PATH)
set(_MONGOCLIENT_IOT ${_MONGOCLIENT_IOT}/..)
endif(_MONGOCLIENT_I_)

# check for boost 1.55
if(BOOST_ROOT)
message(STATUS "BOOST_ROOT:" ${BOOST_ROOT})
else()
file(GLOB_RECURSE _BOOST_I_  ${IOT_ROOT}/libboost_filesystem.so.1.55.0)
if(_BOOST_I_)
list(GET _BOOST_I_ 0 _BOOST_I_)
get_filename_component(_BOOST_IOT ${_BOOST_I_} PATH)
set(_BOOST_IOT ${_BOOST_IOT}/..)
set(BOOST_ROOT ${_BOOST_IOT})
message(STATUS "BOOST_ROOT:" ${BOOST_ROOT})
endif(_BOOST_I_)
endif(BOOST_ROOT)

file(GLOB_RECURSE _JSON_I_  ${IOT_ROOT}/rapidjson.h)
if(_JSON_I_)
list(GET _JSON_I_ 0 _JSON_I_)
get_filename_component(_JSON_IOT ${_JSON_I_} PATH)
set(_JSON_IOT ${_JSON_IOT}/../..)
endif(_JSON_I_)
file(GLOB_RECURSE _MOSQUITTO_I_  ${IOT_ROOT}/libmosquitto.so)
if(_MOSQUITTO_I_)
list(GET _MOSQUITTO_I_ 0 _MOSQUITTO_I_)
get_filename_component(_MOSQUITTO_IOT ${_MOSQUITTO_I_} PATH)
set(_MOSQUITTO_IOT ${_MOSQUITTO_IOT}/..)
endif(_MOSQUITTO_I_)
file(GLOB_RECURSE _LIBVARIANT_I_  ${IOT_ROOT}/libVariant.a)
if(_LIBVARIANT_I_)
list(GET _LIBVARIANT_I_ 0 _LIBVARIANT_I_)
get_filename_component(_LIBVARIANT_IOT ${_LIBVARIANT_I_} PATH)
set(_LIBVARIANT_IOT ${_LIBVARIANT_IOT}/..)
endif(_LIBVARIANT_I_)
file(GLOB_RECURSE _GMOCK_I_  ${IOT_ROOT}/libgmock.a)
if(_GMOCK_I_)
list(GET _GMOCK_I_ 0 _GMOCK_I_)
get_filename_component(_GMOCK_IOT_L ${_GMOCK_I_} PATH)
set(_GMOCK_IOT ${_GMOCK_IOT_L}/..)
endif(_GMOCK_I_)
file(GLOB_RECURSE _CPPUNIT_I_  ${IOT_ROOT}/libcppunit.a)

if(_CPPUNIT_I_)
FOREACH(infileName ${_CPPUNIT_I_})
  get_filename_component(_CPPUNIT_I_L ${infileName} PATH)
  set(CPPUNIT_TESTRUNNER ${_CPPUNIT_I_L}/../include/cppunit/TestRunner.h)
  if(EXISTS ${CPPUNIT_TESTRUNNER})
    set(_CPPUNIT_IOT ${_CPPUNIT_I_L}/..)
  endif()
ENDFOREACH(infileName)
endif(_CPPUNIT_I_)

endif(IOT_ROOT)

# This version only is tested and checked with dynamic libraries
# Dependencies are included in rpms.
# Pion Library (depends on log4cplus and boost)
set(Pion_USE_STATIC_LIBS OFF)
find_package(Pion)
if (PION_FOUND)
  include_directories(${Pion_INCLUDE_DIR})
  if (NOT Pion_USE_STATIC_LIBS)
   link_directories(${Pion_LIBS_DIR})
   set(INSTALL_PION ${Pion_LIBS_DIR}/lib${Pion_LIBRARIES}-${Pion_VER_MAJOR}.${Pion_VER_MINOR}.so)
  endif()
  set(IOTAGENT_LIBS ${Pion_LIBRARIES})
else()
  message(FATAL_ERROR "Pion library not found.")
endif()

# Boost 1.55.0
set(Boost_USE_STATIC_LIBS OFF)
find_package(Boost 1.55.0 COMPONENTS thread filesystem system regex date_time)
if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  message(STATUS "Boost include dirs ${Boost_INCLUDE_DIRS} ${Boost_LIBRARIES}")
	set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${Boost_LIBRARIES})
	foreach(boost_lib ${Boost_LIBRARIES})
	  get_filename_component(Boost_Lib ${boost_lib} NAME_WE)
    set(INSTALL_BOOST "cp ${boost_lib} %{buildroot}/usr/local/iot/lib/${Boost_Lib}.so.${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}\n${INSTALL_BOOST}")
	endforeach(boost_lib)
else()
  message(FATAL_ERROR "Boost library (1.55.0) not found. Define BOOST_ROOT environment variable to set where boost 1.55.0 is installed")
endif()


# Log4cplus
# It use logger library recommended in Pion Library.
set(Log4Cplus_USE_STATIC_LIBS OFF)
find_package(Log4cplus)
if (LOG4CPLUS_FOUND)
  include_directories(${LOG4CPLUS_INCLUDE_DIR})
  if (NOT Log4Cplus_USE_STATIC_LIBS)
   link_directories(${LOG4CPLUS_LIBS_DIR})
   message(STATUS "link:" ${LOG4CPLUS_LIBS_DIR})
   set(INSTALL_LOG4CPLUS ${LOG4CPLUS_LIBS_DIR}/lib${LOG4CPLUS_LIBRARIES}-${LOG4CPLUS_VER_MAJOR}.${LOG4CPLUS_VER_MINOR}.so)
  endif()
  set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${LOG4CPLUS_LIBRARIES})
else()
  message(FATAL_ERROR "log4cplus library not found.")
endif()



# Mongo
set(MONGODB_USE_STATIC_LIBS OFF)
find_package(MongoDB)
if(MongoDB_FOUND)
 include_directories(${MongoDB_INCLUDE_DIR})
  if (NOT MONGODB_USE_STATIC_LIBS)
   link_directories(${MongoDB_LIBS_DIR})
   set(INSTALL_MONGOCLIENT ${MongoDB_LIBS_DIR}/lib${MongoDB_LIBRARIES}.so)
   message(STATUS "MongoDB library directory ${MongoDB_LIBS_DIR}")
  endif()
  set(IOTAGENT_LIBS ${MongoDB_LIBRARIES} ${IOTAGENT_LIBS})
endif()

# rapidjson
find_package(RapidJSON)
if(RAPIDJSON_FOUND)
 include_directories(${RapidJSON_INCLUDE_DIRS})
endif()

# Mosquitto
find_package(Mosquitto)
if (MOSQUITTO_FOUND)
	link_directories(${MOSQUITTO_LIBS_DIR})
	include_directories(${MOSQUITTO_LIBS_DIR} ${MOSQUITTO_LIBS_DIR}/cpp)
	set(IOTAGENT_LIBS ${IOTAGENT_LIBS})
	message(STATUS "Mosquitto library directory ${MOSQUITTO_LIBS_DIR}")
else()
    message(STATUS "Mosquitto not found")
endif()

# Cppunit
set(CPPUNIT_USE_STATIC_LIBS OFF)
find_package(CppUnit)
if(CPPUNIT_FOUND)
 include_directories(${CPPUNIT_INCLUDE_DIR})
  if (NOT CPPUNIT_USE_STATIC_LIBS)
   message(STATUS "link cppunit: " ${CPPUNIT_LIBS_DIR})
   link_directories(${CPPUNIT_LIBS_DIR})
  endif()
  set(TEST_LIBS ${CPPUNIT_LIBRARIES})
endif()

# GMock
find_package(GMock)
if(GMOCK_FOUND)
 include_directories(${GMOCK_INCLUDE_DIR})
 link_directories(${GMOCK_LIBS_DIR})
 set(TEST_LIBS ${TEST_LIBS} gmock)
endif()

#libVariant
find_package(LIBVARIANT)
if(LIBVARIANT_FOUND)
 include_directories(${LIBVARIANT_INCLUDE_DIR})
 link_directories(${LIBVARIANT_LIBS_DIR})
 add_definitions( -DLIBVARIANT=1 )
 message(STATUS "LibVariant include dirs ${LIBVARIANT_INCLUDE_DIR}")
endif()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
message(STATUS "Doxygen found.")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/doxyfile.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
add_custom_target(doc
${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
COMMENT "Generating API documentation with Doxygen" VERBATIM
)
else()
  message(STATUS "Doxygen not found.")
endif()

set(IOTAGENT_LIBS ${IOTAGENT_LIBS} pthread rt ssl crypto dl)

include_directories(src/ ${CMAKE_CURRENT_BINARY_DIR}/include)

add_subdirectory(src)


enable_testing()
add_subdirectory(tests)
add_test(NAME RestTest COMMAND rest_tests)
add_test(NAME NgsiTest COMMAND ngsi_tests)
add_test(NAME ApiKeyTest COMMAND apikey_tests)
add_test(NAME CBCommTest COMMAND cb_comm_tests)
add_test(NAME AdminTest COMMAND admin_tests)
add_test(NAME AdminManagerTest COMMAND admin_manager_tests)
add_test(NAME Ul20Test COMMAND ul20_tests)
if(MOSQUITTO_FOUND)
add_test(NAME MqttTest COMMAND mqtt_tests)
add_test(NAME ttTest COMMAND tt_tests)
endif()
add_test(NAME UrlTest COMMAND url_tests)
add_test(NAME CacheTest COMMAND cache_tests)
add_test(NAME CsvReaderTest COMMAND csv_tests)
add_test(NAME OAuthTest COMMAND oauth_tests)
add_test(NAME SimplePlugingTest COMMAND sp_tests)
add_test(NAME MongoTest COMMAND mongo_tests)
add_test(NAME SamplTest COMMAND sample_tests)
add_test(NAME TcpTest COMMAND tcp_tests)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-base.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-base.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-base.spec")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-ul.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-ul.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-ul.spec")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-mqtt.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mqtt.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-mqtt.spec")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-mocks.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mocks.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-mocks.spec")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-tt.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-tt.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-tt.spec")

add_custom_target(package
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-base.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-ul.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mqtt.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mocks.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-tt.spec
    )

