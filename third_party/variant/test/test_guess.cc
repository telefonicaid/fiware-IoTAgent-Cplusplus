/** \file
 * \author John Bridgman
 * \brief 
 */

#include "TestAssert.h"
#include <Variant/Variant.h>
#include <string>
#include <stdio.h>

using namespace libvariant;
using namespace std;

int main(int argc, char **argv) {
	Variant v = Variant::MapType;
	v["key1"] = 1;
	v["blah"] = "murmur";
	v["foo"] = 1.2356;
	v["bar"] = 49834;
	const char *l[] = { "lisp", "erlang", "haskel", "python", "lua", "c", "c++", "java", "c#", "tcl", 0};
	for (const char **c = &l[0]; *c; c += 1) { v["l"].Append(*c); }
	Variant v2;
#ifdef ENABLE_YAML
	printf("As yaml\n");
	string yaml = SerializeYAML(v);
	printf("%s\n", yaml.c_str());
	v2 = DeserializeGuess(yaml);
	ASSERT(v2 == v);

	string empty = "  #blah\nnull  ";
	v2 = DeserializeGuess(empty);
	ASSERT(v2 == Variant::NullType);
#endif
#ifdef ENABLE_XML
	printf("As xml plist\n");
	string xmlplist = SerializeXMLPLIST(v);
	printf("%s\n", xmlplist.c_str());
	v2 = DeserializeGuess(xmlplist);
	ASSERT(v2 == v);
#endif
	printf("As JSON\n");
	string json = SerializeJSON(v);
	printf("%s\n", json.c_str());
	v2 = DeserializeGuess(json);
	ASSERT(v2 == v);

	return 0;
}
