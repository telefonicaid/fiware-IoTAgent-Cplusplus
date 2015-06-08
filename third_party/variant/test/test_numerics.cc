/** \file
 * \author John Bridgman
 * \brief 
 */
#include "TestAssert.h"
#include <Variant/Variant.h>
#include <iostream>
#include <sstream>
#include <cxxabi.h>
#include <limits>
#include <stdlib.h>
#include <math.h>
using namespace libvariant;
using namespace std;

ostream &operator<<(ostream &os, const type_info &t) {
	const char *name = t.name();
	int status = -1;
	char *dem = 0;
	dem = abi::__cxa_demangle(name, 0, 0, &status);
	if (status == 0) {
		os << dem;
		free(dem);
	} else {
		os << name;
	}
	return os;
}

ostream &operator<<(ostream &os, const Variant &v) {
	Serialize(os.rdbuf(), v, SERIALIZE_JSON);
	return os;
}

template<typename T>
void assignCheck(T check)
{
	T a = check;
	Variant b = a;
	a = variant_cast<T>(b);
	if (a != check) {
		ostringstream oss;
		oss << "Assignment of " << check << " with type " << typeid(T) << " returned " << a;
		throw runtime_error(oss.str());
	}
	cout << ".";
}

template<typename T>
T my_modulus(T r, T l) { return r % l; }
template<> float my_modulus<float>(float r, float l) { return fmod(r, l); }
template<> double my_modulus<double>(double r, double l) { return fmod(r, l); }
template<> long double my_modulus<long double>(long double r, long double l) { return fmod(r, l); }

template<typename T>
void TestOperations(Variant::Type_t type) {
	Variant val = Variant();
	ASSERT(val.IsNull());
	for (T i = 0; i < 1000; i += 10) {
		val = Variant(i);
		ASSERT(val.As<T>() == i);
		ASSERT(val.GetType() == type);
	}

	Variant zero = T(0);
	Variant one = T(1);
	Variant two = T(2);
	ASSERT(one + one == two);
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	ASSERT(two - one == one);
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	ASSERT(one - one == zero);
	ASSERT(zero == T(0));
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	ASSERT(zero == zero);
	ASSERT(one / two == T(1)/T(2));
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	ASSERT(two * two == T(2) * T(2));
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	ASSERT((two + one) % two == my_modulus((T(2) + T(1)), T(2)));
	ASSERT(one == T(1));
	ASSERT(two == T(2));
	// Test doesn't make sense if unsigned
	if (!two.IsUnsigned()) {
		ASSERT(-two == -T(2));
	}
	ASSERT(two == T(2));
	val = two;
	ASSERT(one == --val);
	ASSERT(one == val);
	val = two;
	ASSERT(two == val--);
	ASSERT(one == val);
	val = one;
	ASSERT(two == ++val);
	ASSERT(two == val);
	val = one;
	ASSERT(one == val++);
	ASSERT(two == val);
}


