/** \file
 * \author John Bridgman
 * \brief 
 */

#include "TestCommon.h"
#include <Variant/Variant.h>
#include <iostream>
#include <sstream>
#include <sys/time.h>
#include <string.h>
#include <stdexcept>
#include <errno.h>

using namespace libvariant;
using namespace std;

static double getTime() {
	timeval tv;
	if (gettimeofday(&tv, 0) != 0) {
		throw std::runtime_error(strerror(errno));
	}
	return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

ostream &operator<<(ostream &os, Variant v) {
	SerializeJSON(os.rdbuf(), v, true);
	return os;
}

static const char *suffix[] = { "y", "z", "a", "f", "p", "n", "u", "m", "",
"k", "M", "G", "T", "P", "E", "Z", "Y" };
static const int suffix_len = sizeof(suffix)/sizeof(char *);

class engrfmt {
public:
	engrfmt(double v)
		: value(v){}

	void Print(std::ostream &os) const {
		double v = value;
		int suffix_off = 8;
		while (v >= 1e3 && suffix_off < suffix_len) {
			v /= 1e3;
			suffix_off++;
		}
		while (v < 1.0 && suffix_off > 0) {
			v /= 1e-3;
			suffix_off--;
		}
		os << v << suffix[suffix_off];
	}
private:
	double value;
};

ostream &operator<<(ostream &os, const engrfmt &fmt) {
	fmt.Print(os);
	return os;
}

std::vector<double> msgpack_serialize_time;
std::vector<double> msgpack_deserialize_time;
std::vector<double> json_serialize_time;
std::vector<double> json_deserialize_time;

int DoTest(Variant v) {
	stringstream ss_msgpack;
	//cout << "Testing...\n" << v << endl;
	double start = getTime();
	Serialize(ss_msgpack.rdbuf(), v, SERIALIZE_MSGPACK);
	msgpack_serialize_time.push_back(getTime() - start);
	start = getTime();
	Variant v2 = DeserializeFile(ss_msgpack.rdbuf(), SERIALIZE_MSGPACK);
	msgpack_deserialize_time.push_back(getTime() - start);
	if (v2 != v) {
		cout << "Msgpack: Error not equal:\n";
		cout << "Expected: " << v << endl << "Got: " << v2 << endl;
		return 1;
	}
	stringstream ss_json;
	start = getTime();
	Serialize(ss_json.rdbuf(), v, SERIALIZE_JSON);
	json_serialize_time.push_back(getTime() - start);
	start = getTime();
	v2 = DeserializeFile(ss_json.rdbuf(), SERIALIZE_JSON);
	json_deserialize_time.push_back(getTime() - start);
	if (v2 != v) {
		cout << "JSON: Error not equal:\n";
		cout << "Expected: " << v << endl << "Got: " << v2 << endl;
		return 1;
	}
	return 0;
}

int main(int argc, char **argv) {
	int iterations = 100;
	if (argc > 1) {
		iterations = 1;
		Variant v = DeserializeGuessFile(argv[1]);
		if (DoTest(v)) { return 1; }
	} else {
		for (int i = 0; i < iterations; ++i) {
			Variant v = GenerateRandomVariant(false);
			if (DoTest(v)) { return 1; }
		}
	}
	double mp_s_avg = 0, mp_d_avg = 0, j_s_avg = 0, j_d_avg = 0;
	for (int i = 0; i < iterations; ++i) {
		mp_s_avg += msgpack_serialize_time[i];
		mp_d_avg += msgpack_deserialize_time[i];
		j_s_avg += json_serialize_time[i];
		j_d_avg += json_deserialize_time[i];
	}
	mp_s_avg /= iterations;
	mp_d_avg /= iterations;
	j_s_avg /= iterations;
	j_d_avg /= iterations;
	cout << "msgpack serialize avg time: " << engrfmt(mp_s_avg) << endl;
	cout << "msgpack deserialize avg time: " << engrfmt(mp_d_avg) << endl;
	cout << "msgpack avg time: " << engrfmt(mp_s_avg + mp_d_avg) << endl;
	cout << "json serialize avg time: " << engrfmt(j_s_avg) << endl;
	cout << "json deserialize avg time: " << engrfmt(j_d_avg) << endl;
	cout << "json avg time: " << engrfmt(j_s_avg + j_d_avg) << endl;
	cout << "serialize msgpack/json: " << (mp_s_avg/j_s_avg) << "\tjson/msgpack: " << (j_s_avg/mp_s_avg) << endl;
	cout << "deserialize msgpack/json: " << (mp_d_avg/j_d_avg) << "\tjson/msgpack: " << (j_d_avg/mp_d_avg) << endl;
	cout << "msgpack/json: " << ((mp_s_avg+mp_d_avg)/(j_s_avg+j_d_avg)) << "\tjson/msgpack: " << (j_s_avg+j_d_avg)/(mp_s_avg+mp_d_avg) << endl;
	return 0;
}
