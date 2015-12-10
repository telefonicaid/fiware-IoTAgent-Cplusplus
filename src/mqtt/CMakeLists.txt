file(GLOB SRC_FILES *.cc)
if(MOSQUITTO_FOUND)
include_directories(${MOSQUITTO_INCLUDE_DIR})
set(IOTAGENT_LIBS ${IOTAGENT_LIBS} ${MOSQUITTO_LIBRARIES})
link_directories(${MOSQUITTO_LIBS_DIR})
endif()
include_directories(../esp/source/h ../esp/tinyxml ../esp/plugins/*)
add_library(ModuleMqttService MODULE ${SRC_FILES})
add_library(StaticMqttService STATIC ${SRC_FILES})
add_library(SharedMqttService SHARED ${SRC_FILES})
set_target_properties(ModuleMqttService PROPERTIES OUTPUT_NAME MqttService)
set_target_properties(StaticMqttService PROPERTIES OUTPUT_NAME MqttService)
set_target_properties(SharedMqttService PROPERTIES OUTPUT_NAME MqttService)


#TEST adding sesp as static to all MQTTService objects
target_link_libraries(ModuleMqttService esp_mqtt mosquittopp mosquitto SharedLibraryIota ${IOTAGENT_LIBS})
target_link_libraries(SharedMqttService esp_mqtt mosquittopp mosquitto)
target_link_libraries(StaticMqttService esp_mqtt mosquittopp mosquitto)



install(TARGETS ModuleMqttService SharedMqttService StaticMqttService DESTINATION lib/${CMAKE_BUILD_TYPE})
