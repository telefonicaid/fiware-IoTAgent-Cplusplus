/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Schema.h>
#include <Variant/SchemaLoader.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

using namespace libvariant;
using namespace std;

int TestNormalise() {
	int ret = 0;
	cout << "Test Normalise\n";

	cout << "normalise - untouched immediate $ref ";
	Variant schema;
	schema["items"]["$ref"] = "#";
	SchemaLoader::NormSchema(schema, "");
	if (schema["items"]["$ref"] == "#") {
		cout << "PASS" << endl;
	} else {
		cout << "!!! FAIL !!!" << endl;
		++ret;
		abort();
	}

	cout << "normalise - id as base ";
	schema = Variant::NullType;
	schema["id"] = "baseUrl";
	schema["items"]["$ref"] = "#";
	SchemaLoader::NormSchema(schema, "");
	if (schema["items"]["$ref"] == "baseUrl#") {
		cout << "PASS" << endl;
	} else {
		cout << "!!! FAIL !!!" << endl;
		++ret;
		abort();
	}

	cout << "normalise - id relative to parent ";
	schema = Variant::NullType;
	schema["id"] = "http://example.com/schema";
	schema["items"]["id"] = "otherSchema";
	schema["items"]["items"]["$ref"] = "#";
	SchemaLoader::NormSchema(schema, "");
	if (schema["items"]["id"] == "http://example.com/otherSchema"
			&& schema["items"]["items"]["$ref"] == "http://example.com/otherSchema#"
			) {
		cout << "PASS" << endl;
	} else {
		cout << "!!! FAIL !!!" << endl;
		++ret;
		abort();
	}

	cout << "normalise - do not touch contents of \"enum\" ";

	schema = Variant::NullType;
	schema["id"] = "http://example.com/schema";
	schema["items"]["id"] = "otherSchema";
	schema["items"]["enum"][0]["$ref"] = "#";
	SchemaLoader::NormSchema(schema, "");

	if (schema["items"]["enum"][0]["$ref"] == "#") {
		cout << "PASS" << endl;
	} else {
		cout << "!!! FAIL !!!" << endl;
		++ret;
		abort();
	}

	cout << "Only normalise id and $ref if they are strings ";
	schema = Variant::NullType;
	schema["properties"]["id"]["type"] = "integer";
	schema["properties"]["$ref"]["type"] = "integer";
	Variant data;
	data["id"] = "test";
	data["$ref"] = "test";
	SchemaLoader::NormSchema(schema, "");
	if (!SchemaValidate(schema, data).Error()) {
		cout << " !!! FAIL !!! " << endl;
		++ret;
		abort();
	} else {
		cout << " PASS " << endl;
	}

	return ret;
}

bool TestMissingHandler(SchemaLoader *sb, const std::string &uri, SchemaLoader::LoaderError &error, void *m) {
	vector<string> &missing = *(vector<string>*)(m);
	cout << endl;
	cout << "Validator missing schema: \"" << uri << "\"" << endl;
	missing.push_back(uri);
	return true;
}

int TestFile(const std::string &testfile) {
	int ret = 0;
	cout << "Processing file " << testfile << endl;
	cout << "-------------------------------------\n";
	cout.flush();
	Variant tests = DeserializeJSONFile(testfile.c_str());
	for (Variant::ListIterator i(tests.ListBegin()), e(tests.ListEnd()); i != e; ++i) {
		std::string name = i->At("name").AsString();
		Variant data = i->At("data");
		Variant schema = i->At("schema");
		bool fail = i->Get("fail", false).AsBool();
		bool exception = i->Get("exception", false).AsBool();
		cout << "Running " << (fail ? "negative " : "") << "test \"" << name << "\" ";
		cout.flush();
		SchemaLoader loader;
		std::vector<std::string> missing;
		loader.SetMissingHandler(&TestMissingHandler, &missing);
		SchemaResult result;
		try {
			result = SchemaValidate(schema, data, &loader);
		} catch (const std::exception &e) {
			if (exception) {
				cout << " PASS with exception\n";
				cout << e.what() << endl;
				continue;
			}
			cout << "\n!!!!FAIL!!!!\n";
			cout << e.what() << endl;
			++ret;
			abort();
		}
		if (exception) {
			cout << "\n!!!!FAIL!!!!\n";
			cout << "Excepted SchemaValidate to throw!\n";
			++ret;
			abort();
		}

		for (unsigned k = 0; k < missing.size(); ++k) {
			bool found = false;
			Variant m = i->Get("missing", Variant::ListType);
			for (Variant::ListIterator j(m.ListBegin()), e(m.ListEnd()); j != e && !found; ++j) {
				if (*j == missing[k]) {
					found = true;
				}
			}
			if (!found) {
				cout << "\n!!!!FAIL!!!!\n";
				cout << "Unexpected missing schema: \"" << missing[k] << "\"\n";
				++ret;
				abort();
			}
		}

		if (fail && !result.Error()) {
			cout << "\n!!!!!FAIL!!!!!\n";
			cout << result;
			++ret;
			abort();
		} else if (!fail && result.Error()) {
			cout << "\n!!!!!FAIL!!!!!\n";
			cout << result;
			++ret;
			abort();
		} else {
			cout << " PASS";
			if (result.Error()) {
				cout << endl << result;
			}
		}
		cout << endl;
	}
	cout << endl;
	return ret;
}

