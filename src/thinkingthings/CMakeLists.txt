file(GLOB SRC_FILES *.cpp)
include_directories(../esp/source/h ../esp/tinyxml ../esp/plugins/*)
add_library(ModuleTTService MODULE ${SRC_FILES})
add_library(StaticTTService STATIC ${SRC_FILES})
add_library(SharedTTService SHARED ${SRC_FILES})
set_target_properties(ModuleTTService PROPERTIES OUTPUT_NAME TTService)
set_target_properties(StaticTTService PROPERTIES OUTPUT_NAME TTService)
set_target_properties(SharedTTService PROPERTIES OUTPUT_NAME TTService)


#TEST adding sesp as static to all MQTTService objects
target_link_libraries(ModuleTTService esp StaticLibraryIota ${IOTAGENT_LIBS})
target_link_libraries(StaticTTService esp)
target_link_libraries(SharedTTService esp )



install(TARGETS ModuleTTService StaticTTService SharedTTService DESTINATION lib/${CMAKE_BUILD_TYPE})
