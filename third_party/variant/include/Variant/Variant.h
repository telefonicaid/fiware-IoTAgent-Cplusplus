// vim: foldmethod=marker
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
 *
 * \brief Main implementation file for Variant and related functions
 */


#ifndef VARIANT_VARIANT_H
#define VARIANT_VARIANT_H
#pragma once
#include <Variant/Blob.h>
#include <Variant/SharedPtr.h>
#include <Variant/Exceptions.h>
#include <Variant/VariantInternal.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <stddef.h>
#include <map>
#include <iterator>
#include <complex>

namespace libvariant {

	/** \brief The Variant class.
	 * This is a variant type inspired by types from several different
	 * scripting languages.
	 *
	 * \author John Bridgman
	 *
	 * This class gives the power and convenience of a scripting language variable
	 * to C++ with all the same /downsides/ that such a variable has as well.  The
	 * main idea is that things should be simple, lists are behind the scenes an
	 * std::vector and maps are an std::map.
	 *
	 * A Variant may be null, a number, a string, a boolean, a reference to a list,
	 * or a reference to a map. This means that const Variant of a map is not a
	 * const map but a const reference.  Lists and maps are reference counted.  If
	 * one wants to copy use the Copy member function.
	 *
	 * If you need a reference to a Variant to assign to it or change its type,
	 * use VariantRef.
	 * A const Variant & should be used for an immutable reference.
	 * E.g., void foo(VariantRef inout_param, const Variant &in_param);
	 * That is to say, if you think you want Variant &, you don't, you want
	 * VariantRef.
	 * The library is written so that:
	 * Variant v;
	 * void bar(Variant &);
	 * bar(v["foo"]);
	 * is a compile error.
	 * But
	 * Variant v;
	 * void moo(const Variant &);
	 * moo(v["moo"]);
	 * is correct.
	 *
	 *
	 * Note that this implementation will let you create a back reference (i.e., a
	 * map with itself as a member) but unless the user explicitly breaks it, it
	 * will result in a memory leak.  Checking for this case was deemed too
	 * expensive.
	 *
	 * A Variant will only change its own type if the type is null. That is to say
	 * that a null Variant that gets treated as a map will become a map. But, a
	 * Variant of any other type will never automatically convert.
	 *
	 */
	class Variant : protected Internal::Data, public VariantDefines {
	public:

		typedef VariantRefImpl<Variant> VariantRef;

		typedef std::vector<Variant> List;
		typedef std::map<std::string, Variant> Map;

		typedef Map::iterator MapIterator;
		typedef Map::const_iterator ConstMapIterator;
		typedef List::iterator ListIterator;
		typedef List::const_iterator ConstListIterator;

		Variant() {}
		Variant(VariantDefines::Type_t type) { Assign(type); }
		Variant(bool v) { Assign(v); }

		Variant(int v) { Assign(v); }
		Variant(unsigned v) { Assign(v); }
		Variant(long v) { Assign(v); }
		Variant(unsigned long v) { Assign(v); }
		Variant(long long v) { Assign(v); }
		Variant(unsigned long long v) { Assign(v); }
		Variant(double v) { Assign(v); }
		Variant(long double v) { Assign(v); }

		Variant(BlobPtr b) { Assign(b); }

		Variant(const std::string &v) { Assign(v); }
		Variant(const char *v) { Assign(v); }
		Variant(const Variant &v) { Assign(v); }
		Variant(const VariantRefImpl<Variant> &v) { Assign(v); }

		/// Construct a list from a std::vector.
		template<typename T>
		Variant(const std::vector<T> &v) { Assign(v); }

		/// Construct a map from a std::map.
		template<typename T>
		Variant(const std::map<std::string, T> &v) { Assign(v); }

		~Variant();

		Variant &operator=(const Variant &o) { Assign(o); return *this; }

		Variant &operator=(VariantDefines::Type_t v) { Assign(v); return *this; }
		Variant &operator=(bool v) { Assign(v); return *this; }
		Variant &operator=(int v) { Assign(v); return *this; }
		Variant &operator=(unsigned v) { Assign(v); return *this; }
		Variant &operator=(long v) { Assign(v); return *this; }
		Variant &operator=(unsigned long v) { Assign(v); return *this; }
		Variant &operator=(long long v) { Assign(v); return *this; }
		Variant &operator=(unsigned long long v) { Assign(v); return *this; }
		Variant &operator=(double v) { Assign(v); return *this; }
		Variant &operator=(long double v) { Assign(v); return *this; }
		Variant &operator=(BlobPtr v) { Assign(v); return *this; }
		Variant &operator=(const std::string & v) { Assign(v); return *this; }
		Variant &operator=(const char * v) { Assign(v); return *this; }

