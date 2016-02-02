cmake_minimum_required(VERSION 2.8.12)
project (iot-agent)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/version.h)
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
set (CMAKE_SHARED_MODULE_PREFIX "")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_SPIRIT_THREADSAFE -fPIC")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DBOOST_SPIRIT_THREADSAFE -fPIC")
set (CMAKE_CXX_FLAGS_DEBUGCOVERAGE "-g -O0 --coverage")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
 set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
 set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()
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

# LIBS
set(IOTAGENT_LIBS "")

include(ExternalProject)

ExternalProject_Add(boost PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third_party/boost
	URL http://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.bz2/download
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ./bootstrap.sh
	--with-libraries=thread,filesystem,system,regex,date_time,test
	BUILD_COMMAND ./b2 install --prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/boost
	INSTALL_COMMAND ""
	)
set(BOOST_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party/boost)
set(Boost_INCLUDE_DIRS ${BOOST_DIRECTORY}/include)
set(Boost_LIBRARY_DIRS ${BOOST_DIRECTORY}/lib)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
set(IOTAGENT_LIBS ${IOTAGENT_LIBS} boost_thread boost_filesystem boost_system
	boost_regex boost_date_time)
set(INSTALL_BOOST "cp -P ${Boost_LIBRARY_DIRS}/libboost_thread.so* %{buildroot}/usr/local/iot/lib/\n${INSTALL_BOOST}")
set(INSTALL_BOOST "cp -P ${Boost_LIBRARY_DIRS}/libboost_filesystem.so* %{buildroot}/usr/local/iot/lib/\n${INSTALL_BOOST}")
set(INSTALL_BOOST "cp -P ${Boost_LIBRARY_DIRS}/libboost_system.so* %{buildroot}/usr/local/iot/lib/\n${INSTALL_BOOST}")
set(INSTALL_BOOST "cp -P ${Boost_LIBRARY_DIRS}/libboost_regex.so* %{buildroot}/usr/local/iot/lib/\n${INSTALL_BOOST}")
set(INSTALL_BOOST "cp -P ${Boost_LIBRARY_DIRS}/libboost_date_time.so* %{buildroot}/usr/local/iot/lib/\n${INSTALL_BOOST}")

# Log4cplus
# It use logger library recommended in Pion Library.
set(LOG4CPLUS_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party/log4cplus)
ExternalProject_Add(logger-log4cplus PREFIX
		${CMAKE_CURRENT_BINARY_DIR}/third_party/log4cplus
	URL http://sourceforge.net/projects/log4cplus/files/log4cplus-stable/1.1.3/log4cplus-1.1.3-rc4.tar.bz2/download
	CONFIGURE_COMMAND
	${CMAKE_CURRENT_BINARY_DIR}/third_party/log4cplus/src/logger-log4cplus/configure
	--enable-release-version=yes --enable-so-version=no
	--prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/log4cplus
	INSTALL_COMMAND make install
	)

set(LOG4CPLUS_INCLUDE_DIR ${LOG4CPLUS_DIRECTORY}/include)
set(LOG4CPLUS_LIBS_DIR ${LOG4CPLUS_DIRECTORY}/lib)
set(LOG4CPLUS_LIBRARIES log4cplus)
set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${LOG4CPLUS_LIBRARIES})
set(INSTALL_LOG4CPLUS ${LOG4CPLUS_LIBS_DIR}/lib${LOG4CPLUS_LIBRARIES}*.so)
include_directories(${LOG4CPLUS_INCLUDE_DIR})
link_directories(${LOG4CPLUS_LIBS_DIR})

# Pion
set(PION_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party/pion)
ExternalProject_Add(pion-library DEPENDS boost logger-log4cplus PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third_party/pion
	URL http://downloads.sourceforge.net/project/pion/5.0.6/pion-5.0.6.tar.bz2
	CONFIGURE_COMMAND
	${CMAKE_CURRENT_BINARY_DIR}/third_party/pion/src/pion-library/configure
	--with-boost=${BOOST_DIRECTORY} --without-bzlib
	--with-log4cplus=${LOG4CPLUS_DIRECTORY}
	--prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/pion
	BUILD_IN_SOURCE 1
	INSTALL_COMMAND make install
	)

