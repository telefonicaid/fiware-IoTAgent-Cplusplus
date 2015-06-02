#include <limits>
#include <iostream>
#include <Variant/Variant.h>
using namespace libvariant;
using namespace std;

#include <stdint.h>

int DoTest(SerializeType stype) {
	Variant v1;
	v1["bool_true"] = true;
	v1["bool_false"] = false;
	v1["long_double_max"] = numeric_limits<long double>::max();
	v1["long_double_min"] = numeric_limits<long double>::min();
	v1["double_max"] = numeric_limits<double>::max();
	v1["double_min"] = numeric_limits<double>::min();
	v1["int_max"] = numeric_limits<intmax_t>::max();
	v1["int_min"] = numeric_limits<intmax_t>::min();
	v1["uint_max"] = numeric_limits<uintmax_t>::max();
	v1["uint_min"] = numeric_limits<uintmax_t>::min();

	string s = Serialize(v1, stype);
	Variant v2 = Deserialize(s, stype);

	int result = 0;

	if (variant_cast<bool>(v2["bool_true"]) != true) {
		cerr << "bool true failed" << endl;
		result++;
	}
	if (variant_cast<bool>(v2["bool_false"]) != false) {
		cerr << "bool false failed" << endl;
		result++;
	}

#if 0
	// long double doesn't roundtrip!

	if (variant_cast<long double>(v2["long_double_max"]) != numeric_limits<long double>::max()) {
		cerr << "long double max failed" << endl;
		result++;
	}
	if (variant_cast<long double>(v2["long_double_min"]) != numeric_limits<long double>::min()) {
		cerr << "long double min failed" << endl;
		result++;
	}
#endif

	if (variant_cast<double>(v2["double_max"]) != numeric_limits<double>::max()) {
		cerr << "double max failed" << endl;
		result++;
	}
	if (variant_cast<double>(v2["double_min"]) != numeric_limits<double>::min()) {
		cerr << "double min failed" << endl;
		result++;
	}
	
	if (variant_cast<intmax_t>(v2["int_max"]) != numeric_limits<intmax_t>::max()) {
		cerr << "intmax_t max failed" << endl;
		result++;
	}
	if (variant_cast<intmax_t>(v2["int_min"]) != numeric_limits<intmax_t>::min()) {
		cerr << "intmax_t min failed" << endl;
		result++;
	}

	if (variant_cast<uintmax_t>(v2["uint_max"]) != numeric_limits<uintmax_t>::max()) {
		cerr << "uintmax_t max failed" << endl;
		result++;
	}
	if (variant_cast<uintmax_t>(v2["uint_min"]) != numeric_limits<uintmax_t>::min()) {
		cerr << "uintmax_t min failed" << endl;
		result++;
	}

	return result;
}

int main(int argv, char** argc) {
	int result = 0;
	result += DoTest(SERIALIZE_JSON);
#if ENABLE_YAML
	result += DoTest(SERIALIZE_YAML);
#endif
#if ENABLE_XML
	result += DoTest(SERIALIZE_XMLPLIST);
#endif
	result += DoTest(SERIALIZE_BUNDLEHDR);
	return result;
}
