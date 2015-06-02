/** \file
 * \author John Bridgman
 * \brief 
 */
#include "TestAssert.h"
#include <Variant/Variant.h>
#include <Variant/Payload.h>
#include <string.h>
#include <vector>
#include <iostream>

using namespace libvariant;
using namespace std;

const char bundle_str[] =
"bundle.version: 0.0\n"
"id: 1\n"
"payload.length: 40\n"
"\0"
"1234567890"
"1234567890"
"1234567890"
"1234567890"
"bundle.version: 0.0\n"
"id: 2 # comments\n"
"payload.length: 40\n"
"#comments\n"
"\0"
"\000\00\00\00"
"\001\00\00\00"
"\002\00\00\00"
"\003\00\00\00"
"\004\00\00\00"
"\005\00\00\00"
"\006\00\00\00"
"\007\00\00\00"
"\010\00\00\00"
"\011\00\00\00"
"bundle.version: 0.0\n"
"id: 3\n"
"payload.length: 70\n"
"erroniouskey\n"
"\n\n\n\n\n"
"\0"
"1234567890"
"blakdsflks"
"sdkjhasdfs"
"347iuhafgj"
"347aguhfdh"
"iouq5478q7"
"8oq34guily"
;

std::ostream &operator<<(std::ostream &os, Variant v) {
#ifdef ENABLE_YAML
	SerializeYAML(os.rdbuf(), v);
#endif
	return os;
}

int main(int argc, char **argv) {
	// Need the -1 there because of the \0 at the end of the string (it looks like an empty bundle at the end...)
	shared_ptr<ParserInput> input = CreateParserInput(&bundle_str[0], sizeof(bundle_str) - 1);

	Variant v = DeserializeWithPayload(input, SERIALIZE_BUNDLEHDR);
	cout << v;
	cout << endl << "Payload size: " << v["payload.data"].Size() << endl;
	ASSERT(v["id"].AsUnsigned() == 1);
	ASSERT(v["payload.data"].Size() == 40);
	ASSERT(memcmp("1234567890123456789012345678901234567890", v["payload.data"].AsBlob()->GetPtr(0), 40) == 0);

	v = DeserializeWithPayload(input, SERIALIZE_BUNDLEHDR);
	cout << v << endl;
	ASSERT(v["id"].AsUnsigned() == 2);
	ASSERT(v["payload.data"].Size() == 40);
	std::vector<int> buf;
	for (int i = 0; i < 10; ++i) { buf.push_back(i); }
	ASSERT(memcmp(&buf[0], v["payload.data"].AsBlob()->GetPtr(0), 40) == 0);

	v = DeserializeWithPayload(input, SERIALIZE_BUNDLEHDR);
	cout << v << endl;
	ASSERT(v["payload.data"].Size() == 70);
	string orig = "1234567890" "blakdsflks" "sdkjhasdfs" "347iuhafgj" "347aguhfdh" "iouq5478q7" "8oq34guily";
	std::string p((char *)v["payload.data"].AsBlob()->GetPtr(0), 70);
	ASSERT(orig == p);

	// Test that the unsafe zero copy does what it is supposed to do
	v = DeserializeBundle(&bundle_str[0], sizeof(bundle_str) - 1);
	ASSERT(v["payload.data"].AsBlob()->GetPtr(0) != &bundle_str[strlen(&bundle_str[0])+1]);
	v = DeserializeBundle(&bundle_str[0], sizeof(bundle_str) - 1, false);
	ASSERT(v["payload.data"].AsBlob()->GetPtr(0) == &bundle_str[strlen(&bundle_str[0])+1]);
	return 0;
}
