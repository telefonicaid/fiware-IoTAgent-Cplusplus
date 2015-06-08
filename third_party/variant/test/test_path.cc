/** \file
 * \author John Bridgman
 * \brief 
 */

#include "TestAssert.h"
#include <Variant/Variant.h>
#include <Variant/Path.h>
#include <iostream>
#include <stdlib.h>

using namespace libvariant;
using namespace std;

// {
//   "key1": [ 1, 2, 3, { "blork": true } ],
//   "key2": {
//     "subkey": [ "a", { "furple": 1 }, 2 ],
//     "sub/key2": true
//     }
// }
//
// Then all paths enumerated are:
// Path						Value
// "/"						Identity
// "/key1"					[ 1, 2, 3, { "blork": true } ],
// "/key1[0]"				1
// "/key1[1]"				2
// "/key1[2]"				3
// "/key1[3]"				{ "blork": true }
// "/key1[3]/blork"			true
// "/key2"					{ "subkey": [ "a", { "furple": 1 }, 2 ], "sub/key2": true  }
// "/key2/subkey"			[ "a", { "furple": 1 }, 2 ]
// "/key2/subkey[0]"		"a"
// "/key2/subkey[1]"		{ "furple": 1 }
// "/key2/subkey[1]/furple"	1
// "/key2/subkey[2]"		2
// "/key2/sub\/key2"		true
void TestOne() {

	Variant v = Variant::MapType;
	Variant l = Variant::ListType;
	l[0] = 1; l[1] = 2; l[2] = 3;
	l[3] = Variant::MapType;
	l[3]["blork"] = true;
	v["key1"] = l;
	Variant sub = Variant::MapType;
	l = Variant::ListType;
	l[0] = "a";
	l[1] = Variant::MapType;
	l[1]["furple"] = 1;
	l[2] = 2;
	sub["subkey"] = l;
	sub["sub/key"] = true;
	v["key2"] = sub;

	ASSERT(v.GetPath("/") == v);
	ASSERT(v.GetPath("") == v);
	ASSERT(v.GetPath("/key1") == v["key1"]);
	ASSERT(v.GetPath("/key1[0]") == v["key1"][0]);
	ASSERT(v.GetPath("/key1[1]") == v["key1"][1]);
	ASSERT(v.GetPath("/key1[2]") == v["key1"][2]);
	ASSERT(v.GetPath("/key1[3]") == v["key1"][3]);
	ASSERT(v.GetPath("/key1[3]/blork") == v["key1"][3]["blork"]);
	ASSERT(v.GetPath("/key2") == v["key2"]);
	ASSERT(v.GetPath("/key2/subkey") == v["key2"]["subkey"]);
	ASSERT(v.GetPath("/key2/subkey[0]") == v["key2"]["subkey"][0]);
	ASSERT(v.GetPath("/key2/subkey[1]") == v["key2"]["subkey"][1]);
	ASSERT(v.GetPath("/key2/subkey[1]/furple") == v["key2"]["subkey"][1]["furple"]);
	ASSERT(v.GetPath("/key2/subkey[2]") == v["key2"]["subkey"][2]);
	ASSERT(v.GetPath("/key2/sub\\/key") == v["key2"]["sub/key"]);

	ASSERT(v.GetPath("/key1/") == v["key1"]);
	ASSERT(v.GetPath("/key1[0]/") == v["key1"][0]);
	ASSERT(v.GetPath("/key1[1]/") == v["key1"][1]);
	ASSERT(v.GetPath("/key1[2]/") == v["key1"][2]);
	ASSERT(v.GetPath("/key1[3]/") == v["key1"][3]);
	ASSERT(v.GetPath("/key1[3]/blork/") == v["key1"][3]["blork"]);
	ASSERT(v.GetPath("/key2/") == v["key2"]);
	ASSERT(v.GetPath("/key2/subkey/") == v["key2"]["subkey"]);
	ASSERT(v.GetPath("/key2/subkey[0]/") == v["key2"]["subkey"][0]);
	ASSERT(v.GetPath("/key2/subkey[1]/") == v["key2"]["subkey"][1]);
	ASSERT(v.GetPath("/key2/subkey[1]/furple/") == v["key2"]["subkey"][1]["furple"]);
	ASSERT(v.GetPath("/key2/subkey[2]/") == v["key2"]["subkey"][2]);
	ASSERT(v.GetPath("/key2/sub\\/key/") == v["key2"]["sub/key"]);

	ASSERT(!v.HasPath("/nothing"));

	ASSERT(v.GetPath("key1/") == v["key1"]);
	ASSERT(v.GetPath("key1[0]/") == v["key1"][0]);
	ASSERT(v.GetPath("key1[1]/") == v["key1"][1]);
	ASSERT(v.GetPath("key1[2]/") == v["key1"][2]);
	ASSERT(v.GetPath("key1[3]/") == v["key1"][3]);
	ASSERT(v.GetPath("key1[3]/blork/") == v["key1"][3]["blork"]);
	ASSERT(v.GetPath("key2/") == v["key2"]);
	ASSERT(v.GetPath("key2/subkey/") == v["key2"]["subkey"]);
	ASSERT(v.GetPath("key2/subkey[0]/") == v["key2"]["subkey"][0]);
	ASSERT(v.GetPath("key2/subkey[1]/") == v["key2"]["subkey"][1]);
	ASSERT(v.GetPath("key2/subkey[1]/furple/") == v["key2"]["subkey"][1]["furple"]);
	ASSERT(v.GetPath("key2/subkey[2]/") == v["key2"]["subkey"][2]);
	ASSERT(v.GetPath("key2/sub\\/key/") == v["key2"]["sub/key"]);


	ASSERT(v.AtPath("key1/") == v["key1"]);
	ASSERT(v.AtPath("key1[0]/") == v["key1"][0]);
	ASSERT(v.AtPath("key1[1]/") == v["key1"][1]);
	ASSERT(v.AtPath("key1[2]/") == v["key1"][2]);
	ASSERT(v.AtPath("key1[3]/") == v["key1"][3]);
	ASSERT(v.AtPath("key1[3]/blork/") == v["key1"][3]["blork"]);
	ASSERT(v.AtPath("key2/") == v["key2"]);
	ASSERT(v.AtPath("key2/subkey/") == v["key2"]["subkey"]);
	ASSERT(v.AtPath("key2/subkey[0]/") == v["key2"]["subkey"][0]);
	ASSERT(v.AtPath("key2/subkey[1]/") == v["key2"]["subkey"][1]);
	ASSERT(v.AtPath("key2/subkey[1]/furple/") == v["key2"]["subkey"][1]["furple"]);
	ASSERT(v.AtPath("key2/subkey[2]/") == v["key2"]["subkey"][2]);
	ASSERT(v.AtPath("key2/sub\\/key/") == v["key2"]["sub/key"]);

	Variant o;
	o["foo"] = v;
	VariantRef p = o["foo"];
	ASSERT(p.AtPath("key1/") == v["key1"]);
	ASSERT(p.GetPath("key1/") == v["key1"]);
	ASSERT(!p.HasPath("/nothing"));

	try {
		Variant v2;
	   	v2 = v.AtPath("/nothing");
		VariantPrint(v2);
		VariantPrint(v);
		abort();
	} catch (const std::runtime_error &) {}

	try {
		Variant v2 = v.AtPath("/nothing");
		VariantPrint(v2);
		VariantPrint(v);
		abort();
	} catch (const std::runtime_error &) {}

	try {
		Variant v2 = v.AtPath("/key1/nothing");
		VariantPrint(v2);
		abort();
	} catch (const std::runtime_error &) {}

	Variant v2;
	v2.AtPath("abc/rxy") = 123;
	VariantPrint(v2);

	Variant s;
	s.SetPath("/key1[0]", 1)
		.SetPath("/key1[1]", 2)
		.SetPath("/key1[2]", 3)
		.SetPath("/key1[3]/blork", true)
		.SetPath("/key2/subkey[0]", "a")
		.SetPath("/key2/subkey[1]/furple", 1)
		.SetPath("/key2/subkey[2]", 2)
		.SetPath("/key2/sub\\/key", true);
	ASSERT(v == s);

	s.SetPath("/a/b/c", true);
	ASSERT(s["a"]["b"]["c"].IsTrue());
	s.SetPath("/a/b/c", false);
	ASSERT(s["a"]["b"]["c"].IsFalse());
	s.SetPath("/a/c[0]", 1);
	ASSERT(s["a"]["c"][0].AsInt() == 1);
	try {
		s.SetPath("/a/c/c", 123);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		s.SetPath("/a/b/d[abc]", 1);
		abort();
	} catch (const std::runtime_error &) {}
}

