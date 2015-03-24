include_directories(    ./source/h
			./tinyxml
			./logger/include
			./plugins
			./)

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
list(REMOVE_ITEM SRC_ESP_MQTT ${to_remove})
list(REMOVE_ITEM SRC_ESP ${to_remove})
#add_library(esp SHARED ${SRC_ESP})
#add_library(esp STATIC ${SRC_ESP})



add_library(esp STATIC ${SRC_ESP})
add_library(esp_mqtt STATIC ${SRC_ESP_MQTT})
target_compile_definitions(esp PUBLIC USE_TTOPEN USE_JSON)
target_compile_definitions(esp_mqtt PUBLIC USE_MQTT USE_IOT USE_JSON)
set_target_properties(esp PROPERTIES OUTPUT_NAME Esp)
set_target_properties(esp_mqtt PROPERTIES OUTPUT_NAME EspMqtt)

install(TARGETS esp esp_mqtt DESTINATION lib/${CMAKE_BUILD_TYPE})

#DOUBT: not sure if we need to specify where sesp is going to be installed, using install for instance?
