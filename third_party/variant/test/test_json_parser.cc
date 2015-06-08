/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Parser.h>
#include <limits>
#include "TestCommon.h"

using namespace libvariant;

static const char *input_document = "\n\n\n\n{\n"
"\"string\": \"value\",\n"
"\"int10\": 1234,\n"
"\"truth\": true,\n"
"\"float\": [ 0.1, 0.12, 1.0e-2 ],\n"
"\"strnum\": \"12345\"\n,"
"\"maxuint64\": 18446744073709551615\n"
"}\n";

void ExpectedEvents(Emitter e) {
	e.BeginDocument();
	e.BeginMap();
	e.Emit("string"),
	e.Emit("value"),
	e.Emit("int10"),
	e.Emit(intmax_t(1234ll)),
	e.Emit("truth"),
	e.Emit(true),
	e.Emit("float"),
	e.BeginList();
	e.Emit(0.1),
	e.Emit(.12),
	e.Emit(1.0e-2),
	e.EndList();
	e.Emit("strnum"),
	e.Emit("12345"),
	e.Emit("maxuint64"),
	e.Emit((uintmax_t)std::numeric_limits<uint64_t>::max()),
	e.EndMap();
	e.EndDocument();
}

int main(int argc, char **argv) {
	Parser parser = JSONParser(CreateParserInput(input_document));
	shared_ptr<EventBuffer> expected(new EventBuffer);
	ExpectedEvents(Emitter(expected));
	shared_ptr<EventBuffer> result(new EventBuffer);

	result->Fill(parser);

	if (!ErrorCheckEventBuffer(expected, result)) {
		return 1;
	}
	return 0;
}