		template<typename T>
		Variant &operator=(const std::vector<T> &v) { Assign(v); return *this; }
		template<typename T>
		Variant &operator=(const std::map<std::string, T> &v) { Assign(v); return *this; }

		// Type query functions

		Type_t GetType() const; ///< Return the type of this Variant

		bool IsNull() const { return NullType == GetType(); }
		bool IsBool() const { return BoolType == GetType(); }
		bool IsTrue() const { return (BoolType == GetType()) && AsBool(); }
		bool IsFalse() const { return (BoolType == GetType()) && !AsBool(); }
		bool IsNumber() const
	   	{ return IntegerType == GetType() || UnsignedType == GetType() || FloatType == GetType(); }
		bool IsInt() const { return IntegerType == GetType(); }
		bool IsUnsigned() const { return UnsignedType == GetType(); }
		bool IsFloat() const { return FloatType == GetType(); }
		bool IsString() const { return StringType == GetType(); }
		bool IsList() const { return ListType == GetType(); }
		bool IsMap() const { return MapType == GetType(); }
		bool IsBlob() const { return BlobType == GetType(); }


		// Accessor functions

		bool AsBool() const; ///< Return the value as a bool
		long double AsLongDouble() const; ///< Return the value as a long double
		double AsDouble() const { return (double)AsLongDouble(); }
		uintmax_t AsUnsigned() const; ///< Return the value as an unsigned
		intmax_t AsInt() const; ///< Return the value as an integer
		template<typename T>
		T AsNumber() const { return static_cast<T>(AsLongDouble()); }
		std::string AsString() const; ///< Return the value as a string
		BlobPtr AsBlob();
		ConstBlobPtr AsBlob() const;

		template<typename T>
		T As() const;

		/// \brief If the type is string, list, map or blob returns its size,
		/// otherwise throws an exception
		unsigned Size() const;

		/// \brief If the type is string, list or map return if size == 0 otherwise
		/// throw an exception
		bool Empty() const;

		/// \brief If the type is string, list or map, empty the container otherwise
		/// set type to NullType
		void Clear();

		// List specific accessors
		List &AsList();
		const List &AsList() const;
		ListIterator ListBegin() ///< Equivalent to AsList().begin()
		{ return AsList().begin(); }
		ConstListIterator ListBegin() const ///< Equivalent to AsList().begin()
		{ return AsList().begin(); }
		ListIterator ListEnd() ///< Equivalent to AsList().end()
		{ return AsList().end(); }
		ConstListIterator ListEnd() const ///< Equivalent to AsList().end()
		{ return AsList().end(); }
		VariantRef At(unsigned i); //< Return a proxy/reference to the element at index i
		VariantRef At(unsigned i, Variant def); //< sets to def if doesn't exist
		const Variant &At(unsigned i) const;
		Variant Get(unsigned i) const; //< Return the element at index i or throw
		Variant Get(unsigned i, Variant def) const; //< Return the element at index i or return def
		Variant &Set(unsigned i, Variant v); //< Set the element at index i to v
		/// \brief Only valid when of type ListType Returns *this, so you can
		/// do Variant().Append(v1).Append(v2)...
		Variant &Append(Variant value);

		/// \brief If list return true if list is at least i in size, if map then convert i
		/// to a string, otherwise throw
		bool Contains(unsigned i) const;
		/// \brief Return the index hat v is at.
		/// If v is not contained in the list it return this->Size()
		unsigned Index(Variant v) const;

		/// \brief If list set index i to null, if index at end, reduce size by 1
		/// throw if not a list
		void Erase(unsigned i);

		// Map specific accessors
		Map &AsMap();
		const Map &AsMap() const;
		MapIterator MapBegin() ///< Equivalent to AsMap().begin()
		{ return AsMap().begin(); }
		ConstMapIterator MapBegin() const ///< Equivalent to AsMap().begin()
		{ return AsMap().begin(); }
		MapIterator MapEnd() ///< Equivalent to AsMap().end()
		{ return AsMap().end(); }
		ConstMapIterator MapEnd() const ///< Equivalent to AsMap().end()
		{ return AsMap().end(); }
		VariantRef At(const std::string &s); // Return a proxy/reference to the element at key s
		VariantRef At(const std::string &s, Variant def); //< Sets to def if doesn't exist and return a reference to it
		const Variant &At(const std::string &s) const;

