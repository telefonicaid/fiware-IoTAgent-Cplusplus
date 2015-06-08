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
#include <Variant/Exceptions.h>
#include <sstream>
#include "StackTrace.h"

namespace libvariant {

	std::ostream &operator <<(std::ostream &os, VariantDefines::Type_t type) {
		switch (type) {
		case VariantDefines::NullType:
			os << "NullType";
			break;
		case VariantDefines::BoolType:
			os << "BoolType";
			break;
		case VariantDefines::IntegerType:
			os << "IntegerType";
			break;
		case VariantDefines::UnsignedType:
			os << "UnsignedType";
			break;
		case VariantDefines::FloatType:
			os << "FloatType";
			break;
		case VariantDefines::StringType:
			os << "StringType";
			break;
		case VariantDefines::ListType:
			os << "ListType";
			break;
		case VariantDefines::MapType:
			os << "MapType";
			break;
		case VariantDefines::BlobType:
			os << "BlobType";
			break;
		default:
			os << "Unknown(" << (int)type << ")";
			break;
		}
		return os;
	}

	void AddStackTrace(std::ostream &os, int level) {
#ifdef ENABLE_STACKTRACE
		os << "\nBacktrace:\n" << GetStack(level + 2) << "\n";
#endif
	}

	static inline std::string UnknownTypeErrorMessage(VariantDefines::Type_t type) {
		std::ostringstream oss;
		oss << "Unknown variant type \"" << type << "\"";
		AddStackTrace(oss, 2);
		return oss.str();
	}

	UnknownTypeError::UnknownTypeError(VariantDefines::Type_t type) throw()
		: std::runtime_error(UnknownTypeErrorMessage(type))
   	{}

	static inline std::string UnexpectedTypeErrorMessage(VariantDefines::Type_t expected, VariantDefines::Type_t value) {
		std::ostringstream oss;
		oss << "Unexpected variant type " << value << " expected " << expected;
		AddStackTrace(oss, 2);
		return oss.str();
	}

	static inline std::string UnexpectedTypeErrorMessage(const std::vector<VariantDefines::Type_t> &expected,
			VariantDefines::Type_t value) {
		std::ostringstream oss;
		oss << "Unexpected variant type " << value << " expected one of ";
		for (unsigned i = 0; i < expected.size(); ++i) {
			if (i != 0) { oss << ", "; }
			oss << expected[i];
		}
		AddStackTrace(oss, 2);
		return oss.str();
	}

	UnexpectedTypeError::UnexpectedTypeError(VariantDefines::Type_t expected, VariantDefines::Type_t value) throw()
		: std::runtime_error(UnexpectedTypeErrorMessage(expected, value))
   	{}

	UnexpectedTypeError::UnexpectedTypeError(const std::vector<VariantDefines::Type_t> &expected,
		   	VariantDefines::Type_t value) throw()
		: std::runtime_error(UnexpectedTypeErrorMessage(expected, value))
	{}

	static inline std::string UnableToConvertErrorMessage(VariantDefines::Type_t sourcetype,
		   	const std::string &targettype) {
		std::ostringstream oss;
		oss << "Unable to convert variant " << sourcetype << " to " << targettype;
		AddStackTrace(oss, 2);
		return oss.str();
	}

	UnableToConvertError::UnableToConvertError(VariantDefines::Type_t sourcetype,
		   	const std::string &targettype) throw()
		: std::runtime_error(UnableToConvertErrorMessage(sourcetype, targettype))
   	{}

	static inline std::string TypeNotIndexableErrorMessage(VariantDefines::Type_t type) {
		std::ostringstream oss;
		oss << "Variant type " << type << " is not an indexable type.";
		AddStackTrace(oss, 2);
		return oss.str();
	}

	TypeNotIndexableError::TypeNotIndexableError(VariantDefines::Type_t type) throw()
		: std::runtime_error(TypeNotIndexableErrorMessage(type))
   	{}

	static inline std::string NotNumericTypeErrorMessage(VariantDefines::Type_t type) {
		std::ostringstream oss;
		oss << "Variant type " << type << " not a numeric type.";
		AddStackTrace(oss, 2);
		return oss.str();
	}

	NotNumericTypeError::NotNumericTypeError(VariantDefines::Type_t type) throw()
		: std::runtime_error(NotNumericTypeErrorMessage(type))
	{}

	static inline std::string KeyErrorMessage(const std::string &key) {
		std::ostringstream oss;
		oss << "Key \"" << key << "\" not contained in map.";
		AddStackTrace(oss, 2);
		return oss.str();
	}

	KeyError::KeyError(const std::string &key) throw()
		: std::runtime_error(KeyErrorMessage(key))
	{}

	static inline std::string BadPathErrorMessage(VariantDefines::Type_t type) {
		std::ostringstream oss;
		oss << "Variant paths do not allow " << type << " as a path element.";
		AddStackTrace(oss, 2);
		return oss.str();
	}

	static inline std::string BadPathErrorMessage(const std::string &msg) {
		std::ostringstream oss;
		oss << msg;
		AddStackTrace(oss, 2);
		return oss.str();
	}

	BadPathError::BadPathError(VariantDefines::Type_t type) throw()
		: std::runtime_error(BadPathErrorMessage(type))
	{}

	BadPathError::BadPathError(const std::string &msg) throw()
		: std::runtime_error(BadPathErrorMessage(msg))
	{}

	static inline std::string InvalidReferenceErrorMessage(const std::string &msg) {
		std::ostringstream oss;
		oss << msg;
		AddStackTrace(oss, 2);
		return oss.str();
	}

	InvalidReferenceError::InvalidReferenceError(const std::string &msg) throw()
		: std::runtime_error(InvalidReferenceErrorMessage(msg))
	{}

	static inline std::string InvalidOperationErrorMessage(VariantDefines::Type_t type, const std::string &opname)
	{
		std::ostringstream oss;
		oss << "Variant cannot perform operation " << opname << " on type " << type;
		AddStackTrace(oss, 2);
		return oss.str();
	}

	InvalidOperationError::InvalidOperationError(VariantDefines::Type_t type, const std::string &opname) throw()
		: std::runtime_error(InvalidOperationErrorMessage(type, opname))
	{}
}

