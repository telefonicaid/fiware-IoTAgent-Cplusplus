file(GLOB SRC_FILES *.cc)
add_library(iotmock ${SRC_FILES})
target_link_libraries(iotmock
	                    ${IOTAGENT_LIBS})