int TestDirectory(const std::string &dirname) {
	int ret = 0;
	DIR *dir = opendir(dirname.c_str());
	if (!dir) { return 1; }
	cout << "\nProcessing " << dirname << " for tests." << endl;
	cout << "=========================================\n";
	dirent *ep;
	while ((ep = readdir(dir))) {
		string fname = ep->d_name;
		if (fname == ".") continue;
		if (fname == "..") continue;
		if (ep->d_type == DT_REG) {
			if (fname.find(".") < fname.size() && fname.substr(fname.rfind(".")) == ".test") {
				ret += TestFile(dirname + "/" + fname);
			}
		}
		if (ep->d_type == DT_DIR) {
			ret += TestDirectory(dirname + "/" + fname);
		}
	}
	cout << endl;
	closedir(dir);
	return ret;
}

int TestSchemaLoader() {
	cout << "Test advanced schema loader\n";
	int ret = 0;
	Variant data = DeserializeGuessFile(TEST_SCHEMA_DATA "/simple-test.json");
	Variant faildata = DeserializeGuessFile(TEST_SCHEMA_DATA "/simple-test-fail.json");
	try {
		AdvSchemaLoader loader;
		SchemaResult result = SchemaValidate("simple-test-schema.json", data, &loader);
		cout << "Look up of schema without path succeeded.";
		abort();
		++ret;
	} catch (const std::exception &e) {}
	cout << endl;

	{
		AdvSchemaLoader loader;
		cout << "Testing absolute path.\n";
		SchemaResult result = SchemaValidate(TEST_SCHEMA_DATA "/simple-test-schema.json", data, &loader);
		SchemaResult failresult = SchemaValidate(TEST_SCHEMA_DATA "/simple-test-schema.json", faildata, &loader);
		if (result.Error()) {
			cout << result << endl;
			++ret;
		}
		if (!failresult.Error()) {
			cout << "!!! Negative test failed\n";
			++ret;
		} else {
			//cout << failresult << endl;
		}
	}
	cout << endl;

	{
		AdvSchemaLoader loader;
		cout << "Testing advanced schema loader, loading a file through the path.\n";
		loader.AddPath(TEST_SCHEMA_DATA);
		SchemaResult result = SchemaValidate("simple-test-schema.json", data, &loader);
		SchemaResult failresult = SchemaValidate("simple-test-schema.json", faildata, &loader);
		if (result.Error()) {
			cout << result << endl;
			++ret;
		}
		if (!failresult.Error()) {
			cout << "!!! Negative test failed\n";
			++ret;
		} else {
			//cout << failresult << endl;
		}
	}
	cout << endl;

	{
		AdvSchemaLoader loader;
		cout << "Testing file:// absolute path\n";
		SchemaResult result = SchemaValidate("file://" TEST_SCHEMA_DATA "/simple-test-schema.json", data, &loader);
		SchemaResult failresult = SchemaValidate("file://" TEST_SCHEMA_DATA "/simple-test-schema.json", faildata, &loader);
		if (result.Error()) {
			cout << result << endl;
			++ret;
		}
		if (!failresult.Error()) {
			cout << "!!! Negative test failed\n";
			++ret;
			abort();
		} else {
			//cout << failresult << endl;
		}
	}
	cout << endl;

	{
		AdvSchemaLoader loader;
		cout << "Testing file:// relative path\n";
		loader.AddPath(TEST_SCHEMA_DATA);
		SchemaResult result = SchemaValidate("file://" "simple-test-schema.json", data, &loader);
		SchemaResult failresult = SchemaValidate("file://" "simple-test-schema.json", faildata, &loader);
		if (result.Error()) {
			cout << result << endl;
			++ret;
		}
		if (!failresult.Error()) {
			cout << "!!! Negative test failed\n";
			++ret;
			abort();
		} else {
			//cout << failresult << endl;
		}
	}
	cout << endl;

	try {
		AdvSchemaLoader loader;
		loader.AddPath(TEST_SCHEMA_DATA);
		cout << "Test that missing schema works right...\n";
		SchemaResult result = SchemaValidate("abc-no-such-schema", data, &loader);
		cout << "Failed!\n";
		++ret;
	} catch (const std::exception &e) {
		cout << "Error:\n" << e.what() << endl;
	}

	return ret;
}

