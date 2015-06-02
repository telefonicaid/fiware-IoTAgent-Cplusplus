/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Parser.h>
#include <limits>
#include "EventBuffer.h"
#include "TestCommon.h"
#include <iostream>

using namespace libvariant;

static const char *input_document = "---\n"
"string: value\n"
"int10: 1234\n"
"int8: 0o234\n"
"int16: 0x2F\n"
"float: [ 0.1, .12, 1.0e-2 ]\n"
"nan: .nan\n"
"inf: .inf\n"
"strnum: '12345'\n"
"...\n";

void ExpectedEvents(Emitter e) {
	e.BeginDocument();
	e.BeginMap();
	e.Emit("string");
	e.Emit("value");
	e.Emit("int10");
	e.Emit(intmax_t(1234ll));
	e.Emit("int8");
	e.Emit(uintmax_t(0234ll));
	e.Emit("int16");
	e.Emit(uintmax_t(0x2Fll));
	e.Emit("float");
	e.BeginList();
	e.Emit(0.1);
	e.Emit(.12);
	e.Emit(1.0e-2);
	e.EndList();
	e.Emit("nan"),
	e.Emit(std::numeric_limits<double>::quiet_NaN()),
	e.Emit("inf"),
	e.Emit(std::numeric_limits<double>::infinity()),
	e.Emit("strnum"),
	e.Emit("12345"),
	e.EndMap();
	e.EndDocument();
}


int main(int argc, char **argv) {
	Parser parser = YAMLParser(CreateParserInput(input_document));
	shared_ptr<EventBuffer> expected(new EventBuffer);
	ExpectedEvents(Emitter(expected));
	shared_ptr<EventBuffer> result(new EventBuffer);

	result->Fill(parser);

	if (!ErrorCheckEventBuffer(expected, result)) {
		return 1;
	}
	return 0;
}