		/// \brief If s in *this: return this->At(s); else throw;
		Variant Get(const std::string &s) const;
		/// \brief If s in *this: return this->At(s); else return def
		///  Only works if type is map
		Variant Get(const std::string &s, Variant def) const;

		/// \brief Set the key s to v
		// returns *this
		Variant &Set(const std::string &s, Variant v);

		/// \brief If map return true if s is a key in the map, if list then convert
		/// s to an unsigned and the above Contains, otherwise throw
		bool Contains(const std::string &s) const;
		/// \brief Erase a key in a map, throws if type is not MapType
		void Erase(const std::string &key);

		VariantRef operator[](int i) { return At(i); }
		const Variant &operator[](int i) const { return At(i); }

		VariantRef operator[](unsigned i) { return At(i); }
		const Variant &operator[](unsigned i) const { return At(i); }

		VariantRef operator[](const std::string &s) { return At(s); }
		const Variant &operator[](const std::string &s) const { return At(s); }

		// Path related accessors
		// These are the same as the non-path functions but take a path instead.
		VariantRef AtPath(const Path &path);
		VariantRef AtPath(const std::string &path) { return AtPath(ParsePath(path)); }
		VariantRef AtPath(const Path &path, Variant def);
		VariantRef AtPath(const std::string &path, Variant def) { return AtPath(ParsePath(path), def); }
		const Variant &AtPath(const Path &path) const;
		const Variant &AtPath(const std::string &path) const { return AtPath(ParsePath(path)); }

		/// \brief Like Variant::Get but it takes a path instead.
		Variant GetPath(const Path &path) const;
		Variant GetPath(const std::string &path) const { return GetPath(ParsePath(path)); }
		/// \brief Like Variant::Get but it takes a path instead.  The path is
		/// very like xpath, /key[index]/key/key. See Variant/Path.h
		Variant GetPath(const Path &path, Variant def) const;
		Variant GetPath(const std::string &path, Variant def) const { return GetPath(ParsePath(path), def); }

		/// \brief Set the value at the path, creating elements if they do not
		/// exist.  throws if an intermediate path element exists but is not a
		/// map or list.  returns *this
		Variant &SetPath(const Path &path, Variant val);
		Variant &SetPath(const std::string &path, Variant val) { return SetPath(ParsePath(path), val); }

		/// \brief Erase a value at a path, if remove_empty is false only erase the very end
		/// otherwise erase empty lists and maps in the path. returns *this
		Variant &ErasePath(const Path &path, bool remove_empty=false);
		Variant &ErasePath(const std::string &path, bool remove_empty=false)
	   	{ return ErasePath(ParsePath(path), remove_empty); }

		/// \brief Tester functions for paths, returns true if path is valid and exists.
		bool HasPath(const Path &path) const;
		bool HasPath(const std::string &path) const { return HasPath(ParsePath(path)); }

		/// \brief Just like Variant::Get except a reference to the value to
		/// set is passed in and Variant does the casting.
		template<typename T>
		void GetInto(T &lvalue, const std::string &s) const;

		/// \brief An into version of GetPath
		template<typename T>
		void GetPathInto(T &lvalue, const Path &path) const;
		template<typename T>
		void GetPathInto(T &lvalue, const std::string &path) const
		{ GetPathInto(lvalue, ParsePath(path)); }

		/// \brief Just like Variant::Get except a reference to the value to
		/// set is passed in and Variant does the casting.
		template<typename T>
		void GetInto(T &lvalue, const std::string &s, const T &def) const;

		/// \brief An into version of GetPath
		template<typename T>
		void GetPathInto(T &lvalue, const Path &path, const T &def) const;
		template<typename T>
		void GetPathInto(T &lvalue, const std::string &path, const T &def) const
		{ GetPathInto(lvalue, ParsePath(path), def); }

		/// \brief This type and other type must be MapType.
		///  for (k, v) in other: this[k] = v
		void Merge(Variant other);

		/// \brief Comparison operator
		///  Returns -1 if we are less than other
		///  0 if we are equal
		///  1 if we are greater than
		///  This function always returns 1 if Comparable returns false
		///  This compare is not completely consistent
		///  i.e. it can be such that two Variants are
		///  both < and > (ex map and list types)
		int Compare(const Variant &other) const;

