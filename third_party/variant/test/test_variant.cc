//=============================================================================
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 * \brief A simple test for the Variant and JSONToVariant parser.
 */
#include <Variant/Variant.h>
#include <Variant/Emitter.h>
#include <Variant/Parser.h>
#include "TestCommon.h"
#include "TestAssert.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <algorithm>
#include <limits>

using namespace libvariant;
using namespace std;


void VariantTest() {
	std::cout << "Testing some of Variant's functionality\n";

	Variant null;
	ASSERT(null.IsNull());
	Variant v;
	v = 5;
	ASSERT(!v.IsNull());
	ASSERT(v.AsInt() == 5);
	ASSERT(v.AsUnsigned() == 5u);
	ASSERT(v.AsDouble() == 5.0);
	ASSERT(v == Variant(5));
	ASSERT(v != "5");
	Variant b = true;
	ASSERT(b.IsBool());
	ASSERT(b.IsTrue());
	ASSERT(!b.IsFalse());
	b = false;
	ASSERT(b.IsBool());
	ASSERT(b.IsFalse());
	ASSERT(!b.IsTrue());
	Variant map;
	ASSERT(map.Empty());
	map["one"] = v;
	map["two"] = "two";
	ASSERT(map["one"] == v);
	ASSERT(v == map["one"]);
	ASSERT(map.Empty() == false);
	ASSERT(map["one"].Exists());
	ASSERT(!map["nothing"].Exists());
	ASSERT(map.Exists());
	Variant list;
	ASSERT(list.GetType() == Variant::NullType);
	list[0] = 1;
	list[2] = "two";
	list[5] = map;
	list[4] = 123u;
	list[6] = 0.3535566;
	ASSERT(list.GetType() == Variant::ListType);
	ASSERT(!list.Empty());
	ASSERT(list.Size() == 7);
	ASSERT(list[0].GetType() == Variant::IntegerType);
	ASSERT(list[2].GetType() == Variant::StringType);
	ASSERT(list[4].GetType() == Variant::UnsignedType);
	ASSERT(list[5].GetType() == Variant::MapType);
	ASSERT(list[6].GetType() == Variant::FloatType);

	ASSERT(list.Index("two") == 2);
	ASSERT(list.Index("not here") == list.Size());

	VariantRef listref = list;
	ASSERT(listref.Index("two") == 2);
	ASSERT(listref.Index("not here") == listref.Size());

	try {
		map.Index("abc");
		abort();
	} catch (const std::runtime_error &) {}

	try {
		map.AsBool();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsInt();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsUnsigned();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsLongDouble();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsString();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsBlob();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map.AsList();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		map[1];
		abort();
	} catch (const std::runtime_error &) {}
	try {
		list.AsMap();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		list["test"];
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Size();
		abort();
	} catch (const std::runtime_error &) {}
	ASSERT(b.IsBool());
	try {
		b.Empty();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Erase("abc");
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.At("abc");
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.At(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Get("abc");
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Get(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		list.AtPath("[0][1]").As<int>();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Set("abc", 1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		b.Set(1, 1);
		abort();
	} catch (const std::runtime_error &) {}

	b.Clear();
	ASSERT(b.IsNull());
	ASSERT(!b.Contains(123));
	ASSERT(!b.Contains("abc"));
	ASSERT(b.AsBool() == false);
	ASSERT(b.AsInt() == 0);
	ASSERT(b.AsUnsigned() == 0);
	ASSERT(b.AsLongDouble() == 0.0);
	ASSERT(b.AsString() == "");
	ASSERT(b.Empty());
	ASSERT(b.Size() == 0);
	b.Clear();
	b.Erase("abc");
	b.AsMap();
	ASSERT(b.IsMap());
	b.Clear();
	ASSERT(Variant(true).AsString() == "true");
	ASSERT(Variant(false).AsString() == "false");
	ASSERT(Variant(true).AsInt() == 1);
	ASSERT(Variant(false).AsInt() == 0);
	ASSERT(Variant(true).AsUnsigned() == 1);
	ASSERT(Variant(false).AsUnsigned() == 0);
	ASSERT(Variant(true).AsLongDouble() == 1.);
	ASSERT(Variant(false).AsLongDouble() == 0.);
	ASSERT(Variant(false) != Variant());
	ASSERT(Variant(123u).AsBool());
	ASSERT(Variant(123.456).AsUnsigned() == 123u);
	ASSERT(Variant(123.456).AsInt() == 123);
	ASSERT(Variant(123.456).AsBool() == true);
	ASSERT(Variant(0.0).AsBool() == false);
	ASSERT(Variant("true").AsBool() == true);
	ASSERT(Variant("anything").AsBool() == true);
	ASSERT(Variant("false").AsBool() == false);
	ASSERT(Variant("555.5").AsLongDouble() == 555.5);
	ASSERT(Variant("123").AsUnsigned() == 123u);
	ASSERT(Variant("123").AsInt() == 123);
	ASSERT(Variant("").Empty());
	ASSERT(!Variant("abc").Empty());
	b = "abc";
	b.Clear();
	ASSERT(b.Empty());

	cout << "Some simple path testing\n";

	ASSERT(map.GetPath("/") == map);
	ASSERT(map.GetPath("/one") == v);
	ASSERT(list.GetPath("[5]/two") == "two");

	cout << "Testing that the proxy works as expected.\n";
	list[1] = map["one"];
	VariantRef ref = map["three"];
	ref = 3;
	ASSERT(map["three"].AsInt() == 3);

	VariantRef ref2 = map["nothing"];
	ref2 = 4;
	map.Erase("nothing");
	try {
		ref2.AsUnsigned();
		abort();
	} catch (const std::runtime_error &) {}

	try {
		map["four"].IsNull();
		ASSERT(false);
	} catch (const KeyError &e) {}
	ASSERT(!map.Contains("four"));
	ASSERT(map.AsMap().find("four") == map.MapEnd());

	try {
		Variant tmp;
		tmp = map["four"];
		SerializeJSON(stdout, map);
		ASSERT(false);
	} catch (const KeyError &e) {}
	ASSERT(!map.Contains("four"));
	ASSERT(map.AsMap().find("four") == map.MapEnd());

	try {
		Variant tmp = map["four"];
		SerializeJSON(stdout, map);
		ASSERT(false);
	} catch (const KeyError &e) {}
	ASSERT(!map.Contains("four"));
	ASSERT(map.AsMap().find("four") == map.MapEnd());

	cout << "Testing vector constructor\n";
	std::vector<int> intvec;
	intvec.push_back(1);
	intvec.push_back(2);
	intvec.push_back(3);
	intvec.push_back(4);
	Variant v2 = intvec;
	ASSERT(intvec == variant_cast< std::vector<int> >(v2));

	std::vector< std::complex< float > > complexvec;
	complexvec.push_back(std::complex<float>(1, 3));
	complexvec.push_back(std::complex<float>(0, 2));
	complexvec.push_back(std::complex<float>(10, 0));
	complexvec.push_back(std::complex<float>(4, 0));
	Variant complexvec1 = Variant()
		.Append(Variant().Set("real", 1).Set("imag", 3))
		.Append(Variant().Set("imag", 2))
		.Append(Variant().Set("real", 10))
		.Append(4);
	ASSERT(complexvec == variant_cast< std::vector< std::complex<float> > >(complexvec1));
	Variant complexvec2 = Variant()
		.Append(Variant().Append(1).Append(3))
		.Append(Variant().Append(0).Append(2))
		.Append(10)
		.Append(Variant().Append(4));
	std::vector< std::complex< float > > complexvec4 = variant_cast< std::vector< std::complex<float> > >(complexvec2);
	ASSERT(complexvec == complexvec4);

	cout << "Testing map constructor\n";
	std::map<std::string, int> intmap;
	intmap["key1"] = 1;
	intmap["key2"] = 2;
	v2.Append(intmap);
	ASSERT( (intmap == variant_cast< std::map<std::string, int> >(v2[4])) );
	ASSERT(variant_cast<int>(v2[1]) == 2);
	std::vector<int> intvec2(4);
	transform(v2.ListBegin(), v2.ListEnd() - 1, intvec2.begin(), variant_cast<int>);
	ASSERT(intvec2 == intvec);
	ASSERT(v2 == variant_cast<Variant>(v2));

	VariantPrint(v2);
	// Test a case where proxy had a dangling reference on resize.
	for (unsigned i = v2.Size(), end = v2.AsList().capacity()*2; i < end; ++i) {
		v2[i] = v2[i-1];
	}

	cout << "Test Get and GetInto\n";
	int ival;
	map.GetInto(ival, "one", 0);
	ASSERT(ival == v.AsInt());
	ASSERT(map.Get("three").AsUnsigned() == 3);
	map["intmap"] = intmap;
	ival = 0;
	map["intmap"].GetInto(ival, "key1", 0);
	ASSERT(ival == 1);
	try {
		map["none"].GetInto(ival, "blah", 0);
		ASSERT(false);
	} catch (const KeyError &e) {}

	// list == [ 1, 5, "two", null, 123, { "intmap": { "key1": 1, "key2": 2 }, "one": 5, "two": "two", "three": 3 }, 0.3535566 ]
	ASSERT(list == Variant().Append(1).Append(5).Append("two").Append(Variant::NullType)
			.Append(123u).Append(
				Variant().Set("intmap", Variant().Set("key1", 1).Set("key2", 2))
				.Set("one", 5).Set("two", "two").Set("three", 3)
				).Append(0.3535566)
			);

	list.Clear();
	ASSERT(list.Empty());
	ASSERT(list.Size() == 0);
	ASSERT(list.Get(2, 123) == 123);
	ASSERT(list.At(2, 123) == 123);
	ASSERT(list.Get(2) == 123);
	list.Set(2, 345);
	ASSERT(list.Get(2) == 345);
}

void TestCopy() {
	cout << "Testing Copy\n";
	Variant v1;
	v1.SetPath("foo/bar", 3);
	v1.SetPath("list[0]", "abc");
	Variant v2 = v1;
	ASSERT(v1 == v2);
	v2 = v1.Copy();
	v2.SetPath("foo/goo", 4);
	ASSERT(v1 != v2);
	v2 = v1.Copy();
	v2.SetPath("list[1]", 123);
	ASSERT(v1 != v2);
}

void TestRefReassign() {
	cout << "Testing VariantRef reassign\n";

	Variant v1;
	v1.SetPath("a/b/c", 1)
	.SetPath("a/b/d", 2)
	.SetPath("a/d", 4);
	VariantRef ref = v1["a"]["b"]["c"];
	ASSERT(ref == 1);
	ref.ReassignRef(v1.AtPath("a/b/d"));
	ASSERT(ref == 2);
	ref = 5;
	ASSERT(v1.GetPath("a/b/c") == 1);
	ASSERT(v1.GetPath("a/b/d") == 5);

	VariantRef ref1 = v1.AtPath("foo/bar");
	VariantRef ref2 = v1.AtPath("foo").AtPath("bar");
	VariantPrint(v1);
	ASSERT(!v1.HasPath("foo"));
	ASSERT(!v1.HasPath("foo/bar"));

}

void TestProxy() {
	Variant v = Variant().Set("abc", 123).Set("def", 0.456);
	// Error cases
	try {
		v["none"].GetType();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsBool();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsLongDouble();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsUnsigned();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsInt();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsString();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].AsBlob();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		((const Variant &)v["none"]).AsList();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		((const Variant &)v["none"]).AsMap();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Size();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Empty();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Clear();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Contains(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Contains("1");
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Erase("1");
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Add(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Sub(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Mul(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Div(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Rem(1);
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Incr();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Decr();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Neg();
		abort();
	} catch (const std::runtime_error &) {}
	try {
		v["none"].Compare("abc");
		abort();
	} catch (const std::runtime_error &) {}

	std::map<std::string, int> map;
	map["abc"] = 123;
	std::vector<int> list;
	list.push_back(123);
	v["map"] = map;
	v["list"] = list;

	v["abc"].GetType();
	v["abc"].AsBool();
	v["abc"].AsLongDouble();
	v["abc"].AsInt();
	v["abc"].AsUnsigned();
	v["map"].AsMap();
	v["list"].AsList();
	v["map"].Size();
	v["map"].Empty();
	v["map"].Clear();
	v["map"].Contains("abc");
	v["list"].Contains(1);
	v["map"].Erase("foo");
	v["n"] = 0;
	v["n"]++;
	v["n"]--;
	v["n"] + 1;
	v["n"] - 1;
	v["n"] * 1;
	v["n"] / 1;
	v["n"] % 1;
	-v["n"];

	VariantRef ref = v["n"].Ref();
	ASSERT(ref == v["n"]);
	ASSERT(v["map"].Copy() == v["map"]);
	ASSERT(ref.Resolve() == v["n"]);

	ASSERT(ref.As<int>() == v["n"].As<int>());

	Variant v_copy = v.Copy();
	try {
		v.AtPath("/some/path/string").As<int>();
		abort();
	} catch (const KeyError &) {}
	try {
		v_copy["some"]["path"]["string"].As<int>();
		abort();
	} catch (const KeyError &) {}
	try {
		const VariantRef const_ref = v_copy["does not exist"];
		const_ref.Resolve();
		abort();
	} catch (const KeyError &) {}
	ASSERT(v_copy == v);

	VariantRef p = v["p"].Ref();
	v["p"] = 123;
	ASSERT(p == 123);

	try {
		v.GetPath("/some/path/string");
		abort();
	} catch (const KeyError &) {}
	ASSERT(v.AtPath("/some/path/string", "value") == "value");
	v.AtPath("/some/other/path") = 123;
	ASSERT(v.GetPath("/some/other/path") == 123);
	VariantRef ref_copy = v_copy.AtPath("/some/path/string");
	ref_copy.Resolve() = "value";
	ASSERT(v_copy.AtPath("/some/path/string") == "value");

	v.AtPath("/one/two/three").Resolve(123);
	v.AtPath("/some/path/string").Resolve(123);
	ASSERT(v.GetPath("/one/two/three") == 123);
	ASSERT(v.GetPath("/some/path/string") == "value");
	v.ErasePath("/one/two/three", true);
	v.ErasePath("p");
	v.ErasePath("some/other/path", true);
	v.ErasePath("/some/path/string");
	v_copy.ErasePath("/some/path/string");
	ASSERT(v == v_copy);
	ASSERT(v.HasPath("some/path"));
	v.SetPath("list[1]/a", 123);
	v.ErasePath("list[1]/a", true);
	ASSERT(v == v_copy);

}

void VariantTestJSONParsing() {
	std::cout << "Testing JSON -> Variant parsing\n";
	Variant v;
	Variant param;
	param["pretty"] = true;
	Emitter emitter = JSONEmitter(CreateEmitterOutput(std::cout.rdbuf()), param);
	std::string text = "[1,2,3,4,{\"A\":2}, \"text\"]";
	text += "[\"doodle\", \"foo\", \"bar\", \"\\b\\f\\r\\u0001\\u1111\", true, false]";
	std::istringstream iss(text);
	Parser jv = JSONParser(CreateParserInputFile(iss.rdbuf()));
	v = ParseVariant(jv);
	emitter << v;
	v = v;
	ASSERT(v.IsList());
	ASSERT(v[4].IsMap());
	ASSERT(v[5].IsString());
	ASSERT(v[4]["A"].IsNumber());
	v[4]["b"];
	ASSERT(!v[4].Contains("b"));
	v[10];
	ASSERT(!v.Contains(10));
	const Variant cv = v;
	ASSERT(cv.IsList());
	ASSERT(cv[4].IsMap());
	ASSERT(cv[5].IsString());
	ASSERT(cv[4]["A"].IsNumber());
	ASSERT(!cv[4].Contains("b"));
	try {
		cv[10];
		ASSERT(false);
	} catch (const std::out_of_range&) {

	}
	v = v[4];
	v = "\n";

	jv.Reset();
	v = ParseVariant(jv);
	ASSERT(jv.Done());
	ASSERT(v[1].AsString() == "foo");

	emitter.Flush();
	std::cout << std::endl;
	// Test that really big indents work...
	Serialize(std::cout.rdbuf(), v, SERIALIZE_JSON, Variant().Set("pretty", true).Set("indent", 200));
	std::cout << std::endl;

}

void VariantTestYAMLParsing() {
	std::cout << "Testing YAML -> Variant parsing\n";
	std::string text = "some: yaml\na key: and a value\n"
		"some more: [a, list, 123, values, 1.23, null, true, false]\n"
		"max uintmax: 18446744073709551616\n"
		"---\n"
		"Lets try multiple documents!\n";
	Parser parser = YAMLParser(CreateParserInput(text.c_str(), text.length()));
	Variant v1 = ParseVariant(parser);
	Variant v2 = ParseVariant(parser);
	ASSERT(v1["some"].IsString());
	ASSERT(v1["some more"].IsList());
	ASSERT(v1["some more"].Size() == 8);
	ASSERT(v1["some more"][6].IsTrue());
	ASSERT(v1["max uintmax"].AsUnsigned() == std::numeric_limits<uintmax_t>::max());
	ASSERT(v2.IsString());
}

void PrintError(std::vector<Variant> &stack, const std::string &msg, Variant orig, Variant cur) {
	cerr << "Error comparing variants at: ";
	for (std::vector<Variant>::iterator i(stack.begin()), e(stack.end()); i!=e; ++i) {
		cerr << i->AsString() << ": ";
	}
	cerr << "\n\n" << msg;
	Emitter emitter = JSONEmitter(CreateEmitterOutput(cerr.rdbuf()));
	emitter.BeginMap();
	emitter << "orig" << orig << "cur" << cur;
	emitter.EndMap();
	emitter.Close();
	cerr << "\n";
	abort();
}

void DoDiff(Variant orig, Variant cur, std::vector<Variant> &stack) {
	if (orig != cur) {
		if (orig.GetType() != cur.GetType()) {
			PrintError(stack, "differing types", orig, cur);
		}
		// find the difference and print it out
		switch (orig.GetType()) {
		case Variant::NullType:
			abort();
		case Variant::BoolType:
			PrintError(stack, "differing boolean", orig, cur);
			abort();
			break;
		case Variant::IntegerType:
		case Variant::UnsignedType:
		case Variant::FloatType:
			abort();
			break;
		case Variant::StringType:
			abort();
			break;
		case Variant::ListType:
			for (unsigned i = 0; i < orig.Size(); ++i) {
				stack.push_back(i);
				DoDiff(orig[i], cur[i], stack);
				stack.pop_back();
			}
			break;
		case Variant::MapType:
			for (Variant::MapIterator i(orig.MapBegin()), e(orig.MapEnd()); i!=e; i++) {
				stack.push_back(i->first);
				DoDiff(i->second, cur[i->first], stack);
				stack.pop_back();
			}
			break;
		case Variant::BlobType:
			abort();
			break;
		}
		PrintError(stack, "Comparison showed different but cannot find difference", orig, cur);
	}
}

void Diff(Variant orig, Variant cur) {
	std::vector<Variant> stack;
	DoDiff(orig, cur, stack);
}

void TestVariantEmitParseRandom(int verbose, int numtests, Parser (*Parser_t)(shared_ptr<ParserInput>), Emitter (*Emitter_t)(shared_ptr<EmitterOutput>, Variant), bool allow_nan) {
	for (int i = 0; i < numtests; ++i) {
		if (verbose > 0) {
			std::cout << "Random test number " << i << std::endl;
		}
		Variant v = GenerateRandomVariant(allow_nan);
		ASSERT(!v.IsNull());
		std::stringstream ss;
		Emitter emitter = Emitter_t(CreateEmitterOutput(ss.rdbuf()), Variant());
		emitter << v;
		emitter.Close();
		if (verbose > 1) {
			std::cout << ss.str() << std::endl;
		}
		Parser parser = Parser_t(CreateParserInputFile(ss.rdbuf()));
		Variant v2 = ParseVariant(parser);
		Diff(v, v2);
		if (verbose > 0) {
			std::cout << "End random test number " << i << std::endl;
		}
	}
}

int main(int argv, char** argc) {
	VariantTest();
	TestCopy();
	TestRefReassign();
	TestProxy();
	VariantTestJSONParsing();
#ifdef ENABLE_YAML
	VariantTestYAMLParsing();
#endif
	int verbose = 1;
	int num_tests = 100;
	
	cout << "Random test of JSON\n";
	TestVariantEmitParseRandom(verbose, num_tests, JSONParser, JSONEmitter, false);
#ifdef ENABLE_YAML
	cout << "Random test of YAML\n";
	TestVariantEmitParseRandom(verbose, num_tests, YAMLParser, YAMLEmitter, true);
#endif
#ifdef ENABLE_XML
	cout << "Random test of XML\n";
	TestVariantEmitParseRandom(verbose, num_tests, XMLPLISTParser, XMLPLISTEmitter, true);
#endif
#ifdef ENABLE_MSGPACK
	cout << "Random test of MsgPack\n";
	TestVariantEmitParseRandom(verbose, num_tests, MsgPackParser, MsgPackEmitter, true);
#endif
	return 0;
}
