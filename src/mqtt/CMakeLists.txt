file(GLOB SRC_FILES *.cc)
if(MOSQUITTO_FOUND)
include_directories(${MOSQUITTO_INCLUDE_DIR})
set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${MOSQUITTO_LIBRARIES})
link_directories(${MOSQUITTO_LIBS_DIR})
endif()
include_directories(../esp/source/h ../esp/tinyxml ../esp/plugins/*)
add_library(MqttService MODULE ${SRC_FILES})
add_library(mqttservice STATIC ${SRC_FILES})
add_library(smqttservice SHARED ${SRC_FILES})
set_target_properties(mqttservice PROPERTIES OUTPUT_NAME MqttService)
set_target_properties(smqttservice PROPERTIES OUTPUT_NAME MqttService)


#TEST adding sesp as static to all MQTTService objects
target_link_libraries(MqttService esp_mqtt mosquittopp mosquitto iota ${IOTAGENT_LIBS})
target_link_libraries(mqttservice esp_mqtt mosquittopp mosquitto)
target_link_libraries(smqttservice esp_mqtt mosquittopp mosquitto)



install(TARGETS MqttService mqttservice smqttservice DESTINATION lib/${CMAKE_BUILD_TYPE})
