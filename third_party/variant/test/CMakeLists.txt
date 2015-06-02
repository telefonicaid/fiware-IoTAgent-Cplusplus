
include_directories ("../src")

add_executable(rand_test_json rand_test_json.cc)
target_link_libraries(rand_test_json Variant)
add_test(rand_test_json ${CMAKE_CURRENT_BINARY_DIR}/rand_test_json)

if(LIBVARIANT_ENABLE_YAML)
	add_executable(rand_test_yaml rand_test_yaml.cc)
	target_link_libraries(rand_test_yaml Variant)
	add_test(rand_test_yaml ${CMAKE_CURRENT_BINARY_DIR}/rand_test_yaml)

	add_executable(test_yaml_parser test_yaml_parser.cc)
	target_link_libraries(test_yaml_parser Variant)
	add_test(test_yaml_parser ${CMAKE_CURRENT_BINARY_DIR}/test_yaml_parser)

	add_executable(test_loadall test_loadall.cc)
	target_link_libraries(test_loadall Variant)
	add_test(test_loadall ${CMAKE_CURRENT_BINARY_DIR}/test_loadall)
endif()

if(LIBVARIANT_ENABLE_XML)
	add_executable(rand_test_xml rand_test_xml.cc)
	target_link_libraries(rand_test_xml Variant)
	add_test(rand_test_xml ${CMAKE_CURRENT_BINARY_DIR}/rand_test_xml)

	add_executable(test_xml_parser test_xml_parser.cc)
	target_link_libraries(test_xml_parser Variant)
	add_test(test_xml_parser ${CMAKE_CURRENT_BINARY_DIR}/test_xml_parser)
endif()

add_executable(test_variant test_variant.cc)
target_link_libraries(test_variant Variant)
add_test(test_variant ${CMAKE_CURRENT_BINARY_DIR}/test_variant)

add_executable(test_empty test_empty.cc)
target_link_libraries(test_empty Variant)
add_test(test_empty ${CMAKE_CURRENT_BINARY_DIR}/test_empty)

add_executable(test_base64 test_base64.cc)
target_link_libraries(test_base64 Variant)
add_test(test_base64 ${CMAKE_CURRENT_BINARY_DIR}/test_base64)

add_executable(test_memoryoutput test_memoryoutput.cc)
target_link_libraries(test_memoryoutput Variant)
add_test(test_memoryoutput ${CMAKE_CURRENT_BINARY_DIR}/test_memoryoutput)

add_executable(test_bundlehdr test_bundlehdr.cc)
target_link_libraries(test_bundlehdr Variant)
add_test(test_bundlehdr ${CMAKE_CURRENT_BINARY_DIR}/test_bundlehdr)

add_executable(test_bundleparser test_bundleparser.cc)
target_link_libraries(test_bundleparser Variant)
add_test(test_bundleparser ${CMAKE_CURRENT_BINARY_DIR}/test_bundleparser)

add_executable(test_bundleemitter test_bundleemitter.cc)
target_link_libraries(test_bundleemitter Variant)
add_test(test_bundleemitter ${CMAKE_CURRENT_BINARY_DIR}/test_bundleemitter)

add_executable(test_guess test_guess.cc)
target_link_libraries(test_guess Variant)
add_test(test_guess ${CMAKE_CURRENT_BINARY_DIR}/test_guess)

add_executable(test_numerics test_numerics.cc)
target_link_libraries(test_numerics Variant)
add_test(test_numerics ${CMAKE_CURRENT_BINARY_DIR}/test_numerics)

add_definitions("-DTEST_SCHEMA_DATA=\"${CMAKE_CURRENT_SOURCE_DIR}/test_schema_data/\"")
add_executable(test_schema test_schema.cc)
target_link_libraries(test_schema Variant)
add_test(test_schema ${CMAKE_CURRENT_BINARY_DIR}/test_schema)

add_executable(test_path test_path.cc)
target_link_libraries(test_path Variant)
add_test(test_path ${CMAKE_CURRENT_BINARY_DIR}/test_path)

add_executable(test_extensions test_extensions.cc)
target_link_libraries(test_extensions Variant)
add_test(test_extensions ${CMAKE_CURRENT_BINARY_DIR}/test_extensions)

add_executable(test_argparse test_argparse.cc)
target_link_libraries(test_argparse Variant)
configure_file(test_argparse.sh.in
	"${CMAKE_CURRENT_BINARY_DIR}/test_argparse.sh" @ONLY)
add_test(test_argparse sh ${CMAKE_CURRENT_BINARY_DIR}/test_argparse.sh)


add_executable(test_payload test_payload.cc)
target_link_libraries(test_payload Variant)
add_test(test_payload ${CMAKE_CURRENT_BINARY_DIR}/test_payload)

add_executable(test_roundtrip test_roundtrip.cc)
target_link_libraries(test_roundtrip Variant)
add_test(test_roundtrip ${CMAKE_CURRENT_BINARY_DIR}/test_roundtrip)

add_executable(test_argparse_merge test_argparse_merge.cc)
target_link_libraries(test_argparse_merge Variant)
add_test(test_argparse_merge ${CMAKE_CURRENT_BINARY_DIR}/test_argparse_merge)

add_executable(test_json_parser test_json_parser.cc)
target_link_libraries(test_json_parser Variant)
add_test(test_json_parser ${CMAKE_CURRENT_BINARY_DIR}/test_json_parser)

if(LIBVARIANT_ENABLE_MSGPACK)

	add_executable(prof_msgpack prof_msgpack.cc)
	target_link_libraries(prof_msgpack Variant)
	add_test(prof_msgpack ${CMAKE_CURRENT_BINARY_DIR}/prof_msgpack)

	add_executable(rand_test_msgpack rand_test_msgpack.cc)
	target_link_libraries(rand_test_msgpack Variant)
	add_test(rand_test_msgpack ${CMAKE_CURRENT_BINARY_DIR}/rand_test_msgpack)

endif()
