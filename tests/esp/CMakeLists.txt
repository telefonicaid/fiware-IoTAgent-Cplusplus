file(GLOB ESP_TEST_FILES *.cpp)
file(GLOB ESP_TEST_INCLUDES h/*.h)
include_directories(  	h
		    	${MOSQUITTO_DIRECTORY}/include
		        ${MOSQUITTO_DIRECTORY}/include/cpp
		   	${PROJECT_SOURCE_DIR}/src/esp/source/h
			${PROJECT_SOURCE_DIR}/src/esp/tinyxml
			${PROJECT_SOURCE_DIR}/src/esp/lua
			${PROJECT_SOURCE_DIR}/src/esp/logger/include
			${PROJECT_SOURCE_DIR}/src/esp/plugins
			${PROJECT_SOURCE_DIR}/src/esp/esp/ngsi)

add_executable(esp_tests ${ESP_TEST_FILES} ${ESP_TEST_INCLUDES} )
target_link_libraries(esp_tests esp iotmock iota_a ${IOTAGENT_LIBS}
			${MOSQUITTO_LIBRARIES} ${TEST_LIBS})
