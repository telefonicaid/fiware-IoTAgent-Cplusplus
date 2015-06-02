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
 * \brief 
 */
#ifndef VARIANT_BASE_H
#define VARIANT_BASE_H
#include <Variant/SharedPtr.h>
#include <Variant/Blob.h>
#include <Variant/Path.h>
#include <Variant/VariantDefines.h>
#include <map>
#include <vector>
#include <string>
#include <stdint.h>
namespace libvariant {

	struct RefTag {};

	class Variant;
	template<typename Base> class VariantRefImpl;


	/**
	 * Action implementation for the VariantRef class.
	 */
	template<typename Base>
	class VariantRefImpl : public Base {
	public:
		VariantRefImpl(const Variant &o) : Base(RefTag(), o) {}
		VariantRefImpl(Variant &o, const Path &path)
			: Base(RefTag(), o, path.begin(), path.end()) {}
		VariantRefImpl(Variant &o, Path::const_iterator b, Path::const_iterator e)
			: Base(RefTag(), o, b, e) {}
		VariantRefImpl(const VariantRefImpl<Base> &o)
			: Base(RefTag(), o) {}

		using Base::operator=;
		using Base::As;
		using Base::AsNumber;
		using Base::GetInto;
		using Base::GetPathInto;
		using Base::Assign;

		using Base::ReassignRef;
	};

	namespace Internal {

		class VTable;
		struct Storage;
		struct MapStorage;
		struct ListStorage;
		struct ProxyStorage;
		struct RefData;

		struct Data {
			Data();
			const VTable *vtable;
			union {
				bool b;
				long double f;
				intmax_t i;
				uintmax_t u;
				std::string *s;
				MapStorage *m;
				ListStorage *l;
				BlobPtr *blob;
				Data *ref;
				ProxyStorage *proxy;
			};
			shared_ptr<RefData> ref_data;
			shared_ptr<Storage> storage;
		};
	}



}
#endif
