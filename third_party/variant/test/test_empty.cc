/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Variant.h>
#include <Variant/Emitter.h>
#include <iostream>
using namespace libvariant;
using namespace std;

int main(int argc, char **argv) {
#ifdef ENABLE_YAML
	Emitter emitter = YAMLEmitter(CreateEmitterOutput(std::cout.rdbuf()));
#else
	Emitter emitter = JSONEmitter(CreateEmitterOutput(std::cout.rdbuf()), Variant().Set("pretty", true));
#endif
	emitter.BeginList();
	try {
		Variant v = DeserializeJSON("");
		emitter << v;
		return 1;
	} catch (const std::exception &e) {
		emitter << "JSON passed";
	}

	try {
		Variant v = DeserializeJSON("  ");
		emitter << v;
		return 1;
	} catch (const std::exception &e) {
		emitter << "JSON passed";
	}
	emitter.Flush();

#ifdef ENABLE_YAML
	{
		Variant v = DeserializeYAML("");
		emitter << v;
		emitter.Flush();
		emitter << "YAML passed";
	}
#endif

#ifdef ENABLE_XML
	try {
		Variant v = DeserializeXMLPLIST("");
		emitter << v;
		return 1;
	} catch (const std::exception &e) {
		emitter << "XML passed";
	}
#endif
	emitter.EndList();
	return 0;
}
