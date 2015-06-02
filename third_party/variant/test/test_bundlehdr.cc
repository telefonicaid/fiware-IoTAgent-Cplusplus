/** \file
 * \author John Bridgman
 * \brief 
 *
 */

#include <Variant/Variant.h>
#include <Variant/Emitter.h>
#include <Variant/Parser.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include <string.h>

using namespace libvariant;

const char example_bundle[] = 
"bundle.version: 0.0\n"
"keyone: value\n"
"listkey: value1|value2|value3\n"
"doodle: 1234 # comments!\n"
"foo: 0.234245\n"
"singlelist: blah|\n"
"emptylist: |\n"
"\n\n\n\n\0";

const char expected[] = "{ \"keyone\": \"value\","
"\"listkey\": [ \"value1\", \"value2\", \"value3\" ], \"doodle\": 1234, \"foo\": 0.234245,"
"\"singlelist\": [ \"blah\" ], \"emptylist\": [] }";

int main(int argc, char **argv) {
	std::istringstream iss(std::string(&example_bundle[0], strlen(&example_bundle[0])+1));
	Parser parser = BundleHdrParser(CreateParserInputFile(iss.rdbuf()));

	Variant v = ParseVariant(parser);
	Variant ex = DeserializeJSON(expected);
	if (v != ex) {
		puts("");
		SerializeJSON(stdout, v);
		puts("");
		SerializeJSON(stdout, ex);
		puts("");
		return 1;
	}
	Emitter emitter = BundleHdrEmitter(CreateEmitterOutput(stdout));
	emitter << v;
	return 0;
}
