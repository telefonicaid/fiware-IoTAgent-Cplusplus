include_directories(    ./source/h
			./tinyxml
			./logger/include
			./plugins
			./)
if (FOUND_MOSQUITTO)
file(GLOB SRC_ESP_MQTT ./logger/*.cpp
		./plugins/input_buffer/*.cpp
		./plugins/input_mqtt/*.cpp
		./plugins/output_file/*.cpp
		./plugins/output_iot/*.cpp
		./plugins/parser_basic/*.cpp
		./plugins/postprocessor_sensorml/*.cpp
		./plugins/postprocessor_json/*.cpp
		./plugins/postprocessor_tt/*.cpp
		./plugins/postprocessor_text/*.cpp
		./plugins/postprocessor_ul20/*.cpp
		./source/*.cpp
		./tinyxml/*.cpp)
endif()
file(GLOB SRC_ESP ./logger/*.cpp
		./plugins/input_buffer/*.cpp
		./plugins/output_file/*.cpp
		./plugins/parser_basic/*.cpp
		./plugins/postprocessor_sensorml/*.cpp
		./plugins/postprocessor_json/*.cpp
		./plugins/postprocessor_tt/*.cpp
		./plugins/postprocessor_text/*.cpp
		./plugins/postprocessor_ul20/*.cpp
		./source/*.cpp
		./tinyxml/*.cpp)

#add_definitions (-DUSE_IOT -DUSE_JSON -DUSE_TTOPEN)
file(GLOB to_remove ./source/main.cpp)
if (FOUND_MOSQUITTO)
list(REMOVE_ITEM SRC_ESP_MQTT ${to_remove})
endif()
list(REMOVE_ITEM SRC_ESP ${to_remove})
#add_library(esp SHARED ${SRC_ESP})
#add_library(esp STATIC ${SRC_ESP})



add_library(esp STATIC ${SRC_ESP})
if (FOUND_MOSQUITTO)
add_library(esp_mqtt STATIC ${SRC_ESP_MQTT})
endif()
target_compile_definitions(esp PUBLIC USE_TTOPEN USE_JSON)
if (FOUND_MOSQUITTO)
target_compile_definitions(esp_mqtt PUBLIC USE_MQTT USE_IOT USE_JSON)
endif()
set_target_properties(esp PROPERTIES OUTPUT_NAME Esp)
if (FOUND_MOSQUITTO)
set_target_properties(esp_mqtt PROPERTIES OUTPUT_NAME EspMqtt)
endif()

install(TARGETS esp DESTINATION lib/${CMAKE_BUILD_TYPE})
if (FOUND_MOSQUITTO)
install(TARGETS esp_mqtt DESTINATION lib/${CMAKE_BUILD_TYPE})
endif()

#DOUBT: not sure if we need to specify where sesp is going to be installed, using install for instance?
