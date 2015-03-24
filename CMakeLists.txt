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
set(IOTAGENT_DEPENDENCIES_ROOT /home/develop/iot/pion506
                                /home/develop/iot/log4cplus113
                                /home/develop/iot/mongo-cxx-driver
                                /home/develop/iot/json
                                /home/develop/iot/cppunit/cppunit-1.12.1
                                /home/develop/iot/gmock-1.7.0
                                /home/develop/iot/libVariant
                                /home/develop/iot/mosquitto-1.3.5)
foreach(_dep_path ${IOTAGENT_DEPENDENCIES_ROOT})
  set(IOTAGENT_DEPENDENCIES_PATHS ${IOTAGENT_DEPENDENCIES_PATHS} ${_dep_path}/include ${_dep_path}/lib)
endforeach()

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
   link_directories(${CPPUNIT_LIBS_DIR})
  endif()
  set(TEST_LIBS ${CPPUNIT_LIBRARIES})
endif()

# GMock
find_package(GMock)
if(GMOCK_FOUND)
 include_directories(${GMOCK_INCLUDE_DIR})
 link_directories(${GMOCK_LIBS_DIR})
 set(TEST_LIBS ${TEST_LIBS} gmock gtest)
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
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/specs/iot-agent-puppet.spec.in" "${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-puppet.spec" @ONLY IMMEDIATE)
set(CPACK_RPM_USER_BINARY_SPECFILE "${CMAKE_CURRENT_BINARY_DIR}/iot-agent-puppet.spec")
add_custom_target(package
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-base.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-ul.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mqtt.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mocks.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-tt.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-puppet.spec
    )

