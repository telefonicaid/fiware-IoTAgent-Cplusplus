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
#include <Variant/Variant.h>
#include <Variant/Emitter.h>
#include <stdexcept>
#include <sstream>
#include <fstream>

namespace libvariant {

	Emitter &operator<<(Emitter &e, const Variant &v) {
		switch (v.GetType()) {
		case Variant::NullType:
			e.EmitNull();
			break;
		case Variant::BoolType:
			if (v.AsBool()) {
				e.EmitTrue();
			} else {
				e.EmitFalse();
			}
			break;
		case Variant::IntegerType:
			e.Emit(v.AsInt());
			break;
		case Variant::UnsignedType:
			e.Emit(v.AsUnsigned());
			break;
		case Variant::FloatType:
			e.Emit(v.AsDouble());
			break;
		case Variant::StringType:
			e.Emit(v.AsString());
			break;
		case Variant::ListType:
			e << v.AsList();
			break;
		case Variant::MapType:
			e << v.AsMap();
			break;
		case Variant::BlobType:
			e << v.AsBlob();
			break;
		default:
			throw UnknownTypeError(v.GetType());
		}
		return e;
	}

	std::string Serialize(Variant v, SerializeType type, Variant params) {
		std::ostringstream oss;
		Serialize(oss.rdbuf(), v, type, params);
		return oss.str();

	}

	void Serialize(const std::string &filename, Variant v, SerializeType type,
		   	Variant params) {
		Emitter emitter = CreateEmitter(CreateEmitterOutput(filename.c_str()), type, params);
		emitter << v;
		emitter.Close();
	}

	void Serialize(FILE *f, Variant v, SerializeType type,
		   	Variant params) {
		Emitter emitter = CreateEmitter(CreateEmitterOutput(f), type, params);
		emitter << v;
		emitter.Close();
	}

	void Serialize(std::streambuf *sb, Variant v, SerializeType type,
		   	Variant params) {
		Emitter emitter = CreateEmitter(CreateEmitterOutput(sb), type, params);
		emitter << v;
		emitter.Close();

	}

	unsigned Serialize(void *ptr, unsigned len, Variant v, SerializeType type,
		   	Variant params) {
		unsigned out_len = 0;
		Emitter emitter = CreateEmitter(CreateEmitterOutput(ptr, len, &out_len), type, params);
		emitter << v;
		emitter.Close();
		return out_len;
	}
}

void VariantPrint(const libvariant::Variant v) {
	libvariant::SerializeJSON(stderr, v, true);
	fprintf(stderr, "\n");
}
