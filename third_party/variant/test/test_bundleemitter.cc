/** \file
 * \author John Bridgman
 * \brief 
 */

#include "TestAssert.h"
#include <Variant/Variant.h>
#include <Variant/Payload.h>
#include <Variant/Emitter.h>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sstream>
#include <iostream>
using namespace libvariant;
using namespace std;

void TestOne() {
	cout << "Testing basic functionality" << endl;
	Variant v = Variant::MapType;
	v["foo"] = 12.33456;
	v["bar"] = 873453;
	v["boo"] = false;
	v["null"] = Variant::NullType;
	v["true"] = true;
	v["moo"] = "test one two three blah blah blah";
	v["loo"].Append(1).Append(2).Append(3);
	const char payload[] = "fp9q8ywtrvapwn98nc4yargcfuiahwer8ytqv9p4tcp98y3crhlei4t8hpse985ytvlseuhrotceyiruth";
	v["payload.data"] = Blob::CreateReferenced((char*)payload, sizeof(payload));
	std::vector<char> buff;
	buff.resize(8192);
	unsigned len = SerializeBundle(&buff[0], buff.size(), v);
	buff.resize(len);
	ASSERT((buff.size() - strlen(&buff[0]) - 1) == sizeof(payload));
	ASSERT(memcmp(payload, &buff[strlen(&buff[0]) + 1], sizeof(payload)) == 0);
	fwrite(&buff[0], 1, buff.size(), stdout);
	printf("\n");
	Variant val = DeserializeBundle(&buff[0], buff.size());
	ASSERT(val["bar"] == v["bar"]);
	ASSERT(val["moo"] == v["moo"]);
	ASSERT(val["loo"] == v["loo"]);
	ASSERT(val["boo"].AsInt() == 0); // bundles don't preserve boolean type information
}

void TestErrorPaths() {
	cout << "Testing error paths" << endl;
	std::ostringstream oss;
	Emitter e = BundleHdrEmitter(CreateEmitterOutput(oss.rdbuf()));

	// Test error paths
	e.BeginDocument();
	try {
		e.BeginDocument();
		abort();
	} catch (const std::runtime_error &) {}
	e.BeginMap();
	try {
		e.BeginMap();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		e.BeginList();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		e.EndList();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		e.Emit("key").Emit((ConstBlobPtr)Blob::CreateCopy((void*)0,0));
		abort();
	} catch (const std::runtime_error &) {}
	e.Emit(true);
	e.EndMap();
	try {
		e.EndMap();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		e.Emit("abc");
		abort();
	} catch (const std::runtime_error &) {}
	e.EndDocument();
	try {
		e.EndDocument();
		abort();
	} catch (const std::runtime_error &) {}
}

int main(int argc, char **argv) {
	TestOne();
	TestErrorPaths();
	return 0;
}
