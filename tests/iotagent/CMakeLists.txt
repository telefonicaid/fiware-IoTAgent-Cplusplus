
include_directories(${PROJECT_SOURCE_DIR}/src)

###  GENERAL PURPOSE
# util Test:  general test about  cache, csv, url
add_executable(util_tests util_tests.cc UtilFunctionTest.cc UtilFunctionTest.h)
target_link_libraries(util_tests StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})

#json Test: JSON tests of parsing NGSI messages
add_executable(json_tests JsonTest.cc json_test.cc)
target_link_libraries(json_tests StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})


# ngsi Test: tests about ngsi operation
add_executable(ngsi_tests json_test.cc JsonTest.cc)
target_link_libraries(ngsi_tests StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})
# CB Communicator: general functions to communicate with Context Broker
add_executable(cb_comm_tests cbCommTest.cc cb_comm_test.cc)
target_link_libraries(cb_comm_tests iotmock StaticLibraryIota ${IOTAGENT_LIBS}
	${TEST_LIBS})
# Mongo: general functions to use mongo collections
add_executable(mongo_tests mongoTest.cc mongo_test.cc)
target_link_libraries(mongo_tests iotmock StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})
# OAuth: general functions to check oauth privileges
add_executable(oauth_tests oauthTest.cc oauth_test.cc)
target_link_libraries(oauth_tests iotmock StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})
# TCP: test a simple plugin by tcp
add_executable(tcp_tests tcpTest.cc tcp_test.cc)
target_link_libraries(tcp_tests StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})

### Administration APIs
# API reference for IotAgent adminitration
add_executable(admin_tests adminTest.cc admin_tests.cc)
target_link_libraries(admin_tests iotmock StaticLibraryIota ${IOTAGENT_LIBS} ${TEST_LIBS})

# API reference for Iot Manager adminitration
add_executable(admin_manager_tests adminManagerTest.cc admin_mgmt_test.cc baseTest.cc)
target_link_libraries(admin_manager_tests iotmock StaticUL20Service StaticLibraryIota ${IOTAGENT_LIBS}
        ${TEST_LIBS})

###  PROTOCOLS
# Tutorial basic sample protocol
add_executable(sample_tests sampleTest.cc sample_test.cc baseTest.cc)
target_link_libraries(sample_tests StaticTestService iotmock StaticLibraryIota ${IOTAGENT_LIBS}
        ${TEST_LIBS})

# UltraLight protocol
add_executable(ul20_tests ul20Test.cc ul20_test.cc  baseTest.cc)
target_link_libraries(ul20_tests StaticUL20Service iotmock StaticLibraryIota ${IOTAGENT_LIBS}
	${TEST_LIBS})

# Thinking things protocol
add_executable(tt_tests ttTest.cc tt_test.cc TTBufferTests.cc)
target_link_libraries(tt_tests esp StaticTTService iotmock StaticLibraryIota
	${IOTAGENT_LIBS} ${TEST_LIBS})
if(MOSQUITTO_FOUND)
# MQTT UltraLight protocol
include_directories(${MOSQUITTO_INCLUDE_DIR})
link_directories(${MOSQUITTO_LIBS_DIR})

add_executable(mqtt_tests mqttTest.cc mqtt_test.cc )
target_link_libraries(mqtt_tests esp_mqtt mosquittopp mosquitto StaticMqttService iotmock StaticLibraryIota
	${IOTAGENT_LIBS} ${MOSQUITTO_LIBRARIES} ${TEST_LIBS})

endif()

# Modbus
add_executable(modbus_tests modbusTest.cc modbus_test.cc)
target_link_libraries(modbus_tests StaticLibraryIota ${IOTAGENT_LIBS}
	${TEST_LIBS})

