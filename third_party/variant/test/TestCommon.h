/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef TESTCOMMON_H
#define TESTCOMMON_H
#pragma once
#include "EventBuffer.h"
#include <Variant/Variant.h>
#include <Variant/Parser.h>
#include <Variant/Emitter.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

namespace libvariant {

inline bool ErrorCheckEventBuffer(shared_ptr<EventBuffer> expected, shared_ptr<EventBuffer> result) {
	if (!result->Equal(*expected)) {
		if (result->NumEvents() != expected->NumEvents()) {
			std::cout << "Error size of action are different, got " << result->NumEvents() << " expected " << expected->NumEvents() << "\n";
		}
		for (unsigned i = 0; i < std::max(expected->NumEvents(), result->NumEvents()); ++i) {
			if (i < expected->NumEvents() && i < result->NumEvents()) {
				if (!(expected->GetEvent(i) == result->GetEvent(i))) {
					std::cout << "Action number " << i << " not expected action\nExpected: " << expected->GetEvent(i)
						<< "\nGot: " << result->GetEvent(i) << "\n";
				}
			} else if (i < expected->NumEvents()) {
				std::cout << "Action in original not in result: " << expected->GetEvent(i) << "\n";
			} else if (i < result->NumEvents()) {
				std::cout << "Action in result not in original: " << result->GetEvent(i) << "\n";
			}
		}
		return false;
	}
	return true;
}

inline int DoRandomTest(int argc, char **argv,
libvariant::Emitter (*Emitter_t)(libvariant::shared_ptr<libvariant::EmitterOutput>, libvariant::Variant),
libvariant::Parser (*Parser_t)(libvariant::shared_ptr<libvariant::ParserInput>), bool allow_nan) {
	int verbose = 0;
	int num_tests = 100;
	while (true) {
		int c = getopt(argc, argv, "n:v:s:h");
		if (c == -1) break;
		switch (c) {
		case 'n':
			num_tests = atoi(optarg);
			break;
		case 'v':
			verbose = atoi(optarg);
			break;
		case 's':
			srand(strtoll(optarg, 0, 0));
			break;
		default:
			std::cout << *argv << " -s seed -v verbosity -n num_tests\n";
			return 0;
		}
	}
	for (int test = 0; test < num_tests; ++test) {
		if (verbose > 0) { std::cout << "Performing test " << test << "\n"; }
		shared_ptr<EventBuffer> expected(new EventBuffer);
		expected->FillRandom(EventBuffer::NO_EVENT, 10, allow_nan);
		if (verbose > 0) { std::cout << "Generated " << expected->NumEvents() << " events\n"; }
		std::ostringstream oss;
		libvariant::Variant params;
		libvariant::Emitter emitter = Emitter_t(libvariant::CreateEmitterOutput(oss.rdbuf()), params);
		shared_ptr<EventBuffer> event_copy(new EventBuffer(*expected));
		event_copy->Playback(emitter);
		emitter.Close();
		std::string val = oss.str();
		if (verbose > 1) {
			std::cout << val;
			std::cout.flush();
		}
		std::istringstream iss(val);
		libvariant::Parser parser = Parser_t(libvariant::CreateParserInputFile(iss.rdbuf()));
		shared_ptr<EventBuffer> result(new EventBuffer);
		result->Fill(parser);

		bool err = !ErrorCheckEventBuffer(expected, result);
		if (err) return 1;
		if (verbose > 0) { printf("End test %d\n\n", test); }
	}

	return 0;
}

inline Variant GenerateRandomVariant(bool allow_nan) {
	shared_ptr<EventBuffer> events(new EventBuffer);
	events->FillRandom(EventBuffer::NO_EVENT, 10, allow_nan);
	Parser p(events);
	return ParseVariant(p);
}

}

#endif