int main(int argc, char **argv) {
	assignCheck<bool>(true);
	assignCheck<bool>(false);
	// max integer shall be preserved
	assignCheck<intmax_t>(numeric_limits<intmax_t>::max());
	// min integer shall be preserved
	assignCheck<intmax_t>(numeric_limits<intmax_t>::min());	
	// max unsigned integer shall be preserved
	assignCheck<uintmax_t>(numeric_limits<uintmax_t>::max());

	
	// float precision shall be preserved
	float ff[] = { __FLT_EPSILON__, __FLT_MIN__, __FLT_MAX__ };
	float f;
	for (uint32_t i=0; i<sizeof(ff)/sizeof(float); i++) {
		Variant v = ff[i];
		f = v.As<float>();
		if (f!=ff[i]) {
			ostringstream oss;
			oss << "Float precision test" << v.As<float>() << " != " << ff[i];
			throw runtime_error(oss.str());
		}
	}

	// double precision shall be preserved
	double dd[] = { __DBL_EPSILON__, __DBL_MIN__, __DBL_MAX__ };
	double d;
	for (uint32_t i=0; i<sizeof(dd)/sizeof(double); i++) {
		Variant v = dd[i];
		d = v.AsDouble();
		if (d!=dd[i]) {
			ostringstream oss;
			oss << "Double precision test" << v.As<double>() << " != " << dd[i];
			throw runtime_error(oss.str());
		}
	}

	// long double precision shall be preserved
	long double ldd[] = { __LDBL_EPSILON__, __LDBL_MIN__, __LDBL_MAX__ };
	long double ld;
	for (uint32_t i=0; i<sizeof(dd)/sizeof(long double); i++) {
		Variant v = ldd[i];
		ld = v.AsLongDouble();
		if (ld!=ldd[i]) {
			ostringstream oss;
			oss << "Long double precision test" << v.As<long double>() << " != " << dd[i];
			throw runtime_error(oss.str());
		}
	}

	cout << endl;
	cout << "Math tests." << endl;
	Variant v = 5;
	Variant o = v + 5;
	cout << o << endl;
	ASSERT(o.AsInt() == 10);
	o = v - 2;
	cout << o << endl;
	ASSERT(o.AsInt() == 3);
	o = 7;
	o /= 3;
	cout << o << endl;
	ASSERT(o.AsInt() == 2);
	ASSERT(o.IsInt());
	v = 0.1;
	cout << o << "/" << v << " = ";
	o = o / v;
	cout << o << endl;
	ASSERT(o.IsFloat());
	o %= 9;
	cout << o << endl;
	o = 2;
	v = o++;
	cout << o << " " << v << endl;
	ASSERT(v.AsInt() == 2);
	ASSERT(o.AsInt() == 3);
	--o;
	v = o--;
	ASSERT(v.AsInt() == 2);
	ASSERT(o.AsInt() == 1);
	Variant::Type_t extlist[] = { Variant::NullType, Variant::MapType, Variant::ListType,
	   	Variant::StringType, Variant::BlobType, Variant::BoolType };

	cout << "Verify that numerics throw on non-numeric types." << endl;
	for (uint32_t i=0; i<sizeof(extlist)/sizeof(Variant::Type_t); i++) {
		try {
			v = extlist[i];
			v++;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v += 2;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v -= 1;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v /= 2;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v *= 3;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v %= 2;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			-v;
			abort();
		} catch (const runtime_error &e) { }
		try {
			v = extlist[i];
			v--;
			abort();
		} catch (const runtime_error &e) { }
	}
	cout << "Verify that numerics work on all numeric types." << endl;
	Variant::Type_t tlist[] = { Variant::UnsignedType, Variant::IntegerType, Variant::FloatType };
	for (uint32_t i=0; i<sizeof(tlist)/sizeof(Variant::Type_t); i++) {
		v = tlist[i];
		v++;
		v += 2;
		v -= 1;
		v /= 2;
		v *= 3;
		v %= 2;
		v--;
	}
	for (uint32_t i=0; i<sizeof(tlist)/sizeof(Variant::Type_t); i++) {
		v = tlist[i];
		VariantRef ref = v;
		ref++;
		ref += 2;
		ref -= 1;
		ref /= 2;
		ref *= 3;
		ref %= 2;
		ref--;
	}

	cout << "Verifying numeric operations." << endl;
	TestOperations<int>(Variant::IntegerType);
	TestOperations<unsigned>(Variant::UnsignedType);
	TestOperations<long>(Variant::IntegerType);
	TestOperations<unsigned long>(Variant::UnsignedType);
	TestOperations<long long>(Variant::IntegerType);
	TestOperations<unsigned long long>(Variant::UnsignedType);
	TestOperations<float>(Variant::FloatType);
	TestOperations<double>(Variant::FloatType);
	TestOperations<long double>(Variant::FloatType);

	cout << "Verifying numeric up convert." << endl;

	// T + T == T
	{
		Variant one(int(1));
		Variant two = one + one;
		cout << one.GetType() << " + " << one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2);
		ASSERT(two.IsInt());
	}
	{
		Variant one(unsigned(1));
		Variant two = one + one;
		cout << one.GetType() << " + " << one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2u);
		ASSERT(two.IsUnsigned());
	}
	{
		Variant one(double(1));
		Variant two = one + one;
		cout << one.GetType() << " + " << one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2.0);
		ASSERT(two.IsFloat());
	}
	// anything + float == float
	{
		Variant i_one(int(1));
		Variant f_one(float(1));
		Variant two = i_one + f_one;
		cout << i_one.GetType() << " + " << f_one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2.0);
		ASSERT(two.IsFloat());
	}
	{
		Variant u_one(unsigned(1));
		Variant f_one(float(1));
		Variant two = u_one + f_one;
		cout << u_one.GetType() << " + " << f_one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2.0);
		ASSERT(two.IsFloat());
	}

	// int + unsigned == unsigned
	{
		Variant u_one(unsigned(1));
		Variant i_one(int(1));
		Variant two = u_one + i_one;
		cout << u_one.GetType() << " + " << i_one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2u);
		ASSERT(two.IsUnsigned());
		two = i_one + u_one;
		cout << i_one.GetType() << " + " << u_one.GetType() << " = " << two.GetType() << endl;
		ASSERT(two == 2u);
		ASSERT(two.IsUnsigned());
	}

	cout << "PASS" << endl;
	return 0;
}
