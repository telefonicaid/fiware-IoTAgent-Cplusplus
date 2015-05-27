file(GLOB SRC_FILES *.cc cache.h command_cache.h timer.h device.h service.h CommandResponse.h json_util.h common.h TTCBPublisher.h)
add_library(util_objs OBJECT ${SRC_FILES})