int TestSchemaDefaults() {
	cerr << "Testing schema defaults\n";
	int ret = 0;
	Variant schema;
	schema.SetPath("properties/foo/default", 123)
	.SetPath("properties/bar/default", "abc")
	.SetPath("default/foo", 456)
	.SetPath("default/bar", "def")
	.SetPath("properties/list/items/default", false)
	.SetPath("definitions/zero/default", 0)
	.SetPath("properties/zero/$ref", "#/definitions/zero");


	Variant data;
	AddSchemaDefaults(schema, data);
	if ((data["foo"] != 456 || data["bar"] != "def") && !data.Contains("list")) {
		cerr << "Defaults test failed:\n"
			"Got: \n";
		Serialize(cerr.rdbuf(), data, SERIALIZE_JSON);
		++ret;
	}
	data.Erase("foo");
	AddSchemaDefaults(schema, data);
	if (data["foo"] != 123 || !data["list"][0].IsFalse() || data["zero"] != 0) {
		cerr << "Defaults test failed:\n"
			"Got: \n";
		Serialize(cerr.rdbuf(), data, SERIALIZE_JSON);
		++ret;
	}
	schema.SetPath("properties/alltest/allOf[0]/default", "test");
	AddSchemaDefaults(schema, data);
	if (data["alltest"] != "test") {
		cerr << "Defaults test failed:\n"
			"Got: \n";
		Serialize(cerr.rdbuf(), data, SERIALIZE_JSON);
		++ret;
	}
	return ret;
}

int main(int argc, char **argv) {
	while (true) {
		int c = getopt(argc, argv, "f:");
		if (c == -1) { break; }
		switch (c) {
		case 'f':
			return TestFile(optarg);
		default:
			break;
		}
		
	}
	int i = 0;
	int ret = 0;
	cout << "Testing SchemaLoader::ResolveURI ";
	std::string ruri_file = TEST_SCHEMA_DATA;
	ruri_file += "/test_resolveuri.json";
	Variant v = DeserializeJSONFile(ruri_file.c_str());
	while (true) {
		std::string ref, base, expected;
		ref = v[3*i].AsString();
		base = v[3*i+1].AsString();
		expected = v[3*i+2].AsString();
		if (ref == "" && base == "" && expected == "") { break; }
		string res = SchemaLoader::ResolveURI(base, ref);
		if (res != expected) {
			cout << "\nResolveURI test " << i << " failed\n"
				<< "ref: \"" << ref << "\"\n"
				<< "base: \"" << base << "\"\n"
				<< "result:   \"" << res << "\"\n"
				<< "expected: \"" << expected << "\""
				<< endl;
			++ret;
			abort();
		} else if (i%5 == 0) {
			cout << ".";
			cout.flush();
		}
		++i;
	}
	cout << endl;

	ret += TestNormalise();

	cout << "Performing schema validation tests" << endl;
	ret += TestDirectory(TEST_SCHEMA_DATA);

	ret += TestSchemaLoader();

	ret += TestSchemaDefaults();

	cerr << "================================================================================\n";
	if (ret > 0) {
		cerr << "FAIL!\n";
	} else {
		cerr << "All tests PASS\n";
	}
	return ret == 0 ? 0 : 1;
}
