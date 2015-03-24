file(GLOB SRC_FILES *.cpp)
include_directories(../esp/source/h ../esp/tinyxml ../esp/plugins/*)
add_library(TTService MODULE ${SRC_FILES})
add_library(sttservice STATIC ${SRC_FILES})
add_library(ttservice SHARED ${SRC_FILES})
set_target_properties(ttservice PROPERTIES OUTPUT_NAME TTService)
set_target_properties(sttservice PROPERTIES OUTPUT_NAME TTService)


#TEST adding sesp as static to all MQTTService objects
target_link_libraries(TTService esp iota ${IOTAGENT_LIBS})
target_link_libraries(ttservice esp)
target_link_libraries(sttservice esp )



install(TARGETS TTService ttservice sttservice DESTINATION lib/${CMAKE_BUILD_TYPE})