		/// \brief Return true if this and other are comparable. Two types are
		/// considered comparable if they have the same type or if they are both
		/// number types.
		bool Comparable(const Variant &other) const;

		void Incr();
		void Decr();
		void Add(const Variant &o);
		void Sub(const Variant &o);
		void Mul(const Variant &o);
		void Div(const Variant &o);
		void Rem(const Variant &o);

		Variant Neg() const;

		Variant &operator++() { Incr(); return *this; }
		Variant &operator--() { Decr(); return *this; }
		Variant operator++(int) { Variant tmp = *this; ++(*this); return tmp; }
		Variant operator--(int) { Variant tmp = *this; --(*this); return tmp; }
		Variant operator-() const { return Neg(); }
		Variant operator+() const { return *this; }

		Variant &operator+=(const Variant &o) { Add(o); return *this; }
		Variant &operator-=(const Variant &o) { Sub(o); return *this; }
		Variant &operator*=(const Variant &o) { Mul(o); return *this; }
		Variant &operator/=(const Variant &o) { Div(o); return *this; }
		Variant &operator%=(const Variant &o) { Rem(o); return *this; }

		VariantRef Ref(); //< Return a new VariantRef to this Variant
		Variant Copy() const; //< Return a copy of this Variant
		Variant &Resolve(); //< Return *this or return the Variant this is a reference to
		const Variant &Resolve() const;
		/// Return *this or Variant referenced to if exists, otherwise create it with def.
		Variant &Resolve(const Variant &def);
		/// Return weather or not Resolve() const would throw.
		bool Exists() const;

		/// Make this equal other.
		void Assign(const Variant &other);

		void Assign(VariantDefines::Type_t type);
		void Assign(bool v);
		void Assign(int v);
		void Assign(unsigned v);
		void Assign(long v);
		void Assign(unsigned long v);
		void Assign(long long v);
		void Assign(unsigned long long v);
		void Assign(double v);
		void Assign(long double v);
		void Assign(BlobPtr b);
		void Assign(const std::string &v);
		void Assign(const char *v);

		template<typename T>
		void Assign(const std::vector<T> &v) {
			Assign(VariantDefines::ListType);
			AsList().assign(v.begin(), v.end());
		}

		template<typename T>
		void Assign(const std::map<std::string, T> &v) {
			Assign(VariantDefines::MapType);
			AsMap().insert(v.begin(), v.end());
		}

	protected:
		Variant(const RefTag &, const Variant &o);
		Variant(const RefTag &, Variant &o, Path::const_iterator b, Path::const_iterator e);
		/// Reassing this reference, only available through the VariantRef interface.
		/// The regular Assign makes what this is refering to equal the new value, this
		/// function makes this reference refer to the new value.
		void ReassignRef(const Variant &o);
		friend class Internal::VTable;
	};

	typedef VariantRefImpl<Variant> VariantRef;

