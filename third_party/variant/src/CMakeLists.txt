
set(sources
   	Variant.cc
	VariantEmitting.cc
	VariantParsing.cc
	Exceptions.cc
	Extensions.cc
	Blob.cc
	Base64.cc
	ParseBool.cc
	Path.cc
	Emitter.cc
	EmitterOutput.cc
   	JSONEmitter.cc
	Parser.cc
	ParserInput.cc
	JSONParser.cc
	../lib/json/JSON_parser.c
	GuessScalar.cc
	BundleHdrParser.cc
	BundleHdrEmitter.cc
	GuessFormat.cc
	Payload.cc
	ArgParse.cc
	argparse-schema.cc
	Schema.cc
	SchemaLoader.cc
	json-schema-v4.cc
	StackTrace.cc
	EventBuffer.cc
	)

if(LIBVARIANT_ENABLE_XML)
	list(APPEND sources XMLPLISTEmitter.cc XMLPLISTParser.cc)
endif()

if(LIBVARIANT_ENABLE_YAML)
	list(APPEND sources YAMLEmitter.cc YAMLParser.cc)
endif()

if(LIBVARIANT_ENABLE_MSGPACK)
	list(APPEND sources MsgPackEmitter.cc MsgPackParser.cc)
endif()

include_directories("../lib/json/")

add_library(Variant OBJECT ${sources})
set_target_properties(Variant PROPERTIES VERSION ${LIBVARIANT_VERSION})
#target_link_libraries(Variant ${EXTRA_LIBS})

#add_executable(varsh varsh.cc)
#target_link_libraries(varsh Variant)

#install(TARGETS Variant varsh
#	EXPORT LibVariantLibraryDepends
#	RUNTIME DESTINATION ${INSTALL_BIN_DIR}
#   	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
#   	ARCHIVE DESTINATION ${INSTALL_LIB_DIR})


