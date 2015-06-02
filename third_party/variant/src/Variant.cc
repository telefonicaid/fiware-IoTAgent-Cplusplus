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
 * Variant implementation
 */
#include <Variant/Variant.h>
#include "ParseBool.h"
#include <sstream>
#include <stdexcept>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <algorithm>

template<typename T>
class my_modulus {
public:
	T operator()(const T &lhs, const T &rhs) { return lhs % rhs; }
};

template<>
class my_modulus<long double> {
public:
	long double operator()(const long double &lhs, const long double &rhs) { return fmod(lhs, rhs); }
};

namespace libvariant {

	static inline Variant::Type_t NumericUpcast(Variant::Type_t t1, Variant::Type_t t2) {
		switch (t1) {
		case Variant::IntegerType:
			switch (t2) {
			case Variant::IntegerType:
				return Variant::IntegerType;
			case Variant::UnsignedType:
				return Variant::UnsignedType;
			case Variant::FloatType:
				return Variant::FloatType;
			case Variant::NullType:
			case Variant::BoolType:
			case Variant::StringType:
			case Variant::ListType:
			case Variant::MapType:
				throw UnableToConvertError(t2, "a numeric type.");
			default:
				throw UnknownTypeError(t2);
			}
		case Variant::UnsignedType:
			switch (t2) {
			case Variant::IntegerType:
				return Variant::UnsignedType;
			case Variant::UnsignedType:
				return Variant::UnsignedType;
			case Variant::FloatType:
				return Variant::FloatType;
			case Variant::NullType:
			case Variant::BoolType:
			case Variant::StringType:
			case Variant::ListType:
			case Variant::MapType:
				throw UnableToConvertError(t2, "a numeric type.");
			default:
				throw UnknownTypeError(t2);
			}
		case Variant::FloatType:
			switch (t2) {
			case Variant::IntegerType:
				return Variant::FloatType;
			case Variant::UnsignedType:
				return Variant::FloatType;
			case Variant::FloatType:
				return Variant::FloatType;
			case Variant::NullType:
			case Variant::BoolType:
			case Variant::StringType:
			case Variant::ListType:
			case Variant::MapType:
				throw UnableToConvertError(t2, "a numeric type.");
			default:
				throw UnknownTypeError(t2);
			}
		case Variant::NullType:
		case Variant::BoolType:
		case Variant::StringType:
		case Variant::ListType:
		case Variant::MapType:
			throw UnableToConvertError(t1, "a numeric type.");
		default:
			throw UnknownTypeError(t1);
		}
	}


	namespace Internal {

		struct RefData {
			RefData() : valid(true) {}
			bool valid;
		};

		struct Storage {
			Storage() {}

			virtual ~Storage() {}

		};

		struct MapStorage : public Storage {
			MapStorage(const Variant::Map &m) : map(m) {}
			Variant::Map map;
		};

		struct ListStorage : public Storage {
			ListStorage(const Variant::List &l) : list(l) {}
			Variant::List list;
		};

		struct StringStorage : public Storage {
			StringStorage(const std::string &s) : str(s) {}
			std::string str;
		};

		struct BlobStorage : public Storage {
			BlobStorage(BlobPtr b) : blob(b) {}
			BlobPtr blob;
		};

		struct RefStorage : public Storage {
			shared_ptr<Storage> storage;
		};

		struct ProxyStorage : public Storage {
			ProxyStorage(Data *ref_, Path::const_iterator b, Path::const_iterator e)
				: ref(ref_), storage(ref_->storage), path(b, e)
			{}
			Data *ref;
			shared_ptr<Storage> storage;
			Path path;
		};


		// Three ways to look up a value in a map or list:
		// Get a reference/proxy -> GetRef
		// Get pointer/throw -> Get checked=true
		// Get pointer/null -> Get checked=false

		class VTable {
		public:
			static inline Data *GetData(Variant *v) { return v; }
			static inline const Data *GetData(const Variant *v) { return v; }
			static inline Variant *GetVar(Data *d) { return static_cast<Variant*>(d); }
			static inline const Variant *GetVar(const Data *d) { return static_cast<const Variant*>(d); }

			virtual ~VTable() {}
			virtual void Copy(const Data *that, Data *other) const = 0;
			virtual void Assign(const Data *that, Data *other) const;
			virtual void MakeRef(const Data *that, Data *ref) const;
			virtual void MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const;
			virtual Variant::Type_t GetType(const Data *that) const = 0;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual BlobPtr AsBlob(Data *that) const;
			virtual ConstBlobPtr AsBlobConst(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::List &AsList(Data *that) const;
			virtual const Variant::List &AsListConst(const Data *that) const;
			virtual bool ContainsIndex(const Data *that, unsigned i) const;
			virtual unsigned Index(const Data *that, Variant v) const;
			virtual void EraseIndex(Data *that, unsigned i) const;
			virtual VariantRef GetRefIndex(Data *that, unsigned i, Variant *def) const;
			virtual const Variant *GetConstIndex(const Data *that, unsigned i, bool checked) const;
			virtual Variant *GetIndex(Data *that, unsigned i, bool checked) const;
			virtual void SetIndex(Data *that, unsigned i, const Data *other) const;
			virtual Variant::Map &AsMap(Data *that) const;
			virtual const Variant::Map &AsMapConst(const Data *that) const;
			virtual bool ContainsKey(const Data *that, const std::string &s) const;
			virtual void EraseKey(Data *that, const std::string &key) const;
			virtual VariantRef GetRefKey(Data *that, const std::string &s, Variant *def) const;
			virtual const Variant *GetConstKey(const Data *that, const std::string &s, bool checked) const;
			virtual Variant *GetKey(Data *that, const std::string &s, bool checked) const;
			virtual void SetKey(Data *that, const std::string &s, const Data *other) const;
			VariantRef GetPathRef(Data *that, Path::const_iterator b, Path::const_iterator e, Variant *def) const;
			const Variant *GetPathConst(const Data *that, Path::const_iterator b, Path::const_iterator e, bool checked) const;
			void SetPath(Data *that, Path::const_iterator b, Path::const_iterator e, const Variant *other) const;
			void ErasePath(Data *that, Path::const_iterator b, Path::const_iterator e, bool recursive) const;
			bool Comparable(const Data *that, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const = 0;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
			virtual void Add(Data *that, const Data *other) const;
			virtual void Sub(Data *that, const Data *other) const;
			virtual void Mul(Data *that, const Data *other) const;
			virtual void Div(Data *that, const Data *other) const;
			virtual void Rem(Data *that, const Data *other) const;
			shared_ptr<RefData> EnsureRef(Data *that) const;
			virtual void Destroy(Data *that) const;
			virtual void Destruct(Data *that) const;
			virtual Variant *Resolve(Data *that) const;
			virtual const Variant *ResolveConst(const Data *that) const;
			virtual Variant *ResolveDefault(Data *that, const Data *def) const;
			virtual bool Exists(const Data *that) const;
		};

		void DefaultInit(Data *that, VariantDefines::Type_t type);

		void NullInit(Data *that);

		void BoolInit(Data *that, bool b);

		void IntegerInit(Data *that, intmax_t i);

		void UnsignedInit(Data *that, uintmax_t u);

		void FloatInit(Data *that, long double f);

		void StringInit(Data *that, const std::string &s);

		void ListInit(Data *that, const Variant::List &l);

		void MapInit(Data *that, const Variant::Map &m);

		void BlobInit(Data *that, BlobPtr blob);

		void RefInit(Data *that, const Data *other);

		void ProxyInit(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e);

		//--------------------
		// VTable 
		//--------------------

		void VTable::Assign(const Data *that, Data *other) const
		{ that->vtable->Copy(that, VTable::GetData(other->vtable->Resolve(other))); }

		void VTable::MakeRef(const Data *that, Data *ref) const {
			RefInit(ref, that);
		}
		void VTable::MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const
		{ ProxyInit(ref, that, b, e); }

		bool VTable::AsBool(const Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a bool"); }

		long double VTable::AsLongDouble(const Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a float"); }

		uintmax_t VTable::AsUnsigned(const Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a unsigned"); }

		intmax_t VTable::AsInt(const Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a int"); }

		std::string VTable::AsString(const Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a string"); }

		BlobPtr VTable::AsBlob(Data *that) const
		{ throw UnexpectedTypeError(VariantDefines::BlobType, that->vtable->GetType(that)); }

		ConstBlobPtr VTable::AsBlobConst(const Data *that) const
		{ throw UnexpectedTypeError(VariantDefines::BlobType, that->vtable->GetType(that)); }

		unsigned VTable::Size(const Data *that) const
		{ throw InvalidOperationError(that->vtable->GetType(that), "Size"); }

		bool VTable::Empty(const Data *that) const
		{ throw InvalidOperationError(that->vtable->GetType(that), "Empty"); }

		void VTable::Clear(Data *that) const { NullInit(that); }

		Variant::List &VTable::AsList(Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a list"); }

		const Variant::List &VTable::AsListConst(const Data *that) const
		{ throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that)); }

		bool VTable::ContainsIndex(const Data *that, unsigned i) const
		{ return false; }

		unsigned VTable::Index(const Data *that, Variant o) const
		{ throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that)); }

		void VTable::EraseIndex(Data *that, unsigned i) const
		{ throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that)); }

		VariantRef VTable::GetRefIndex(Data *that, unsigned i, Variant *def) const
		{ throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that)); }