set(Pion_LIBS_DIR ${PION_DIRECTORY}/lib)
set(Pion_INCLUDE_DIR ${PION_DIRECTORY}/include)
set(Pion_LIBRARIES pion)
include_directories(${Pion_INCLUDE_DIR})
link_directories(${Pion_LIBS_DIR})
set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${Pion_LIBRARIES})
set(INSTALL_PION ${Pion_LIBS_DIR}/lib${Pion_LIBRARIES}*.so)


# MongoDB driver
set(MONGODB_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party/mongo-driver)
ExternalProject_Add(mongo-driver PREFIX
		${CMAKE_CURRENT_BINARY_DIR}/third_party/mongo-driver
		GIT_REPOSITORY git@github.com:mongodb/mongo-cxx-driver.git
		GIT_TAG legacy-1.1.0
	CONFIGURE_COMMAND ""
	BUILD_IN_SOURCE 1
	BUILD_COMMAND scons
	--prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/mongo-driver
	--extrapath=${BOOST_DIRECTORY}
  --libpath=${BOOST_DIRECTORY}/lib
  --cpppath=${BOOST_DIRECTORY}/include
  --boost-lib-search-suffixes=
  --extralib=boost_system,boost_regex,boost_thread
  --sharedclient install
	INSTALL_COMMAND ""
	)

set(MongoDB_INCLUDE_DIR ${MONGODB_DIRECTORY}/include)
set(MongoDB_LIBS_DIR ${MONGODB_DIRECTORY}/lib)
set(MongoDB_LIBRARIES mongoclient)
set(IOTAGENT_LIBS ${MongoDB_LIBRARIES} ${IOTAGENT_LIBS})
include_directories(${MongoDB_INCLUDE_DIR})
link_directories(${MongoDB_LIBS_DIR})
set(INSTALL_MONGOCLIENT ${MongoDB_LIBS_DIR}/lib${MongoDB_LIBRARIES}.so)


# rapidjson
include_directories(third_party/rapidjson/include)

# Mosquitto
if (MQTT)
set(WITH_CARES ON)
if (WITH_CARES)
      ExternalProject_Add(libcares PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third_party/libcares SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/c-ares-1.10.0 CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/third_party/c-ares-1.10.0/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/libcares BUILD_COMMAND ${MAKE})
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
      ExternalProject_Add(broker-mqtt DEPENDS libcares PREFIX ${CMAKE_CURRENT_BINARY_DIR}/third_party/broker-mqtt SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mosquitto-1.4.4 CMAKE_ARGS -DC-ARES-ROOT=${CMAKE_CURRENT_BINARY_DIR}/third_party/libcares -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/third_party/broker-mqtt)
else()
      set(WITH_SRV OFF)
      ExternalProject_Add(broker-mqtt DEPENDS libcares PREFIX ${CMAKE_CURRENT_BINARY_DIR}/broker-mqtt SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mosquitto-1.4.4 CMAKE_ARGS -DWITH_SRV=OFF -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/broker-mqtt)
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_party/mosquitto-1.4.4 ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mosquitto-1.4.4/lib/cpp)

set(MOSQUITTO_FOUND TRUE)
set(IOTAGENT_LIBS ${IOTAGENT_LIBS})
set(MOSQUITTO_LIBS_DIR ${CMAKE_CURRENT_BINARY_DIR}/third_party/broker-mqtt)
link_directories(${MOSQUITTO_LIBS_DIR}/lib)
include_directories(${MOSQUITTO_LIBS_DIR}/include)
endif()

# Cppunit
set(CPPUNIT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/third_party/cppunit)
ExternalProject_Add(test-cppunit PREFIX
	${CMAKE_CURRENT_BINARY_DIR}/third_party/cppunit
	URL http://sourceforge.net/projects/cppunit/files/cppunit/1.12.1/cppunit-1.12.1.tar.gz/download
	CONFIGURE_COMMAND
	${CMAKE_CURRENT_BINARY_DIR}/third_party/cppunit/src/test-cppunit/configure LDFLAGS=-ldl
	--prefix=${CMAKE_CURRENT_BINARY_DIR}/third_party/cppunit
	BUILD_IN_SOURCE 1
	BUILD_COMMAND make
	INSTALL_COMMAND make install
	)

