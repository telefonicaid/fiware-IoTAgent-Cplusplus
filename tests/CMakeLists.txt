set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_subdirectory(mocks)
include_directories(${CPPUNIT_INCLUDE_DIR}
		${GMOCK_INCLUDE_DIR}
		${MOSQUITTO_DIRECTORY}/include
		    ${MOSQUITTO_DIRECTORY}/include/cpp
		${PROJECT_SOURCE_DIR}/src/esp/source/h
		${PROJECT_SOURCE_DIR}/src/esp/tinyxml
		${PROJECT_SOURCE_DIR}/src/esp/lua
		${PROJECT_SOURCE_DIR}/src/esp/logger/include
		${PROJECT_SOURCE_DIR}/src/esp/plugins
		${PROJECT_SOURCE_DIR}/src/esp/esp/ngsi)

add_subdirectory(iotagent)
#if(MOSQUITTO_FOUND)
#add_subdirectory(esp)
#endif()

