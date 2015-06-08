/** \file
 * \author John Bridgman
 * \brief 
 */

#include "TestAssert.h"
#include <Variant/Payload.h>
#include <Variant/ParserInput.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

using namespace libvariant;
using namespace std;

#define TESTPAYLOAD \
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"\
"1234567890"

#define TESTPAYLOAD_LEN 100
#define TESTPAYLOAD_LEN_STR "100"

const char test_input[] = 
"{\"payload\": { \"length\": " TESTPAYLOAD_LEN_STR " }, \"abc\": 123 }"
"\n\n\n\n\n"
"\0"
TESTPAYLOAD

"{\"payload\": { \"length\": " TESTPAYLOAD_LEN_STR ", \"data\": null }, \"abc\": 123 }"
"\n\n\n\n\n"
"\n\n\n\n\n"
"\0"
TESTPAYLOAD
"{ \"abc\": 123 }"
"\0"
"{ \"payload\": { \"length\": " TESTPAYLOAD_LEN_STR "} }"
"\n\n\n\n\n"
"\n\n\n\n\n"
"\n\n\n\n\n"
"\0"
TESTPAYLOAD
"{ \"payload\": { \"length\": 0 } }"
;

void TestParse() {
	shared_ptr<ParserInput> input(CreateParserInput(&test_input[0], sizeof(test_input) - 1));
	const char *datapath = "/payload/data";
	const char *lenpath = "/payload/length";
	Variant params = Variant().Set("length_path", lenpath).Set("data_path", datapath);

	Variant v = DeserializeWithPayload(input, SERIALIZE_JSON, params);
	ASSERT(v.GetPath(lenpath) == 100);
	ASSERT(v.GetPath("abc") == 123);
	BlobPtr b = v.GetPath(datapath).AsBlob();
	ASSERT(b->GetNumBuffers() == 1);
	ASSERT(memcmp(b->GetPtr(0), TESTPAYLOAD, TESTPAYLOAD_LEN) == 0);

	v = DeserializeWithPayload(input, SERIALIZE_JSON, params);
	ASSERT(v.GetPath(lenpath) == 100);
	ASSERT(v.GetPath("abc") == 123);
	b = v.GetPath(datapath).AsBlob();
	ASSERT(b->GetNumBuffers() == 1);
	ASSERT(memcmp(b->GetPtr(0), TESTPAYLOAD, TESTPAYLOAD_LEN) == 0);

	v = DeserializeWithPayload(input, SERIALIZE_JSON, params);
	ASSERT(v.HasPath(datapath) == false);
	ASSERT(v.GetPath("abc") == 123);
	ASSERT(v.HasPath(lenpath) == false);

	v = DeserializeWithPayload(input, SERIALIZE_JSON, Variant(params.Copy()).Set("data_path", "other/data"));
	ASSERT(v.GetPath(lenpath) == 100);
	b = v.GetPath("other/data").AsBlob();
	ASSERT(b->GetNumBuffers() == 1);
	ASSERT(memcmp(b->GetPtr(0), TESTPAYLOAD, TESTPAYLOAD_LEN) == 0);

	v = DeserializeWithPayload(input, SERIALIZE_JSON, params);
	ASSERT(v.GetPath(lenpath) == 0);
	ASSERT(v.HasPath(datapath) == false);
}

void TestEmit() {

	Variant params;
	params.Set("length_path", "payload/length").Set("data_path", "other/data");
	Variant v;
	BlobPtr b = Blob::CreateCopy(TESTPAYLOAD, TESTPAYLOAD_LEN);
	v.SetPath("other/data", b);
	SerializeWithPayload(cout.rdbuf(), v, SERIALIZE_JSON, params);
	cout << "\n--------------------------------------------------------------------------------\n";
	params.Set("ignore_payload", true);
	SerializeWithPayload(cout.rdbuf(), v, SERIALIZE_JSON, params);
	cout << "\n--------------------------------------------------------------------------------\n";
	params.Set("ignore_payload", false);
	params.Set("payload_length", 5);
	SerializeWithPayload(cout.rdbuf(), v, SERIALIZE_JSON, params);
	cout << "\n--------------------------------------------------------------------------------\n";
	params.Set("ignore_payload", true);
	params.Set("payload_length", 5);
	SerializeWithPayload(cout.rdbuf(), v, SERIALIZE_JSON, params);
	cout << "\n\n";
}

int main(int argc, char **argv) {
try{
	TestParse();
	TestEmit();
	cout << endl;
} catch (const std::exception &e) {
	cerr << e.what() << endl;
	exit(1);
}
	return 0;
}
