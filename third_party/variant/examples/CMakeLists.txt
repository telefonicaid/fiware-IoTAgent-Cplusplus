
add_executable(convert convert.cc)
target_link_libraries(convert Variant ${EXTRA_LIBS})

add_executable(example1 example1.cc)
target_link_libraries(example1 Variant ${EXTRA_LIBS})

add_executable(example2 example2.cc)
target_link_libraries(example2 Variant ${EXTRA_LIBS})

add_executable(map_example map_example.cc)
target_link_libraries(map_example Variant ${EXTRA_LIBS})

add_executable(list_example list_example.cc)
target_link_libraries(list_example Variant ${EXTRA_LIBS})

add_executable(pretty_print pretty_print.cc)
target_link_libraries(pretty_print Variant ${EXTRA_LIBS})

add_executable(arg_parse arg_parse.cc)
target_link_libraries(arg_parse Variant ${EXTRA_LIBS})