void TestTwo() {
	Variant l = Variant::ListType;
	l.Append("a");
	l.Append("b");
	l.Append(Variant::MapType);
	l[2]["a"] = 1;
	l[2]["b"] = Variant::MapType;
	l[2]["b"]["c"] = 3u;
	ASSERT(l.GetPath("[0]") == "a");
	ASSERT(l.GetPath("[1]") == "b");
	ASSERT(l.GetPath("[2]/a") == 1);
	ASSERT(l.GetPath("[2]/b/c") == 3u);
	ASSERT(l.GetPath("[2]/b/c").IsUnsigned());
	ASSERT(l.GetPath("[3]/a", 4) == 4);
	ASSERT(l.Contains(3) == false);

	Variant s;
	s.SetPath("[0]", "a")
		.SetPath("[1]", "b")
		.SetPath("[2]/a", 1)
		.SetPath("[2]/b/c", 3u);
	ASSERT(l == s);
	s.SetPath("[3]/a/b/c", 123);
	ASSERT(s[3]["a"]["b"]["c"].AsInt() == 123);
	try {
		s.SetPath("/a/b/c", 123);
		ASSERT(false);
	} catch (const std::runtime_error &) {}
	try {
		s.GetPath("[a]");
		ASSERT(false);
	} catch (const std::runtime_error &) {}
}

int main(int argc, char **argv) {
	TestOne();
	TestTwo();

	Variant v;
	Variant b = v.AtPath("abc/123", Variant::MapType);
	ASSERT(b.IsMap());
	b["def"] = 123;
	ASSERT(v["abc"].AtPath("123/def") == 123);
	ASSERT(v["abc"].At("456", "foo") == "foo");
	ASSERT(v["abc"]["456"] == "foo");
	ASSERT(v["abc"].AtPath("123/rst", "abc") == "abc");
	ASSERT(b["rst"] == "abc");
	b = v.At("list", Variant::ListType);
	ASSERT(b.IsList());
	ASSERT(v.AtPath("list[1]", "123") == "123");
	ASSERT(v.AtPath("list[0]/abc", "abc") == "abc");
	VariantPrint(v);
	return 0;
}