	inline bool operator==(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) == 0; }
	inline bool operator!=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) != 0; }
	inline bool operator<(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) < 0; }
	inline bool operator>(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) > 0; }
	inline bool operator<=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) <= 0; }
	inline bool operator>=(const Variant &lhs, const Variant &rhs) { return lhs.Compare(rhs) >= 0; }

	inline Variant operator+(const Variant &lhs, const Variant &rhs) {
		Variant ret = lhs;
		return ret += rhs;
   	}
	inline Variant operator-(const Variant &lhs, const Variant &rhs) {
		Variant ret = lhs;
		return ret -= rhs;
   	}
	inline Variant operator*(const Variant &lhs, const Variant &rhs) {
		Variant ret = lhs;
		return ret *= rhs;
	}
	inline Variant operator/(const Variant &lhs, const Variant &rhs) {
		Variant ret = lhs;
		return ret /= rhs;
	}
	inline Variant operator%(const Variant &lhs, const Variant &rhs) {
		Variant ret = lhs;
		return ret %= rhs;
	}

	/// \defgroup variant_cast Template casting operators.
	/// These are for use inside templates to extract the value of a Variant into
	/// a templated type.
	/// @{
	
	template<typename T>
	struct VariantCaster {
		static T Cast(const Variant &v);
	};

	// Default
	template<typename T> inline T VariantCaster<T>::Cast(const Variant &v) { return T(v); }

	template<typename T>
	inline T variant_cast(const Variant &v) { return VariantCaster<T>::Cast(v); }

	// Provide specializations for all the basic types
	template<> inline bool VariantCaster<bool>::Cast(const Variant &v) { return v.AsBool(); }
	template<> inline char VariantCaster<char>::Cast(const Variant &v) { return (char)v.AsInt(); }
	template<> inline unsigned char VariantCaster<unsigned char>::Cast(const Variant &v) { return (unsigned char)v.AsUnsigned(); }
	template<> inline signed char VariantCaster<signed char>::Cast(const Variant &v) { return (signed char)v.AsUnsigned(); }
	template<> inline short VariantCaster<short>::Cast(const Variant &v) { return (short)v.AsInt(); }
	template<> inline unsigned short VariantCaster<unsigned short>::Cast(const Variant &v) { return (unsigned short)v.AsUnsigned(); }
	template<> inline int VariantCaster<int>::Cast(const Variant &v) { return (int)v.AsInt(); }
	template<> inline unsigned VariantCaster<unsigned>::Cast(const Variant &v) { return (unsigned)v.AsUnsigned(); }
	template<> inline long VariantCaster<long>::Cast(const Variant &v) { return (long)v.AsInt(); }
	template<> inline unsigned long VariantCaster<unsigned long>::Cast(const Variant &v) { return (unsigned long)v.AsUnsigned(); }
	template<> inline long long VariantCaster<long long>::Cast(const Variant &v) { return (long long)v.AsInt(); }
	template<> inline unsigned long long VariantCaster<unsigned long long>::Cast(const Variant &v) { return (unsigned long long)v.AsUnsigned(); }
	template<> inline float VariantCaster<float>::Cast(const Variant &v) { return (float)v.AsDouble(); }
	template<> inline double VariantCaster<double>::Cast(const Variant &v) { return v.AsDouble(); }
	template<> inline long double VariantCaster<long double>::Cast(const Variant &v) { return v.AsLongDouble(); }
	template<> inline std::string VariantCaster<std::string>::Cast(const Variant &v) { return v.AsString(); }

	// Allow things like variant_cast< std::vector<int> >(v) to work
	template<typename T>
	struct VariantCaster< std::vector<T> > {
		static std::vector<T> Cast(const Variant &v);
	};

	template<typename T>
	inline std::vector<T> VariantCaster< std::vector<T> >::Cast(const Variant &v) {
		std::vector<T> ret;
		for (Variant::ConstListIterator i(v.ListBegin()), e(v.ListEnd()); i != e; ++i) {
			ret.push_back( variant_cast<T>( *i ) );
		}
		return ret;
	}

	template<> inline Variant::List VariantCaster<Variant::List>::Cast(const Variant &v) { return v.AsList(); }

	// allow things like variant_cast< std::map<std::string, std::string> >(v)
	// to work
	template<typename T>
	struct VariantCaster< std::map<std::string, T> > {
		static std::map<std::string, T> Cast(const Variant &v);
	};

	template<typename T>
	inline std::map<std::string, T> VariantCaster< std::map<std::string, T> >::Cast(const Variant &v) {
		std::map<std::string, T> ret;
		for (Variant::ConstMapIterator i(v.MapBegin()), e(v.MapEnd()); i != e; ++i) {
			ret.insert( std::make_pair( i->first, variant_cast<T>( i->second ) ) );
		}
		return ret;
	}

	template<> inline Variant::Map VariantCaster<Variant::Map>::Cast(const Variant &v) { return v.AsMap(); }

	template<typename T>
	struct VariantCaster< std::complex<T> > {
		static std::complex<T> Cast(const Variant &v);
	};

	template<typename T>
	std::complex<T> VariantCaster< std::complex<T> >::Cast(const Variant &v) {
		if (v.IsNull()) {
			return std::complex<T>();
		} else if (v.IsNumber()) {
			return std::complex<T>(v.As<T>(), T());
		} else if (v.IsMap()) {
			T r = variant_cast<T>(v.Get("real", T()));
			T c = variant_cast<T>(v.Get("imag", T()));
			return std::complex<T>(r, c);
		} else if (v.IsList()) {
			if (v.Size() > 1) {
				return std::complex<T>(v[0].As<T>(), v[1].As<T>());
			} else if (v.Size() > 0) {
				return std::complex<T>(v[0].As<T>(), T());
			} else {
				return std::complex<T>();
			}
		}
		throw UnableToConvertError(v.GetType(), "std::complex<T>");
	}

	/// @}

	template<typename T>
	T Variant::As() const { return variant_cast<T>(*this); }

	template<typename T>
	void Variant::GetInto(T &lvalue, const std::string &s) const {
		lvalue = variant_cast<T>(Get(s));
	}

	template<typename T>
	void Variant::GetPathInto(T &lvalue, const Path &p) const {
		lvalue = variant_cast<T>(GetPath(p));
	}

	template<typename T>
	void Variant::GetInto(T &lvalue, const std::string &s, const T &def) const {
		if (Contains(s)) { lvalue = variant_cast<T>(Get(s)); }
		else { lvalue = def; }
	}

	template<typename T>
	void Variant::GetPathInto(T &lvalue, const Path &p, const T &def) const {
		lvalue = variant_cast<T>(GetPath(p, def));
	}

	class Parser;
	/// \brief Takes a parser and produces a Variant from it.
	Variant ParseVariant(Parser &p);

	class Emitter;
	/// Takes a Variant and Emitter and emits the Variant.
	Emitter &operator<<(Emitter &e, const Variant &v);

	/// \brief The different types of serialization supported.
	enum SerializeType {
		// Attempt to guess the format
		// Cannot be specified as an output format.
		SERIALIZE_GUESS = 0x0,

		// Built in text formats
		SERIALIZE_JSON = 0x00000001,
		SERIALIZE_YAML,
		SERIALIZE_XMLPLIST,
		SERIALIZE_BUNDLEHDR,

		// Built in binary formats
		SERIALIZE_MSGPACK = 0x000000FF,

	};


	/// \brief An iterator that iterates over all documents that the parser will generate
	class LoadAllIterator {
	public:
		typedef Variant value_type;
		typedef ptrdiff_t difference_type;
		typedef Variant* pointer;
		typedef Variant& reference;
		typedef std::forward_iterator_tag iterator_category;

		LoadAllIterator();
		explicit LoadAllIterator(Parser &parser);
		Variant &operator*() { return Get(); }
		Variant *operator->() { return &Get(); }
		Variant &Get();
		void Next();
		LoadAllIterator &operator++() {
			Next();
			return *this;
		}
		LoadAllIterator operator++(int) {
			LoadAllIterator copy = *this;
			Next();
			return copy;
		}
		bool operator==(const LoadAllIterator &that) const;
		bool operator!=(const LoadAllIterator &that) const { return !(*this == that); }
	private:
		class Impl;
		shared_ptr<Impl> impl;
		struct Value;
		shared_ptr<Value> value;
	};

	// Some convenience functions
	// A version is provided that will go to/from a string,
	// a memory buffer, a file name, an open FILE*, and
	// a streambuf* (usually acquired through iostream.rdbuf()).
	
	/// \brief Serialize a Variant to a string using format type.
	std::string Serialize(Variant v, SerializeType type, Variant params = Variant::NullType);
	/// \brief Serialize a Variant to a file using format type.
	void Serialize(const std::string &filename, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	/// \brief Serialize a Variant to a FILE pointer using format type.
	void Serialize(FILE *f, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	/// \brief Serialize a Variant to a streambuf* (iostrem.rdbuf()) using format type.
	void Serialize(std::streambuf *sb, Variant v, SerializeType type,
			Variant params = Variant::NullType);
	/// \brief Serialize a Variant to a memory buffer of length len, using format type
	//and return how many bytes produced.
	unsigned Serialize(void *ptr, unsigned len, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);

	// The basic deserializing functions

	/// \brief Attempt to deserialize a string in format type to a Variant
	Variant Deserialize(const std::string &str, SerializeType type);
	/// \brief Attempt to deserialize a null terminated string in format type to a Variant
	Variant Deserialize(const char *str, SerializeType type);
	/// \brief Attempt to deserialize a pointer and length in format type to a Variant
	Variant Deserialize(const void *ptr, unsigned len, SerializeType type);
	/// \brief Attempt to deserialize the file to a Variant in format type
	Variant DeserializeFile(const char *filename, SerializeType type);
	/// \brief Deserialize from a FILE pointer to a Variant in format type
	Variant DeserializeFile(FILE *f, SerializeType type);
	/// \brief Deserialize from a streambuf to a Variant in format type
	Variant DeserializeFile(std::streambuf *sb, SerializeType type);

	/// \defgroup deserialize_guess Guess format
	/// These functions attempt to guess the type of the input.  The guessing
	/// function only looks at the first non-whitespace character to make the
	/// decision. As such on sufficiently badly behaved input can select the
	/// wrong format.
	/// @{
	Variant DeserializeGuess(const std::string &str);
	Variant DeserializeGuess(const char *str);
	Variant DeserializeGuess(const void *ptr, unsigned len);
	Variant DeserializeGuessFile(const char *filename);
	Variant DeserializeGuessFile(FILE *f);
	Variant DeserializeGuessFile(std::streambuf *sb);
	/// @}

	/// \defgroup deserialize_all Deserialize all elements
	/// These functions return an iterator. These are for format types that
	/// support multiple documents in one stream. Otherwise same as Deserialize
	/// functions above.
	/// @{
	LoadAllIterator DeserializeAll(const std::string &str, SerializeType type);
	LoadAllIterator DeserializeAll(const char *str, SerializeType type);
	LoadAllIterator DeserializeAll(const void *ptr, unsigned len, SerializeType type);
	LoadAllIterator DeserializeAllFile(const char *filename, SerializeType type);
	/// File must remain open for as long as the iterator is referenced
	LoadAllIterator DeserializeAllFile(FILE *f, SerializeType type);
	/// Streambuf must exist for as long as the iterator is referenced
	LoadAllIterator DeserializeAllFile(std::streambuf *sb, SerializeType type);
	/// @}


	// Serialize and Deserializing JSON
	//
	inline std::string SerializeJSON(Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		return Serialize(v, SERIALIZE_JSON, param);
	}
	inline void SerializeJSON(const std::string &filename, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(filename, v, SERIALIZE_JSON, param);
	}
	inline void SerializeJSON(FILE *f, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(f, v, SERIALIZE_JSON, param);
	}
	inline void SerializeJSON(std::streambuf *sb, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(sb, v, SERIALIZE_JSON, param);
	}
	inline unsigned SerializeJSON(void *ptr, unsigned len, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		return Serialize(ptr, len, v, SERIALIZE_JSON, param);
	}
	inline Variant DeserializeJSON(const std::string &str) { return Deserialize(str, SERIALIZE_JSON); }
	inline Variant DeserializeJSON(const char *str) { return Deserialize(str, SERIALIZE_JSON); }
	inline Variant DeserializeJSON(const void *ptr, unsigned len) { return Deserialize(ptr, len, SERIALIZE_JSON); }
	inline Variant DeserializeJSONFile(const char *filename) { return DeserializeFile(filename, SERIALIZE_JSON); }
	inline Variant DeserializeJSONFile(FILE *f) { return DeserializeFile(f, SERIALIZE_JSON); }
	inline Variant DeserializeJSONFile(std::streambuf *sb) { return DeserializeFile(sb, SERIALIZE_JSON); }

	//Serialize and Deserialize YAML
	//
	inline std::string SerializeYAML(Variant v) { return Serialize(v, SERIALIZE_YAML); }
	inline void SerializeYAML(const std::string &filename, Variant v) { Serialize(filename, v, SERIALIZE_YAML); }
	inline void SerializeYAML(FILE *f, Variant v) { Serialize(f, v, SERIALIZE_YAML); }
	inline void SerializeYAML(std::streambuf *sb, Variant v) { Serialize(sb, v, SERIALIZE_YAML); }
	inline unsigned SerializeYAML(void *ptr, unsigned len, Variant v) { return Serialize(ptr, len, v, SERIALIZE_YAML); }

	inline Variant DeserializeYAML(const std::string &str) { return Deserialize(str, SERIALIZE_YAML); }
	inline Variant DeserializeYAML(const char *str) { return Deserialize(str, SERIALIZE_YAML); }
	inline Variant DeserializeYAML(const void *ptr, unsigned len) { return Deserialize(ptr, len, SERIALIZE_YAML); }
	inline Variant DeserializeYAMLFile(const char *filename) { return DeserializeFile(filename, SERIALIZE_YAML); }
	inline Variant DeserializeYAMLFile(FILE *f) { return DeserializeFile(f, SERIALIZE_YAML); }
	inline Variant DeserializeYAMLFile(std::streambuf *sb) { return DeserializeFile(sb, SERIALIZE_YAML); }

	inline LoadAllIterator DeserializeYAMLAll(const std::string &str) { return DeserializeAll(str, SERIALIZE_YAML); }
	inline LoadAllIterator DeserializeYAMLAll(const char *str) { return DeserializeAll(str, SERIALIZE_YAML); }
	inline LoadAllIterator DeserializeYAMLAll(const void *ptr, unsigned len) { return DeserializeAll(ptr, len, SERIALIZE_YAML); }
	inline LoadAllIterator DeserializeYAMLAllFile(const char *filename) { return DeserializeAllFile(filename, SERIALIZE_YAML); }
	inline LoadAllIterator DeserializeYAMLAllFile(FILE *f) { return DeserializeAllFile(f, SERIALIZE_YAML); }
	inline LoadAllIterator DeserializeYAMLAllFile(std::streambuf *sb) { return DeserializeAllFile(sb, SERIALIZE_YAML); }

	//Serialize and Deserialize XML plist
	//
	inline std::string SerializeXMLPLIST(Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		return Serialize(v, SERIALIZE_XMLPLIST, param);
	}
	inline void SerializeXMLPLIST(const std::string &filename, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(filename, v, SERIALIZE_XMLPLIST, param);
	}
	inline void SerializeXMLPLIST(FILE *f, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(f, v, SERIALIZE_XMLPLIST, param);
	}
	inline void SerializeXMLPLIST(std::streambuf *sb, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		Serialize(sb, v, SERIALIZE_XMLPLIST, param);
	}
	inline unsigned SerializeXMLPLIST(void *ptr, unsigned len, Variant v, bool pretty=false) {
		Variant param = Variant::MapType; param["pretty"] = pretty;
		return Serialize(ptr, len, v, SERIALIZE_XMLPLIST, param);
	}

	inline Variant DeserializeXMLPLIST(const std::string &str) { return Deserialize(str, SERIALIZE_XMLPLIST); }
	inline Variant DeserializeXMLPLIST(const char *str) { return Deserialize(str, SERIALIZE_XMLPLIST); }
	inline Variant DeserializeXMLPLIST(const void *ptr, unsigned len) { return Deserialize(ptr, len, SERIALIZE_XMLPLIST); }
	inline Variant DeserializeXMLPLISTFile(const char *filename) { return DeserializeFile(filename, SERIALIZE_XMLPLIST); }
	inline Variant DeserializeXMLPLISTFile(FILE *f) { return DeserializeFile(f, SERIALIZE_XMLPLIST); }
	inline Variant DeserializeXMLPLISTFile(std::streambuf *sb) { return DeserializeFile(sb, SERIALIZE_XMLPLIST); }

	//Serialize and Deserialize BundleHdr
	//
	// BundleHdr requires the root element to be of MapType and
	// no submaps exist.
	//
	// Note that these do not add a null byte to the end.
	// Use SerializeBundle for that.

	inline std::string SerializeBundleHdr(Variant v) {
		Variant param = Variant::MapType;
		return Serialize(v, SERIALIZE_BUNDLEHDR, param);
	}
	inline void SerializeBundleHdr(const std::string &filename, Variant v) {
		Variant param = Variant::MapType;
		Serialize(filename, v, SERIALIZE_BUNDLEHDR, param);
	}
	inline void SerializeBundleHdr(FILE *f, Variant v) {
		Variant param = Variant::MapType;
		Serialize(f, v, SERIALIZE_BUNDLEHDR, param);
	}
	inline void SerializeBundleHdr(std::streambuf *sb, Variant v) {
		Variant param = Variant::MapType;
		Serialize(sb, v, SERIALIZE_BUNDLEHDR, param);
	}
	inline unsigned SerializeBundleHdr(void *ptr, unsigned len, Variant v) {
		Variant param = Variant::MapType;
		return Serialize(ptr, len, v, SERIALIZE_BUNDLEHDR, param);
	}

	inline Variant DeserializeBundleHdr(const std::string &str) { return Deserialize(str, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleHdr(const char *str) { return Deserialize(str, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleHdr(const void *ptr, unsigned len) { return Deserialize(ptr, len, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleHdrFile(const char *filename) { return DeserializeFile(filename, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleHdrFile(FILE *f) { return DeserializeFile(f, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleHdrFile(std::streambuf *sb) { return DeserializeFile(sb, SERIALIZE_BUNDLEHDR); }

}

/// Debug function not inside the namespace to print out a variant (calls SerializeJSON)
void VariantPrint(const libvariant::Variant v);

#endif
