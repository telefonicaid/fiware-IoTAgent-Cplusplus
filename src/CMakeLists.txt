include_directories(rest ngsi services util)
add_subdirectory(util)
add_subdirectory(ngsi)
add_subdirectory(rest)
add_subdirectory(services)
if(MOSQUITTO_FOUND)
add_subdirectory(esp)
add_subdirectory(mqtt)
endif()
add_subdirectory(thinkingthings)
add_subdirectory(ultra_light)
add_subdirectory(sample_test)

add_library(iota_a STATIC $<TARGET_OBJECTS:util_objs>
                        $<TARGET_OBJECTS:ngsi_objs>
                        $<TARGET_OBJECTS:rest_objs>
                        $<TARGET_OBJECTS:admin_objs>
                        )
add_library(iota SHARED $<TARGET_OBJECTS:util_objs>
                        $<TARGET_OBJECTS:ngsi_objs>
                        $<TARGET_OBJECTS:rest_objs>
                        $<TARGET_OBJECTS:admin_objs>
                        )
set_target_properties(iota_a PROPERTIES OUTPUT_NAME iota)
add_executable(iotagent app/main.cc)

target_link_libraries(iotagent
                      iota
                      Variant
                      ${IOTAGENT_LIBS})

install(TARGETS iotagent DESTINATION bin/${CMAKE_BUILD_TYPE})
install(TARGETS iota iota_a DESTINATION lib/${CMAKE_BUILD_TYPE})
