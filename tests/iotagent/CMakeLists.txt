
include_directories(${PROJECT_SOURCE_DIR}/src)
add_executable(rest_tests rest_tests.cc RestFunctionTest.cc RestFunctionTest.h)
target_link_libraries(rest_tests iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})
add_executable(ngsi_tests json_test.cc JsonTest.cc)
target_link_libraries(ngsi_tests iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})
add_executable(apikey_tests ApiKeyRunner.cc ApikeyTest.cc)
target_link_libraries(apikey_tests iota_a ${IOTAGENT_LIBS} Variant ${TEST_LIBS})
add_executable(cb_comm_tests cbCommTest.cc cb_comm_test.cc)
target_link_libraries(cb_comm_tests iotmock iota_a ${IOTAGENT_LIBS}
	${TEST_LIBS})

add_executable(ul20_tests ul20Test.cc ul20_test.cc)
target_link_libraries(ul20_tests sul20 iotmock iota_a ${IOTAGENT_LIBS} Variant
	${TEST_LIBS})
add_executable(admin_tests adminTest.cc admin_tests.cc)
target_link_libraries(admin_tests iota_a ${IOTAGENT_LIBS} Variant ${TEST_LIBS})

add_executable(adminMgM_tests adminMgmTest.cc adminMgm_tests.cc)
target_link_libraries(adminMgM_tests iota_a ${IOTAGENT_LIBS} Variant ${TEST_LIBS})

if(MOSQUITTO_FOUND)
include_directories(${MOSQUITTO_INCLUDE_DIR})
#set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${MOSQUITTO_LIBRARIES})
link_directories(${MOSQUITTO_LIBS_DIR})

add_executable(mqtt_tests mqttTest.cc mqtt_test.cc)
target_link_libraries(mqtt_tests esp_mqtt mosquittopp mosquitto smqttservice iotmock iota_a
	${IOTAGENT_LIBS} ${MOSQUITTO_LIBRARIES} Variant ${TEST_LIBS})

add_executable(tt_tests ttTest.cc tt_test.cc TTBufferTests.cc)
target_link_libraries(tt_tests esp sttservice iotmock iota_a
	${IOTAGENT_LIBS} Variant ${TEST_LIBS})
endif()


add_executable(url_tests urlTest.cc url_test.cc)
target_link_libraries(url_tests iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})

add_executable(cache_tests cacheTests.cc cache_test.cc)
target_link_libraries(cache_tests iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})
add_executable(csv_tests CsvTest.cc csv_test.cc)
target_link_libraries(csv_tests iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})


add_executable(oauth_tests oauthTest.cc oauth_test.cc)
target_link_libraries(oauth_tests iotmock iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})
add_executable(sp_tests simpleplugingTest.cc simplepluging_test.cc sp_service.cc)
target_link_libraries(sp_tests iotmock iota_a ${IOTAGENT_LIBS} Variant ${TEST_LIBS})
add_executable(mongo_tests mongoTest.cc mongo_test.cc)
target_link_libraries(mongo_tests iotmock iota_a ${IOTAGENT_LIBS} ${TEST_LIBS})

add_executable(sample_tests sampleTest.cc sample_test.cc)
target_link_libraries(sample_tests stestsrv iotmock iota_a ${IOTAGENT_LIBS} Variant
        ${TEST_LIBS})