set(CPPUNIT_LIBS_DIR ${CPPUNIT_DIRECTORY}/lib)
set(CPPUNIT_INCLUDE_DIR ${CPPUNIT_DIRECTORY}/include)
set(CPPUNIT_LIBRARIES cppunit)
include_directories(${CPPUNIT_INCLUDE_DIR})
link_directories(${CPPUNIT_LIBS_DIR})
set(TEST_LIBS ${CPPUNIT_LIBRARIES})

# GMock
add_subdirectory(third_party/gmock-1.7.0)
set(GMOCK_INCLUDE_DIR third_party/gmock-1.7.0/include third_party/gmock-1.7.0/gtest/include)
set(GMOCK_LIBS_DIR third_party/gmock-1.7.0)
include_directories(${GMOCK_INCLUDE_DIR})
link_directories(${GMOCK_LIBS_DIR})
set(TEST_LIBS ${TEST_LIBS} gmock)

#Variant
set(LIBVARIANT_ENABLE_XML OFF CACHE BOOL "")
set(LIBVARIANT_ENABLE_YAML OFF CACHE BOOL "")
set(LIBVARIANT_ENABLE_CURL OFF CACHE BOOL "")
set(LIBVARIANT_ENABLE_MSGPACK OFF CACHE BOOL "")
add_subdirectory(third_party/variant)
add_definitions( -DLIBVARIANT=1 )
set(LIBVARIANT_INCLUDE_DIR
	 ${CMAKE_CURRENT_BINARY_DIR}/third_party/variant/include
	 ${CMAKE_CURRENT_SOURCE_DIR}/third_party/variant/include)
include_directories(${LIBVARIANT_INCLUDE_DIR})

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

###  GENERAL PURPOSE
# util Test:  general test about  cache, csv, url
add_test(NAME UtilTest COMMAND util_tests)
# ngsi Test: tests about ngsi operation
add_test(NAME NgsiTest COMMAND ngsi_tests)
# CB Communicator: general functions to communicate with Context Broker
add_test(NAME CBCommTest COMMAND cb_comm_tests)
# Mongo: general functions to use mongo collections
add_test(NAME MongoTest COMMAND mongo_tests)
# OAuth: general functions to check oauth privileges
add_test(NAME OAuthTest COMMAND oauth_tests)
# TCP: simple test to check tcp plugin
add_test(NAME TcpTest COMMAND tcp_tests)

### Administration APIs
# API reference for IotAgent adminitration
add_test(NAME AdminTest COMMAND admin_tests)
# API reference for Iot Manager adminitration
add_test(NAME AdminManagerTest COMMAND admin_manager_tests)

###  PROTOCOLS
# Tutorial basic sample protocol
add_test(NAME SampleTest COMMAND sample_tests)
# UltraLight protocol
add_test(NAME Ul20Test COMMAND ul20_tests)
# Thinking things protocol
add_test(NAME ttTest COMMAND tt_tests)
if(MOSQUITTO_FOUND)
# MQTT UltraLight protocol
add_test(NAME MqttTest COMMAND mqtt_tests)
endif()
# Modbus 
add_test(NAME ModbusTest COMMAND modbus_tests)

file(READ ${CMAKE_CURRENT_SOURCE_DIR}/BaseChangeLog.txt BASECHANGELOG)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/MqttChangeLog.txt MQTTCHANGELOG)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/TTChangeLog.txt TTCHANGELOG)
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/ULChangeLog.txt ULCHANGELOG)

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
	    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/pack/Linux/RPM/BUILD
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-base.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-ul.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mqtt.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-mocks.spec
      COMMAND rpmbuild -bb  ${CMAKE_CURRENT_BINARY_DIR}/specs/iot-agent-tt.spec
    )