		const Variant *VTable::GetConstIndex(const Data *that, unsigned i, bool checked) const { 
			if (!checked) { return 0; }
			throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that));
		}

		Variant *VTable::GetIndex(Data *that, unsigned i, bool checked) const { 
			if (!checked) { return 0; }
			throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that));
		}

		void VTable::SetIndex(Data *that, unsigned i, const Data *other) const
		{ throw UnexpectedTypeError(VariantDefines::ListType, that->vtable->GetType(that)); }

		Variant::Map &VTable::AsMap(Data *that) const
		{ throw UnableToConvertError(that->vtable->GetType(that), "a map"); }

		const Variant::Map &VTable::AsMapConst(const Data *that) const
		{ throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that)); }

		bool VTable::ContainsKey(const Data *that, const std::string &s) const
		{ return false; }

		void VTable::EraseKey(Data *that, const std::string &key) const
		{ throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that)); }

		VariantRef VTable::GetRefKey(Data *that, const std::string &s, Variant *def) const
		{ throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that)); }

		const Variant *VTable::GetConstKey(const Data *that, const std::string &s, bool checked) const {
			if (!checked) { return 0; }
			throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that));
		}

		Variant *VTable::GetKey(Data *that, const std::string &s, bool checked) const {
			if (!checked) { return 0; }
			throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that));
		}

		void VTable::SetKey(Data *that, const std::string &s, const Data *other) const
		{ throw UnexpectedTypeError(VariantDefines::MapType, that->vtable->GetType(that)); }

		const Variant *GetPathElem(const Data *that, const PathElement &elem, bool checked) {
			if (elem.IsString()) {
				return that->vtable->GetConstKey(that, elem.AsString(), checked);
			} else /* if (elem.IsNumber()) */ {
				return that->vtable->GetConstIndex(that, elem.AsUnsigned(), checked);
			}
		}

		Variant *GetPathElem(Data *that, const PathElement &elem, bool checked) {
			if (elem.IsString()) {
				return that->vtable->GetKey(that, elem.AsString(), checked);
			} else /* if (elem.IsNumber()) */ {
				return that->vtable->GetIndex(that, elem.AsUnsigned(), checked);
			}
		}

		VariantRef GetRefPathElem(Data *that, const PathElement &elem, Variant *def) {
			if (elem.IsString()) {
				return that->vtable->GetRefKey(that, elem.AsString(), def);
			} else /* if (elem.IsNumber()) */ {
				return that->vtable->GetRefIndex(that, elem.AsUnsigned(), def);
			}
		}

		void SetPathElem(Data *that, const PathElement &elem, const Variant *other) {
			if (elem.IsString()) {
				that->vtable->SetKey(that, elem.AsString(), VTable::GetData(other));
			} else /* if (elem.IsNumber()) */ {
				that->vtable->SetIndex(that, elem.AsUnsigned(), VTable::GetData(other));
			}
		}

		VariantRef VTable::GetPathRef(Data *that, Path::const_iterator b,
				Path::const_iterator e, Variant *def) const
		{
			if (b == e) { return VariantRef(*GetVar(that)); }
			Variant *ref = 0;
			ref = GetPathElem(that, *b, false);
			if (!ref) {
				if (!def) {
					return VariantRef(*GetVar(that), b, e);
				}
				if (b + 1 == e) {
					return GetRefPathElem(that, *b,  def);
				} else {
					Variant null_value;
					SetPathElem(that, *b, &null_value);
					return that->vtable->GetPathRef(that, b, e, def);
				}
			}
			if (b + 1 == e) {
				return VariantRef(*ref);
			} else {
				Data *ref_data = VTable::GetData(ref);
				return ref_data->vtable->GetPathRef(ref_data, b + 1, e, def);
			}
		}

		const Variant *VTable::GetPathConst(const Data *that, Path::const_iterator b, Path::const_iterator e, bool checked) const
		{
			if (b == e) { return that->vtable->ResolveConst(that); }
			const Variant *ref = 0;
			ref = GetPathElem(that, *b, checked);
			if (!ref) {
				if (checked) {
					if (b->IsString()) {
						throw KeyError(b->AsString());
					} else {
						throw std::out_of_range("Variant::List index out of range.");
					}
				}
			   	return 0;
		   	}
			if (b + 1 == e) {
				return ref;
			} else {
				const Data *ref_data = VTable::GetData(ref);
				return ref_data->vtable->GetPathConst(ref_data, b + 1, e, checked);
			}
		}

		void VTable::SetPath(Data *that, Path::const_iterator b, Path::const_iterator e, const Variant *other) const
	   	{
			if (b == e) {
				const Data *other_data = VTable::GetData(other);
			   	other_data->vtable->Assign(other_data, that);
				return;
		   	}
			Variant *ref = 0;
			ref = GetPathElem(that, *b, false);
			if (!ref) {
				if (b + 1 == e) {
					SetPathElem(that, *b, other);
					return;
				} else {
					Variant null_value;
					SetPathElem(that, *b, &null_value);
					that->vtable->SetPath(that, b, e, other);
					return;
				}
			}
			Data *ref_data = VTable::GetData(ref);
			ref_data->vtable->SetPath(ref_data, b + 1, e, other);
		}

		void ErasePathElem(Data *that, const PathElement &elem) {
			if (elem.IsString()) {
				return that->vtable->EraseKey(that, elem.AsString());
			} else /* if (elem.IsNumber()) */ {
				return that->vtable->EraseIndex(that, elem.AsUnsigned());
			}
		}

		void VTable::ErasePath(Data *that, Path::const_iterator b, Path::const_iterator e, bool remove_empty) const
		{
			if (b == e) { return; }
			if (b + 1 == e) {
				ErasePathElem(that, *b);
				return;
			}
			Variant *ref = 0;
			ref = GetPathElem(that, *b, false);
			if (!ref) { return; }
			Data *ref_data = VTable::GetData(ref);
			ref_data->vtable->ErasePath(ref_data, b + 1, e, remove_empty);
			if (remove_empty && ref_data->vtable->Empty(ref_data)) {
				ErasePathElem(that, *b);
			}
		}

		bool VTable::Comparable(const Data *that, const Data *other) const {
			VariantDefines::Type_t ltype = that->vtable->GetType(that), otype = other->vtable->GetType(other);
			return ( ltype == otype )
				|| (
						( ltype == VariantDefines::IntegerType
						  || ltype == VariantDefines::UnsignedType
						  || ltype == VariantDefines::FloatType )
						&& ( otype == VariantDefines::IntegerType
						  || otype == VariantDefines::UnsignedType
						  || otype == VariantDefines::FloatType )
				   );
		}

		void VTable::Incr(Data *that) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Decr(Data *that) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		Variant VTable::Neg(const Data *that) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Add(Data *that, const Data *other) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Sub(Data *that, const Data *other) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Mul(Data *that, const Data *other) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Div(Data *that, const Data *other) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		void VTable::Rem(Data *that, const Data *other) const
		{ throw NotNumericTypeError(that->vtable->GetType(that)); }

		shared_ptr<RefData> VTable::EnsureRef(Data *that) const {
			if (!that->ref_data) {
				that->ref_data.reset(new RefData());
			}
			return that->ref_data;
		}

		void VTable::Destroy(Data *that) const {
			assert(that->vtable);
			that->storage.reset();
			that->vtable = 0;
		}

		void VTable::Destruct(Data *that) const {
			that->vtable->Destroy(that);
			if (that->ref_data) {
				that->ref_data->valid = false;
				that->ref_data.reset();
			}
		}

		Variant *VTable::Resolve(Data *that) const
	   	{ return VTable::GetVar(that); }

		const Variant *VTable::ResolveConst(const Data *that) const
	   	{ return VTable::GetVar(that); }

		Variant *VTable::ResolveDefault(Data *that, const Data *) const
	   	{ return VTable::GetVar(that); }

		bool VTable::Exists(const Data *that) const { return true; }

		//--------------------
		// NullVTable
		//--------------------

		class Null : public VTable {
		public:
			Null() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::List &AsList(Data *that) const;
			virtual void EraseIndex(Data *that, unsigned i) const;
			virtual VariantRef GetRefIndex(Data *that, unsigned i, Variant *def) const;
			virtual const Variant *GetConstIndex(const Data *that, unsigned i, bool checked) const;
			virtual Variant *GetIndex(Data *that, unsigned i, bool checked) const;
			virtual void SetIndex(Data *that, unsigned i, const Data *other) const;
			virtual Variant::Map &AsMap(Data *that) const;
			virtual void EraseKey(Data *that, const std::string &key) const;
			virtual VariantRef GetRefKey(Data *that, const std::string &s, Variant *def) const;
			virtual const Variant *GetConstKey(const Data *that, const std::string &s, bool checked) const;
			virtual Variant *GetKey(Data *that, const std::string &s, bool checked) const;
			virtual void SetKey(Data *that, const std::string &s, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual Variant *ResolveDefault(Data *that, const Data *def) const;
		};

		const VTable *NullVTable() {
			static const Null null_vtable;
			return &null_vtable;
		}

		void Null::Copy(const Data *that, Data *other) const { NullInit(other); }

		VariantDefines::Type_t Null::GetType(const Data *that) const
		{ return VariantDefines::NullType; }

		bool Null::AsBool(const Data *that) const { return false; }

		long double Null::AsLongDouble(const Data *that) const { return 0.0; }

		uintmax_t Null::AsUnsigned(const Data *that) const { return 0; }

		intmax_t Null::AsInt(const Data *that) const { return 0; }

		std::string Null::AsString(const Data *that) const { return ""; }

		unsigned Null::Size(const Data *that) const { return 0; }

		bool Null::Empty(const Data *that) const { return true; }

		void Null::Clear(Data *that) const {}

		Variant::List &Null::AsList(Data *that) const {
			ListInit(that, Variant::List());
			return that->vtable->AsList(that);
		}
		void Null::EraseIndex(Data *that, unsigned i) const {}

		VariantRef Null::GetRefIndex(Data *that, unsigned i, Variant *def) const {
			if (def) {
				ListInit(that, Variant::List());
				return that->vtable->GetRefIndex(that, i, def);
			}
			return VariantRef(*that->vtable->Resolve(that), Path(1, i));
		}

		const Variant *Null::GetConstIndex(const Data *that, unsigned i, bool checked) const { 
			if (!checked) { return 0; }
			throw std::out_of_range("Variant::List index out of range.");
		}

		Variant *Null::GetIndex(Data *that, unsigned i, bool checked) const {
			if (!checked) { return 0; }
			throw std::out_of_range("Variant::List index out of range.");
		}

		void Null::SetIndex(Data *that, unsigned i, const Data *other) const {
			ListInit(that, Variant::List());
			that->vtable->SetIndex(that, i, other);
		}

		Variant::Map &Null::AsMap(Data *that) const {
			MapInit(that, Variant::Map());
			return that->vtable->AsMap(that);
		}


		void Null::EraseKey(Data *that, const std::string &) const {}

		VariantRef Null::GetRefKey(Data *that, const std::string &s, Variant *def) const {
			if (def) {
				MapInit(that, Variant::Map());
				return that->vtable->GetRefKey(that, s, def);
			}
			return VariantRef(*that->vtable->Resolve(that), Path(1, s));
		}

		const Variant *Null::GetConstKey(const Data *that, const std::string &s, bool checked) const {
			if (!checked) { return 0; }
			throw KeyError(s);
		}

		Variant *Null::GetKey(Data *that, const std::string &s, bool checked) const {
			if (!checked) { return 0; }
			throw KeyError(s);
		}

		void Null::SetKey(Data *that, const std::string &s, const Data *other) const {
			MapInit(that, Variant::Map());
			that->vtable->SetKey(that, s, other);
		}

		int Null::Compare(const Data *that, const Data *other) const {
			return (Comparable(that, other) ? 0 : 1);
		}

		Variant *Null::ResolveDefault(Data *that, const Data *def) const {
			def->vtable->Assign(def, that);
			return that->vtable->Resolve(that);
		}

		void NullInit(Data *that) {
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = NullVTable();
		}

		//--------------------
		// BoolVTable
		//--------------------

		class Bool : public VTable {
		public:
			Bool() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual int Compare(const Data *that, const Data *other) const;
		};

		void Bool::Copy(const Data *that, Data *other) const
		{ BoolInit(other, that->b); }

		VariantDefines::Type_t Bool::GetType(const Data *that) const
		{ return VariantDefines::BoolType; }

		bool Bool::AsBool(const Data *that) const
		{ return that->b; }

		long double Bool::AsLongDouble(const Data *that) const
		{ return that->b; }

		uintmax_t Bool::AsUnsigned(const Data *that) const
		{ return that->b; }

		intmax_t Bool::AsInt(const Data *that) const
		{ return that->b; }

		std::string Bool::AsString(const Data *that) const {
			if (that->b) { return "true"; }
			else { return "false"; }
		}

		int Bool::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				other = VTable::GetData(other->vtable->ResolveConst(other));
				return (that->b == other->b ? 0 : 1);
			}
			return 1;
		}

		void BoolInit(Data *that, bool b) {
			static const Bool bool_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &bool_vtable;
			that->b = b;
		}

		//--------------------
		// NumericVTable
		//--------------------
		
		class Numeric : public VTable {
		public:
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Add(Data *that, const Data *other) const;
			virtual void Sub(Data *that, const Data *other) const;
			virtual void Mul(Data *that, const Data *other) const;
			virtual void Div(Data *that, const Data *other) const;
			virtual void Rem(Data *that, const Data *other) const;
		};

		int Numeric::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				long double us = that->vtable->AsLongDouble(that);
				long double o = other->vtable->AsLongDouble(other);
				if (us < o) { return -1; }
				else if (us > o) { return 1; }
				else { return 0; }
			}
			return 1;
		}

		template< template<typename T> class Op >
		void NumericOperation(Data *that, const Data *other) {
			VariantDefines::Type_t ntype = NumericUpcast(that->vtable->GetType(that),
					other->vtable->GetType(other));
			switch (ntype) {
			case VariantDefines::IntegerType:
				{
					Op<intmax_t> op;
					IntegerInit(that, 
							op(that->vtable->AsInt(that), other->vtable->AsInt(other)));
				}
				break;
			case VariantDefines::UnsignedType:
				{
					Op<uintmax_t> op;
					UnsignedInit(that, 
							op(that->vtable->AsUnsigned(that), other->vtable->AsUnsigned(other)));
				}
				break;
			case VariantDefines::FloatType:
				{
					Op<long double> op;
					FloatInit(that, 
							op(that->vtable->AsLongDouble(that), other->vtable->AsLongDouble(other)));
				}
				break;
			default:
				throw NotNumericTypeError(ntype);
			}
		}

		void Numeric::Add(Data *that, const Data *other) const
		{ NumericOperation<std::plus>(that, other); }

		void Numeric::Sub(Data *that, const Data *other) const
		{ NumericOperation<std::minus>(that, other); }

		void Numeric::Mul(Data *that, const Data *other) const
		{ NumericOperation<std::multiplies>(that, other); }

		void Numeric::Div(Data *that, const Data *other) const
		{ NumericOperation<std::divides>(that, other); }

		void Numeric::Rem(Data *that, const Data *other) const
		{ NumericOperation<my_modulus>(that, other); }

		//--------------------
		// IntegerVTable
		//--------------------

		class Integer : public Numeric {
		public:
			Integer() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
		};

		void Integer::Copy(const Data *that, Data *other) const
		{ IntegerInit(other, that->i); }

		VariantDefines::Type_t Integer::GetType(const Data *that) const
		{ return VariantDefines::IntegerType; }

		bool Integer::AsBool(const Data *that) const
		{ return that->i != 0; }

		long double Integer::AsLongDouble(const Data *that) const
		{ return (long double)that->i; }

		uintmax_t Integer::AsUnsigned(const Data *that) const
		{ return (uintmax_t)that->i; }

		intmax_t Integer::AsInt(const Data *that) const
		{ return that->i; }

		std::string Integer::AsString(const Data *that) const {
			std::ostringstream oss;
			oss << that->i;
			return oss.str();
		}

		void Integer::Incr(Data *that) const
		{ ++that->i; }

		void Integer::Decr(Data *that) const
		{ --that->i; }

		Variant Integer::Neg(const Data *that) const
		{ return Variant(-that->i); }

		void IntegerInit(Data *that, intmax_t i) {
			static const Integer integer_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &integer_vtable;
			that->i = i;
		}

		//--------------------
		// UnsignedVTable
		//--------------------

		class Unsigned : public Numeric {
		public:
			Unsigned() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
		};

		void Unsigned::Copy(const Data *that, Data *other) const
		{ UnsignedInit(other, that->u); }

		VariantDefines::Type_t Unsigned::GetType(const Data *that) const
		{ return VariantDefines::UnsignedType; }

		bool Unsigned::AsBool(const Data *that) const
		{ return that->u != 0; }

		long double Unsigned::AsLongDouble(const Data *that) const
		{ return (long double)that->u; }

		uintmax_t Unsigned::AsUnsigned(const Data *that) const
		{ return that->u; }

		intmax_t Unsigned::AsInt(const Data *that) const
		{ return (intmax_t)that->u; }

		std::string Unsigned::AsString(const Data *that) const {
			std::ostringstream oss;
			oss << that->u;
			return oss.str();
		}

		void Unsigned::Incr(Data *that) const
		{ ++that->u; }

		void Unsigned::Decr(Data *that) const
		{ --that->u; }

		Variant Unsigned::Neg(const Data *that) const {
			return Variant(-that->u);
		}

		void UnsignedInit(Data *that, uintmax_t u) {
			static const Unsigned unsigned_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &unsigned_vtable;
			that->u = u;
		}

		//--------------------
		// FloatVTable
		//--------------------

		class Float : public Numeric {
		public:
			Float() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
		};

		void Float::Copy(const Data *that, Data *other) const {
			FloatInit(other, that->f);
		}

		VariantDefines::Type_t Float::GetType(const Data *that) const
		{ return VariantDefines::FloatType; }

		bool Float::AsBool(const Data *that) const
		{ return that->f != 0; }

		long double Float::AsLongDouble(const Data *that) const
		{ return that->f; }

		uintmax_t Float::AsUnsigned(const Data *that) const
		{ return (uintmax_t)that->f; }

		intmax_t Float::AsInt(const Data *that) const
		{ return (intmax_t)that->f; }

		std::string Float::AsString(const Data *that) const {
			std::ostringstream oss;
			oss << that->f;
			return oss.str();
		}

		void Float::Incr(Data *that) const
		{ ++that->f; }

		void Float::Decr(Data *that) const
		{ --that->f; }

		Variant Float::Neg(const Data *that) const {
			return Variant(-that->f);
		}

		void FloatInit(Data *that, long double f) {
			static const Float float_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &float_vtable;
			that->f = f;
		}

		//--------------------
		// StringVTable
		//--------------------

		class String : public VTable {
		public:
			String() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Destroy(Data *that) const;
		};

		const VTable *StringVTable() {
			static const String string_vtable;
			return &string_vtable;
		}

		void String::Copy(const Data *that, Data *other) const
		{ StringInit(other, *that->s); }

		void String::Assign(const Data *that, Data *other) const {
			assert(that->vtable);
			shared_ptr<Storage> storage = that->storage;
			std::string *s = that->s;
			if (other->vtable) { other = VTable::GetData(other->vtable->Resolve(other)); }
			if (other->vtable) { other->vtable->Destroy(other); }
			other->vtable = StringVTable();
			other->s = s;
			other->storage = storage;
		}

		VariantDefines::Type_t String::GetType(const Data *that) const
		{ return VariantDefines::StringType; }

		bool String::AsBool(const Data *that) const
		{ return ParseBool(*that->s); }

		long double String::AsLongDouble(const Data *that) const {
			std::istringstream iss(*that->s);
			long double val = 0;
			iss >> val;
			if (iss.fail()) { throw UnableToConvertError(that->vtable->GetType(that), "a float"); }
			return val;
		}

		uintmax_t String::AsUnsigned(const Data *that) const {
			std::istringstream iss(*that->s);
			uintmax_t val = 0;
			iss >> val;
			if (iss.fail()) { throw UnableToConvertError(that->vtable->GetType(that), "an unsigned"); }
			return val;
		}

		intmax_t String::AsInt(const Data *that) const {
			std::istringstream iss(*that->s);
			intmax_t val = 0;
			iss >> val;
			if (iss.fail()) { throw UnableToConvertError(that->vtable->GetType(that), "an int"); }
			return val;
		}

		std::string String::AsString(const Data *that) const
		{ return *that->s; }

		unsigned String::Size(const Data *that) const { return that->s->size(); }

		bool String::Empty(const Data *that) const { return that->s->empty(); }

		void String::Clear(Data *that) const { return that->s->clear(); }

		int String::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				other = VTable::GetData(other->vtable->ResolveConst(other));
				return that->s->compare(*other->s);
			}
			return 1;
		}

		void String::Destroy(Data *that) const {
			VTable::Destroy(that);
			that->s = 0;
		}

		void StringInit(Data *that, const std::string &s) {
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = StringVTable();
			shared_ptr<StringStorage> storage(new StringStorage(s));
			that->s = &storage->str;
			that->storage = storage;
		}

		//--------------------
		// ListVTable
		//--------------------

		class List : public VTable {
		public:
			List() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::List &AsList(Data *that) const;
			virtual const Variant::List &AsListConst(const Data *that) const;
			virtual bool ContainsIndex(const Data *that, unsigned i) const;
			virtual unsigned Index(const Data *that, Variant v) const;
			virtual void EraseIndex(Data *that, unsigned i) const;
			virtual VariantRef GetRefIndex(Data *that, unsigned i, Variant *def) const;
			virtual const Variant *GetConstIndex(const Data *that, unsigned i, bool checked) const;
			virtual Variant *GetIndex(Data *that, unsigned i, bool checked) const;
			virtual void SetIndex(Data *that, unsigned i, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Destroy(Data *that) const;
		};

		const VTable *ListVTable() {
			static const List list_vtable;
			return &list_vtable;
		}

		void List::Copy(const Data *that, Data *other) const {
			ListInit(other, Variant::List());
			for (Variant::ConstListIterator i(that->l->list.begin()), e(that->l->list.end());
					i != e; ++i) {
				other->l->list.push_back(i->Copy());
			}
		}

		void List::Assign(const Data *that, Data *other) const {
			assert(that->vtable);
			shared_ptr<Storage> storage = that->storage;
			ListStorage *l = that->l;
			if (other->vtable) { other = VTable::GetData(other->vtable->Resolve(other)); }
			if (other->vtable) { other->vtable->Destroy(other); }
			other->vtable = ListVTable();
			other->l = l;
			other->storage = storage;
		}

		VariantDefines::Type_t List::GetType(const Data *that) const { return VariantDefines::ListType; }

		unsigned List::Size(const Data *that) const { return that->l->list.size(); }

		bool List::Empty(const Data *that) const { return that->l->list.empty(); }

		void List::Clear(Data *that) const { return that->l->list.clear(); }

		Variant::List &List::AsList(Data *that) const { return that->l->list; }

		const Variant::List &List::AsListConst(const Data *that) const { return that->l->list; }

		bool List::ContainsIndex(const Data *that, unsigned i) const { return i < that->l->list.size(); }

		unsigned List::Index(const Data *that, Variant v) const {
			Variant::ListIterator begin = that->l->list.begin(),
				end = that->l->list.end();
			return std::distance(begin, std::find(begin, end, v));
		}

		void List::EraseIndex(Data *that, unsigned i) const
		{
			unsigned size = that->l->list.size();
			if (i + 1 < size) {
				that->l->list.at(i).Assign(Variant::NullType);
			} else if (i < size) {
				that->l->list.pop_back();
			}
		}

		VariantRef List::GetRefIndex(Data *that, unsigned i, Variant *def) const {
			if (i < that->l->list.size()) {
				return VariantRef(that->l->list.at(i));
			} else if (def) {
				that->l->list.resize(i + 1);
				that->l->list.at(i) = *def;
				return VariantRef(that->l->list.at(i));
			} else {
				return VariantRef(*that->vtable->Resolve(that), Path(1, i));
			}
		}

		const Variant *List::GetConstIndex(const Data *that, unsigned i, bool checked) const {
			if (i < that->l->list.size()) {
				return &that->l->list.at(i);
			} else if (!checked) {
				return 0;
			} else {
				throw std::out_of_range("Variant::List index out of range.");
			}
		}

		Variant *List::GetIndex(Data *that, unsigned i, bool checked) const {
			if (i < that->l->list.size()) {
				return &that->l->list.at(i);
			} else if (!checked) {
				return 0;
			} else {
				throw std::out_of_range("Variant::List index out of range.");
			}
		}

		void List::SetIndex(Data *that, unsigned i, const Data *other) const {
			if ( ! (i < that->l->list.size()) ) {
				that->l->list.resize(i + 1);
			}
			other->vtable->Assign(other, VTable::GetData(&that->l->list.at(i)));
		}

		int List::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				other = VTable::GetData(other->vtable->ResolveConst(other));
				return (that->l->list != other->l->list ? 1 : 0);
			}
			return 1;
		}

		void List::Destroy(Data *that) const {
			VTable::Destroy(that);
			that->l = 0;
		}

		void ListInit(Data *that, const Variant::List &l) {
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = ListVTable();
			shared_ptr<ListStorage> storage(new ListStorage(l));
			that->l = storage.get();
			that->storage = storage;
		}

		//--------------------
		// MapVTable
		//--------------------

		class Map : public VTable {
		public:
			Map() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::Map &AsMap(Data *that) const;
			virtual const Variant::Map &AsMapConst(const Data *that) const;
			virtual bool ContainsKey(const Data *that, const std::string &s) const;
			virtual void EraseKey(Data *that, const std::string &key) const;
			virtual VariantRef GetRefKey(Data *that, const std::string &s, Variant *def) const;
			virtual const Variant *GetConstKey(const Data *that, const std::string &s, bool checked) const;
			virtual Variant *GetKey(Data *that, const std::string &s, bool checked) const;
			virtual void SetKey(Data *that, const std::string &s, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Destroy(Data *that) const;
		};

		const VTable *MapVTable() {
			static const Map map_vtable;
			return &map_vtable;
		}

		void Map::Copy(const Data *that, Data *other) const {
			MapInit(other, Variant::Map());
			for (Variant::ConstMapIterator i(that->m->map.begin()), e(that->m->map.end());
					i != e; ++i) {
				other->m->map.insert(std::make_pair(i->first, i->second.Copy()));
			}
		}

		void Map::Assign(const Data *that, Data *other) const {
			assert(that->vtable);
			shared_ptr<Storage> storage = that->storage;
			MapStorage *m = that->m;
			if (other->vtable) { other = VTable::GetData(other->vtable->Resolve(other)); }
			if (other->vtable) { other->vtable->Destroy(other); }
			other->vtable = MapVTable();
			other->m = m;
			other->storage = storage;
		}

		VariantDefines::Type_t Map::GetType(const Data *that) const { return VariantDefines::MapType; }

		unsigned Map::Size(const Data *that) const { return that->m->map.size(); }

		bool Map::Empty(const Data *that) const { return that->m->map.empty(); }

		void Map::Clear(Data *that) const { return that->m->map.clear(); }

		Variant::Map &Map::AsMap(Data *that) const { return that->m->map; }

		const Variant::Map &Map::AsMapConst(const Data *that) const { return that->m->map; }

		bool Map::ContainsKey(const Data *that, const std::string &s) const
		{ return that->m->map.find(s) != that->m->map.end(); }

		void Map::EraseKey(Data *that, const std::string &key) const { that->m->map.erase(key); }

		VariantRef Map::GetRefKey(Data *that, const std::string &s, Variant *def) const {
			Variant::MapIterator entry = that->m->map.find(s);
			if (entry != that->m->map.end()) {
				return VariantRef(entry->second);
			} else if (def) {
				return VariantRef(that->m->map.insert(std::make_pair(s, *def)).first->second);
			} else {
				return VariantRef(*that->vtable->Resolve(that), Path(1, s));
			}
		}

		const Variant *Map::GetConstKey(const Data *that, const std::string &s, bool checked) const {
			Variant::MapIterator entry = that->m->map.find(s);
			if (entry != that->m->map.end()) {
				return &entry->second;
			} else if (!checked) {
				return 0;
			} else {
				throw KeyError(s);
			}
		}

		Variant *Map::GetKey(Data *that, const std::string &s, bool checked) const {
			Variant::MapIterator entry = that->m->map.find(s);
			if (entry != that->m->map.end()) {
				return &entry->second;
			} else if (!checked) {
				return 0;
			} else {
				throw KeyError(s);
			}
		}

		void Map::SetKey(Data *that, const std::string &s, const Data *other) const {
			Variant::MapIterator entry = that->m->map.insert(std::make_pair(s, Variant())).first;
			other->vtable->Assign(other, VTable::GetData(&entry->second));
		}


		int Map::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				other = VTable::GetData(other->vtable->ResolveConst(other));
				return (that->m->map != other->m->map ? 1 : 0);
			}
			return 1;
		}

		void Map::Destroy(Data *that) const {
			VTable::Destroy(that);
			that->m = 0;
		}

		void MapInit(Data *that, const Variant::Map &m) {
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = MapVTable();
			shared_ptr<MapStorage> storage(new MapStorage(m));
			that->m = storage.get();
			that->storage = storage;
		}

		//--------------------
		// BlobVTable
		//--------------------

		class Blob : public VTable {
		public:
			Blob() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual BlobPtr AsBlob(Data *that) const;
			virtual ConstBlobPtr AsBlobConst(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual int Compare(const Data *that, const Data *other) const;
		};

		const VTable *BlobVTable() {
			static const Blob blob_vtable;
			return &blob_vtable;
		}

		void Blob::Copy(const Data *that, Data *other) const
		{ BlobInit(other, ( (*that->blob) ? (*that->blob)->Copy() : BlobPtr() )); }

		void Blob::Assign(const Data *that, Data *other) const {
			assert(that->vtable);
			shared_ptr<Storage> storage = that->storage;
			BlobPtr *b = that->blob;
			if (other->vtable) { other = VTable::GetData(other->vtable->Resolve(other)); }
			if (other->vtable) { other->vtable->Destroy(other); }
			other->vtable = BlobVTable();
			other->blob = b;
			other->storage = storage;
		}

		VariantDefines::Type_t Blob::GetType(const Data *that) const
		{ return VariantDefines::BlobType; }

		BlobPtr Blob::AsBlob(Data *that) const
		{ return *(that->blob); }

		ConstBlobPtr Blob::AsBlobConst(const Data *that) const
		{ return *(that->blob); }

		unsigned Blob::Size(const Data *that) const
		{ return (*that->blob)->GetTotalLength(); }

		int Blob::Compare(const Data *that, const Data *other) const {
			if (Comparable(that, other)) {
				ConstBlobPtr us = (*that->blob);
				ConstBlobPtr o = other->vtable->AsBlobConst(other);
				if (us == o) { return 0; }
				if (us && o) { return us->Compare(o); }
			}
			return 1;
		}

		void BlobInit(Data *that, BlobPtr blob) {
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = BlobVTable();
			shared_ptr<BlobStorage> storage(new BlobStorage(blob));
			that->blob = &storage->blob;
			that->storage = storage;
		}

		//--------------------
		// RefVTable
		//--------------------

		class Ref : public VTable {
		public:
			Ref() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual void MakeRef(const Data *that, Data *ref) const;
			virtual void MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual BlobPtr AsBlob(Data *that) const;
			virtual ConstBlobPtr AsBlobConst(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::List &AsList(Data *that) const;
			virtual const Variant::List &AsListConst(const Data *that) const;
			virtual bool ContainsIndex(const Data *that, unsigned i) const;
			virtual unsigned Index(const Data *that, Variant v) const;
			virtual void EraseIndex(Data *that, unsigned i) const;
			virtual VariantRef GetRefIndex(Data *that, unsigned i, Variant *def) const;
			virtual const Variant *GetConstIndex(const Data *that, unsigned i, bool checked) const;
			virtual Variant *GetIndex(Data *that, unsigned i, bool checked) const;
			virtual void SetIndex(Data *that, unsigned i, const Data *other) const;
			virtual Variant::Map &AsMap(Data *that) const;
			virtual const Variant::Map &AsMapConst(const Data *that) const;
			virtual bool ContainsKey(const Data *that, const std::string &s) const;
			virtual void EraseKey(Data *that, const std::string &key) const;
			virtual VariantRef GetRefKey(Data *that, const std::string &s, Variant *def) const;
			virtual const Variant *GetConstKey(const Data *that, const std::string &s, bool checked) const;
			virtual Variant *GetKey(Data *that, const std::string &s, bool checked) const;
			virtual void SetKey(Data *that, const std::string &s, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
			virtual void Add(Data *that, const Data *other) const;
			virtual void Sub(Data *that, const Data *other) const;
			virtual void Mul(Data *that, const Data *other) const;
			virtual void Div(Data *that, const Data *other) const;
			virtual void Rem(Data *that, const Data *other) const;
			virtual void Destroy(Data *that) const;
			virtual void Destruct(Data *that) const;
			virtual Variant *Resolve(Data *that) const;
			virtual const Variant *ResolveConst(const Data *that) const;
			virtual Variant *ResolveDefault(Data *that, const Data *def) const;
			virtual bool Exists(const Data *that) const;
		};

		static inline void CheckRef(const Data *that) {
			if (!that->ref_data || !that->ref_data->valid) {
				throw InvalidReferenceError("Error attempting to access an invalid VariantRef");
			}
		}

		void Ref::Copy(const Data *that, Data *other) const
		{ CheckRef(that); that->ref->vtable->Copy(that->ref, other); }

		void Ref::Assign(const Data *that, Data *other) const
		{ CheckRef(that); that->ref->vtable->Assign(that->ref, other); }

		void Ref::MakeRef(const Data *that, Data *ref) const
		{ CheckRef(that); that->ref->vtable->MakeRef(that->ref, ref); }

		void Ref::MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const
		{ CheckRef(that); that->ref->vtable->MakeProxy(that->ref, ref, b, e); }

		VariantDefines::Type_t Ref::GetType(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->GetType(that->ref); }

		bool Ref::AsBool(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsBool(that->ref); }

		long double Ref::AsLongDouble(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsLongDouble(that->ref); }

		uintmax_t Ref::AsUnsigned(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsUnsigned(that->ref); }

		intmax_t Ref::AsInt(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsInt(that->ref); }

		std::string Ref::AsString(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsString(that->ref); }

		BlobPtr Ref::AsBlob(Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsBlob(that->ref); }

		ConstBlobPtr Ref::AsBlobConst(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsBlobConst(that->ref); }

		unsigned Ref::Size(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->Size(that->ref); }

		bool Ref::Empty(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->Empty(that->ref); }

		void Ref::Clear(Data *that) const
		{ CheckRef(that); return that->ref->vtable->Clear(that->ref); }

		Variant::List &Ref::AsList(Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsList(that->ref); }

		const Variant::List &Ref::AsListConst(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsListConst(that->ref); }

		bool Ref::ContainsIndex(const Data *that, unsigned i) const
		{ CheckRef(that); return that->ref->vtable->ContainsIndex(that->ref, i); }

		unsigned Ref::Index(const Data *that, Variant v) const
		{ CheckRef(that); return that->ref->vtable->Index(that->ref, v); }

		void Ref::EraseIndex(Data *that, unsigned i) const
		{ CheckRef(that); return that->ref->vtable->EraseIndex(that->ref, i); }

		VariantRef Ref::GetRefIndex(Data *that, unsigned i, Variant *def) const
		{ CheckRef(that); return that->ref->vtable->GetRefIndex(that->ref, i , def); }

		const Variant *Ref::GetConstIndex(const Data *that, unsigned i, bool checked) const
		{ CheckRef(that); return that->ref->vtable->GetConstIndex(that->ref, i, checked); }

		Variant *Ref::GetIndex(Data *that, unsigned i, bool checked) const
		{ CheckRef(that); return that->ref->vtable->GetIndex(that->ref, i, checked); }

		void Ref::SetIndex(Data *that, unsigned i, const Data *other) const
		{ CheckRef(that); that->ref->vtable->SetIndex(that->ref, i, other); }

		Variant::Map &Ref::AsMap(Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsMap(that->ref); }

		const Variant::Map &Ref::AsMapConst(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->AsMapConst(that->ref); }

		bool Ref::ContainsKey(const Data *that, const std::string &s) const
		{ CheckRef(that); return that->ref->vtable->ContainsKey(that->ref, s); }

		void Ref::EraseKey(Data *that, const std::string &key) const
		{ CheckRef(that); return that->ref->vtable->EraseKey(that->ref, key); }

		VariantRef Ref::GetRefKey(Data *that, const std::string &s, Variant *def) const
		{ CheckRef(that); return that->ref->vtable->GetRefKey(that->ref, s, def); }

		const Variant *Ref::GetConstKey(const Data *that, const std::string &s, bool checked) const
		{ CheckRef(that); return that->ref->vtable->GetConstKey(that->ref, s, checked); }

		Variant *Ref::GetKey(Data *that, const std::string &s, bool checked) const
		{ CheckRef(that); return that->ref->vtable->GetKey(that->ref, s, checked); }

		void Ref::SetKey(Data *that, const std::string &s, const Data *other) const
		{ CheckRef(that); that->ref->vtable->SetKey(that->ref, s, other); }

		int Ref::Compare(const Data *that, const Data *other) const
		{ CheckRef(that); return that->ref->vtable->Compare(that->ref, other); }

		void Ref::Incr(Data *that) const
		{ CheckRef(that); that->ref->vtable->Incr(that->ref); }

		void Ref::Decr(Data *that) const
		{ CheckRef(that); that->ref->vtable->Decr(that->ref); }

		Variant Ref::Neg(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->Neg(that->ref); }

		void Ref::Add(Data *that, const Data *other) const
		{ CheckRef(that); that->ref->vtable->Add(that->ref, other); }

		void Ref::Sub(Data *that, const Data *other) const
		{ CheckRef(that); that->ref->vtable->Sub(that->ref, other); }

		void Ref::Mul(Data *that, const Data *other) const
		{ CheckRef(that); that->ref->vtable->Mul(that->ref, other); }

		void Ref::Div(Data *that, const Data *other) const
		{ CheckRef(that); that->ref->vtable->Div(that->ref, other); }

		void Ref::Rem(Data *that, const Data *other) const
		{ CheckRef(that); that->ref->vtable->Rem(that->ref, other); }

		void Ref::Destroy(Data *that) const {
			VTable::Destroy(that);
			that->ref = 0;
		}

		void Ref::Destruct(Data *that) const {
			Destroy(that);
			that->ref_data.reset();
		}

		Variant *Ref::Resolve(Data *that) const
	   	{ CheckRef(that); return that->ref->vtable->Resolve(that->ref); }

		const Variant *Ref::ResolveConst(const Data *that) const
		{ CheckRef(that); return that->ref->vtable->ResolveConst(that->ref); }

		Variant *Ref::ResolveDefault(Data *that, const Data *def) const
	   	{ CheckRef(that); return that->ref->vtable->ResolveDefault(that->ref, def); }

		bool Ref::Exists(const Data *that) const {
			return (that->ref != 0) && (that->ref_data->valid == true);
		}

		void RefInit(Data *that, const Data *other) {
			static const Ref ref_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &ref_vtable;
			// XXX: const cast.. double ICKY!
			that->ref = const_cast<Data*>(other);
			that->ref_data = that->ref->vtable->EnsureRef(that->ref);
		}

		//--------------------
		// ProxyVTable
		//--------------------

		class Proxy : public VTable {
		public:
			Proxy() {}
			virtual void Copy(const Data *that, Data *other) const;
			virtual void Assign(const Data *that, Data *other) const;
			virtual void MakeRef(const Data *that, Data *ref) const;
			virtual void MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const;
			virtual Variant::Type_t GetType(const Data *that) const;
			virtual bool AsBool(const Data *that) const;
			virtual long double AsLongDouble(const Data *that) const;
			virtual uintmax_t AsUnsigned(const Data *that) const;
			virtual intmax_t AsInt(const Data *that) const;
			virtual std::string AsString(const Data *that) const;
			virtual BlobPtr AsBlob(Data *that) const;
			virtual ConstBlobPtr AsBlobConst(const Data *that) const;
			virtual unsigned Size(const Data *that) const;
			virtual bool Empty(const Data *that) const;
			virtual void Clear(Data *that) const;
			virtual Variant::List &AsList(Data *that) const;
			virtual const Variant::List &AsListConst(const Data *that) const;
			virtual bool ContainsIndex(const Data *that, unsigned i) const;
			virtual unsigned Index(const Data *that, Variant v) const;
			virtual void EraseIndex(Data *that, unsigned i) const;
			virtual VariantRef GetRefIndex(Data *that, unsigned i, Variant *def) const;
			virtual const Variant *GetConstIndex(const Data *that, unsigned i, bool checked) const;
			virtual Variant *GetIndex(Data *that, unsigned i, bool checked) const;
			virtual void SetIndex(Data *that, unsigned i, const Data *other) const;
			virtual Variant::Map &AsMap(Data *that) const;
			virtual const Variant::Map &AsMapConst(const Data *that) const;
			virtual bool ContainsKey(const Data *that, const std::string &s) const;
			virtual void EraseKey(Data *that, const std::string &key) const;
			virtual VariantRef GetRefKey(Data *that, const std::string &s, Variant *def) const;
			virtual const Variant *GetConstKey(const Data *that, const std::string &s, bool checked) const;
			virtual Variant *GetKey(Data *that, const std::string &s, bool checked) const;
			virtual void SetKey(Data *that, const std::string &s, const Data *other) const;
			virtual int Compare(const Data *that, const Data *other) const;
			virtual void Incr(Data *that) const;
			virtual void Decr(Data *that) const;
			virtual Variant Neg(const Data *that) const;
			virtual void Add(Data *that, const Data *other) const;
			virtual void Sub(Data *that, const Data *other) const;
			virtual void Mul(Data *that, const Data *other) const;
			virtual void Div(Data *that, const Data *other) const;
			virtual void Rem(Data *that, const Data *other) const;
			virtual void Destroy(Data *that) const;
			virtual void Destruct(Data *that) const;
			virtual Variant *Resolve(Data *that) const;
			virtual const Variant *ResolveConst(const Data *that) const;
			virtual Variant *ResolveDefault(Data *that, const Data *def) const;
			virtual bool Exists(const Data *that) const;
		};

		bool ProxyResolveCheck(const Data *that) {
			CheckRef(that);
			Data *ref = that->proxy->ref;
			const Variant *result = ref->vtable->GetPathConst(ref, that->proxy->path.begin(),
					that->proxy->path.end(), false);
			if (result) {
				const Data *res = VTable::GetData(result);
				// XXX: Const cast, ICKY! I don't know how to do this without making all of Data mutable...
				res->vtable->MakeRef(res, const_cast<Data*>(that));
				return true;
			}
			return false;
		}

		void ProxyResolveThrow(const Data *that) {
			CheckRef(that);
			Data *ref = that->proxy->ref;
			const Variant *result = ref->vtable->GetPathConst(ref, that->proxy->path.begin(),
					that->proxy->path.end(), true);
			const Data *res = VTable::GetData(result);
			// XXX: Const cast, ICKY! I don't know how to do this without making all of Data mutable...
			res->vtable->MakeRef(res, const_cast<Data*>(that));
		}

		void ProxyResolveCreate(Data *that) {
			CheckRef(that);
			Variant tmp_null;
			Data *ref = that->proxy->ref;
			VariantRef result = ref->vtable->GetPathRef(ref, that->proxy->path.begin(),
					that->proxy->path.end(), &tmp_null);
			Data *res = VTable::GetData(&result);
			res->vtable->MakeRef(res, that);
		}


		void Proxy::Copy(const Data *that, Data *other) const
		{ ProxyResolveThrow(that); that->vtable->Copy(that, other); }

		void Proxy::Assign(const Data *that, Data *other) const
		{ ProxyResolveThrow(that); that->vtable->Assign(that, other); }

		void Proxy::MakeRef(const Data *that, Data *ref) const {
			CheckRef(that);
			ProxyInit(ref, that->proxy->ref, that->proxy->path.begin(), that->proxy->path.end());
		}

		void Proxy::MakeProxy(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) const
		{
			CheckRef(that);
			Path newpath = that->proxy->path;
			newpath.insert(newpath.end(), b, e);
			ProxyInit(ref, that->proxy->ref, newpath.begin(), newpath.end());
		}

		VariantDefines::Type_t Proxy::GetType(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->GetType(that); }

		bool Proxy::AsBool(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsBool(that); }

		long double Proxy::AsLongDouble(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsLongDouble(that); }

		uintmax_t Proxy::AsUnsigned(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsUnsigned(that); }

		intmax_t Proxy::AsInt(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsInt(that); }

		std::string Proxy::AsString(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsString(that); }

		BlobPtr Proxy::AsBlob(Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsBlob(that); }

		ConstBlobPtr Proxy::AsBlobConst(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsBlobConst(that); }

		unsigned Proxy::Size(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Size(that); }

		bool Proxy::Empty(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Empty(that); }

		void Proxy::Clear(Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Clear(that); }

		Variant::List &Proxy::AsList(Data *that) const
		{ ProxyResolveCreate(that); return that->vtable->AsList(that); }

		const Variant::List &Proxy::AsListConst(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsListConst(that); }

		bool Proxy::ContainsIndex(const Data *that, unsigned i) const
		{ ProxyResolveThrow(that); return that->vtable->ContainsIndex(that, i); }

		unsigned Proxy::Index(const Data *that, Variant v) const
		{ ProxyResolveThrow(that); return that->vtable->Index(that, v); }

		void Proxy::EraseIndex(Data *that, unsigned i) const
		{ ProxyResolveThrow(that); return that->vtable->EraseIndex(that, i); }

		VariantRef Proxy::GetRefIndex(Data *that, unsigned i, Variant *def) const
		{ 
			if (def) {
				ProxyResolveCreate(that);
				return that->vtable->GetRefIndex(that, i, def);
			}
			if (ProxyResolveCheck(that)) {
				return that->vtable->GetRefIndex(that, i, def);
		   	} else {
				Path path = that->proxy->path;
				path.push_back(i);
				return VariantRef(*VTable::GetVar(that->proxy->ref), path);
			}
		}

		const Variant *Proxy::GetConstIndex(const Data *that, unsigned i, bool checked) const
		{ ProxyResolveThrow(that); return that->vtable->GetConstIndex(that, i, checked); }

		Variant *Proxy::GetIndex(Data *that, unsigned i, bool checked) const
		{
		   	if (ProxyResolveCheck(that)) {
				return that->vtable->GetIndex(that, i, checked);
			} else if (checked)  {
				throw std::out_of_range("Variant::List index out of range.");
			} else {
				return 0;
			}
	   	}

		void Proxy::SetIndex(Data *that, unsigned i, const Data *other) const
		{ ProxyResolveCreate(that); return that->vtable->SetIndex(that, i, other); }

		Variant::Map &Proxy::AsMap(Data *that) const
		{ ProxyResolveCreate(that); return that->vtable->AsMap(that); }

		const Variant::Map &Proxy::AsMapConst(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->AsMapConst(that); }

		bool Proxy::ContainsKey(const Data *that, const std::string &s) const
		{ ProxyResolveThrow(that); return that->vtable->ContainsKey(that, s); }

		void Proxy::EraseKey(Data *that, const std::string &key) const
		{ ProxyResolveThrow(that); return that->vtable->EraseKey(that, key); }

		VariantRef Proxy::GetRefKey(Data *that, const std::string &s, Variant *def) const
		{
			if (def) {
				ProxyResolveCreate(that);
			   	return that->vtable->GetRefKey(that, s, def);
		   	}
			if (ProxyResolveCheck(that)) {
			   	return that->vtable->GetRefKey(that, s, def);
			} else {
				Path path = that->proxy->path;
				path.push_back(s);
				return VariantRef(*VTable::GetVar(that->proxy->ref), path);
			}
		}

		const Variant *Proxy::GetConstKey(const Data *that, const std::string &s, bool checked) const
		{ ProxyResolveThrow(that); return that->vtable->GetConstKey(that, s, checked); }

		Variant *Proxy::GetKey(Data *that, const std::string &s, bool checked) const
		{
		   	if (ProxyResolveCheck(that)) {
				return that->vtable->GetKey(that, s, checked);
			} else if (checked) {
				throw KeyError(s);
			} else {
				return 0;
			}
	   	}

		void Proxy::SetKey(Data *that, const std::string &s, const Data *other) const
		{ ProxyResolveCreate(that); return that->vtable->SetKey(that, s, other); }

		int Proxy::Compare(const Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Compare(that, other); }

		void Proxy::Incr(Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Incr(that); }

		void Proxy::Decr(Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Decr(that); }

		Variant Proxy::Neg(const Data *that) const
		{ ProxyResolveThrow(that); return that->vtable->Neg(that); }

		void Proxy::Add(Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Add(that, other); }

		void Proxy::Sub(Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Sub(that, other); }

		void Proxy::Mul(Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Mul(that, other); }

		void Proxy::Div(Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Div(that, other); }

		void Proxy::Rem(Data *that, const Data *other) const
		{ ProxyResolveThrow(that); return that->vtable->Rem(that, other); }

		void Proxy::Destroy(Data *that) const {
			VTable::Destroy(that);
			that->proxy = 0;
		}

		void Proxy::Destruct(Data *that) const {
			Destroy(that);
			that->ref_data.reset();
		}

		Variant *Proxy::Resolve(Data *that) const {
			ProxyResolveCreate(that);
			return that->vtable->Resolve(that);
		}

		const Variant *Proxy::ResolveConst(const Data *that) const {
			ProxyResolveThrow(that);
			return that->vtable->ResolveConst(that);
		}

		Variant *Proxy::ResolveDefault(Data *that, const Data *def) const {
			ProxyResolveCreate(that);
			return that->vtable->ResolveDefault(that, def);
		}

		bool Proxy::Exists(const Data *that) const {
			if (!that->ref_data || !that->ref_data->valid) {
				return false;
			}
			Data *ref = that->proxy->ref;
			const Variant *result = ref->vtable->GetPathConst(ref, that->proxy->path.begin(),
					that->proxy->path.end(), false);
			return result != 0;
		}

		void ProxyInit(Data *that, Data *ref, Path::const_iterator b, Path::const_iterator e) {
			static const Proxy proxy_vtable;
			if (that->vtable) that->vtable->Destroy(that);
			that->vtable = &proxy_vtable;
			that->ref_data = ref->vtable->EnsureRef(ref);
			shared_ptr<ProxyStorage> storage(new ProxyStorage(ref, b, e));
			that->proxy = storage.get();
			that->storage = storage;
		}


		//-------------------

		void DefaultInit(Data *that, VariantDefines::Type_t type) {
			switch (type) {
			case VariantDefines::NullType:
				NullInit(that);
				break;
			case VariantDefines::BoolType:
				BoolInit(that, false);
				break;
			case VariantDefines::IntegerType:
				IntegerInit(that, 0);
				break;
			case VariantDefines::UnsignedType:
				UnsignedInit(that, 0u);
				break;
			case VariantDefines::FloatType:
				FloatInit(that, 0.0);
				break;
			case VariantDefines::StringType:
				StringInit(that, "");
				break;
			case VariantDefines::ListType:
				ListInit(that, Variant::List());
				break;
			case VariantDefines::MapType:
				MapInit(that, Variant::Map());
				break;
			case VariantDefines::BlobType:
				BlobInit(that, BlobPtr());
				break;
			}
		}

		Data::Data() : vtable(NullVTable()) {}
	}

	//--------------------
	// Variant
	//--------------------

	Variant::Variant(const RefTag &, const Variant &o)
	{
		o.vtable->MakeRef(&o, this);
	}

	Variant::Variant(const RefTag &, Variant &o, Path::const_iterator b, Path::const_iterator e)
	{
		o.vtable->MakeProxy(&o, this, b, e);
	}

	Variant::~Variant() {
		vtable->Destruct(this);
	}

	VariantDefines::Type_t Variant::GetType() const
	{ return vtable->GetType(this); }

	bool Variant::AsBool() const
   	{ return vtable->AsBool(this); }

	long double Variant::AsLongDouble() const
	{ return vtable->AsLongDouble(this); }

	uintmax_t Variant::AsUnsigned() const
	{ return vtable->AsUnsigned(this); }

	intmax_t Variant::AsInt() const
	{ return vtable->AsInt(this); }

	std::string Variant::AsString() const
	{ return vtable->AsString(this); }

	BlobPtr Variant::AsBlob()
	{ return vtable->AsBlob(this); }

	ConstBlobPtr Variant::AsBlob() const
	{ return vtable->AsBlobConst(this); }

	unsigned Variant::Size() const
	{ return vtable->Size(this); }

	bool Variant::Empty() const
	{ return vtable->Empty(this); }

	void Variant::Clear()
	{ vtable->Clear(this); }

	Variant::List &Variant::AsList()
   	{ return vtable->AsList(this); }

	const Variant::List &Variant::AsList() const
   	{ return vtable->AsListConst(this); }

	VariantRef Variant::At(unsigned i)
	{ return vtable->GetRefIndex(this, i, 0); }

	VariantRef Variant::At(unsigned i, Variant def)
	{ return vtable->GetRefIndex(this, i, &def); }

	const Variant &Variant::At(unsigned i) const
	{ return *vtable->GetConstIndex(this, i, true); }

	Variant Variant::Get(unsigned i) const
	{ return *vtable->GetConstIndex(this, i, true); }

	Variant Variant::Get(unsigned i, Variant def) const {
		const Variant *ret = vtable->GetConstIndex(this, i, false);
		if (ret) { return *ret; }
		else { return def; }
	}

	Variant &Variant::Set(unsigned i, Variant v) {
		vtable->SetIndex(this, i, &v);
		return *this;
	}

	Variant &Variant::Append(Variant value) {
		vtable->AsList(this).push_back(value);
		return *this;
	}

	bool Variant::Contains(unsigned i) const
	{ return vtable->ContainsIndex(this, i); }

	unsigned Variant::Index(Variant v) const
	{ return vtable->Index(this, v); }

	void Variant::Erase(unsigned i)
	{ vtable->EraseIndex(this, i); }

	Variant::Map &Variant::AsMap()
	{ return vtable->AsMap(this); }

	const Variant::Map &Variant::AsMap() const
	{ return vtable->AsMapConst(this); }

	VariantRef Variant::At(const std::string &s)
	{ return vtable->GetRefKey(this, s, 0); }

	VariantRef Variant::At(const std::string &s, Variant def)
	{ return vtable->GetRefKey(this, s, &def); }

	const Variant &Variant::At(const std::string &s) const
	{ return *vtable->GetConstKey(this, s, true); }

	Variant Variant::Get(const std::string &s) const
	{ return *vtable->GetConstKey(this, s, true); }

	Variant Variant::Get(const std::string &s, Variant def) const {
		const Variant *ret = vtable->GetConstKey(this, s, false);
		if (ret) { return *ret; }
		else { return def; }
	}

	Variant &Variant::Set(const std::string &s, Variant v) {
		vtable->SetKey(this, s, &v);
		return *this;
	}

	bool Variant::Contains(const std::string &s) const
	{ return vtable->ContainsKey(this, s); }

	void Variant::Erase(const std::string &key)
	{ vtable->EraseKey(this, key); }

	VariantRef Variant::AtPath(const Path &path)
	{ return vtable->GetPathRef(this, path.begin(), path.end(), 0); }

	VariantRef Variant::AtPath(const Path &path, Variant def)
	{ return vtable->GetPathRef(this, path.begin(), path.end(), &def); }

	const Variant &Variant::AtPath(const Path &path) const
	{ return *vtable->GetPathConst(this, path.begin(), path.end(), true); }

	Variant Variant::GetPath(const Path &path) const
	{ return *vtable->GetPathConst(this, path.begin(), path.end(), true); }

	Variant Variant::GetPath(const Path &path, Variant def) const {
		const Variant *ret = vtable->GetPathConst(this, path.begin(), path.end(), false);
		if (ret) { return *ret; }
		else { return def; }
	}

	Variant &Variant::SetPath(const Path &path, Variant val) {
		vtable->SetPath(this, path.begin(), path.end(), &val);
		return *this;
	}

	Variant &Variant::ErasePath(const Path &path, bool remove_empty) {
		vtable->ErasePath(this, path.begin(), path.end(), remove_empty);
		return *this;
	}

	bool Variant::HasPath(const Path &path) const {
		return vtable->GetPathConst(this, path.begin(), path.end(), false) != 0;
	}

	void Variant::Merge(Variant other) {
		for (ConstMapIterator i(other.MapBegin()), e(other.MapEnd()); i != e; ++i) {
			this->Set(i->first, i->second);
		}
	}

	int Variant::Compare(const Variant &other) const
   	{ return vtable->Compare(this, &other); }

	bool Variant::Comparable(const Variant &other) const
	{ return vtable->Comparable(this, &other); }

	void Variant::Incr() {
		vtable->Incr(this);
	}

	void Variant::Decr() {
		vtable->Decr(this);
	}

	void Variant::Add(const Variant &o){
		vtable->Add(this, &o);
	}

	void Variant::Sub(const Variant &o){
		vtable->Sub(this, &o);
	}

	void Variant::Mul(const Variant &o){
		vtable->Mul(this, &o);
	}

	void Variant::Div(const Variant &o){
		vtable->Div(this, &o);
	}

	void Variant::Rem(const Variant &o){
		vtable->Rem(this, &o);
	}

	Variant Variant::Neg() const
   	{ return vtable->Neg(this); }

	VariantRef Variant::Ref() {
		return VariantRef(*this);
	}

	Variant Variant::Copy() const {
		Variant result;
		vtable->Copy(this, &result);
		return result;
	}

	Variant &Variant::Resolve() {
		return *vtable->Resolve(this);
	}

	const Variant &Variant::Resolve() const {
		return *vtable->ResolveConst(this);
	}

	Variant &Variant::Resolve(const Variant &def) {
		return *vtable->ResolveDefault(this, &def);
	}

	bool Variant::Exists() const {
		return vtable->Exists(this);
	}

	void Variant::Assign(const Variant &other)
   	{ other.vtable->Assign(&other, this); }

	void Variant::Assign(VariantDefines::Type_t type)
   	{ DefaultInit(vtable->Resolve(this), type); }

	void Variant::Assign(bool v)
   	{ Internal::BoolInit(vtable->Resolve(this), v); }

	void Variant::Assign(int v)
   	{ Internal::IntegerInit(vtable->Resolve(this), v); }

	void Variant::Assign(unsigned v)
   	{ Internal::UnsignedInit(vtable->Resolve(this), v); }

	void Variant::Assign(long v)
   	{ Internal::IntegerInit(vtable->Resolve(this), v); }

	void Variant::Assign(unsigned long v)
   	{ Internal::UnsignedInit(vtable->Resolve(this), v); }

	void Variant::Assign(long long v)
   	{ Internal::IntegerInit(vtable->Resolve(this), v); }

	void Variant::Assign(unsigned long long v)
   	{ Internal::UnsignedInit(vtable->Resolve(this), v); }

	void Variant::Assign(double v)
	{ Internal::FloatInit(vtable->Resolve(this), v); }

	void Variant::Assign(long double v)
	{ Internal::FloatInit(vtable->Resolve(this), v); }

	void Variant::Assign(BlobPtr b)
	{ Internal::BlobInit(vtable->Resolve(this), b); }

	void Variant::Assign(const std::string &v)
	{ Internal::StringInit(vtable->Resolve(this), v); }

	void Variant::Assign(const char *v)
	{ Internal::StringInit(vtable->Resolve(this), v); }

	void Variant::ReassignRef(const Variant &o) {
		o.vtable->MakeRef(&o, this);
	}
}
