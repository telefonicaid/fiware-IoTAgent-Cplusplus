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
#ifndef VARIANT_EXCEPTIONS_H
#define VARIANT_EXCEPTIONS_H
#include <Variant/VariantDefines.h>
#include <stdexcept>
#include <iosfwd>
#include <vector>
namespace libvariant {

	/// \brief Exception thrown when a key does not exist.
	class KeyError : public std::runtime_error {
	public:
		KeyError(const std::string &key) throw();
	};

	/// \brief Exception thrown when internal state is corrupt.
	class UnknownTypeError : public std::runtime_error {
	public:
		UnknownTypeError(VariantDefines::Type_t type) throw();
	};

	/// \brief Exception thrown when a function expects a specific type but the action type conflicts.
	class UnexpectedTypeError : public std::runtime_error {
	public:
		UnexpectedTypeError(VariantDefines::Type_t expected, VariantDefines::Type_t value) throw();
		UnexpectedTypeError(const std::vector<VariantDefines::Type_t> &expected, VariantDefines::Type_t value) throw();
	};

	/// \brief Exception thrown when a function cannot convert the Variant to an acceptable type for the routine.
	class UnableToConvertError : public std::runtime_error {
	public:
		UnableToConvertError(VariantDefines::Type_t sourcetype, const std::string &targettype) throw();
	};

	/// \brief Exception thrown when trying to index a type other than map or list.
	class TypeNotIndexableError : public std::runtime_error {
	public:
		TypeNotIndexableError(VariantDefines::Type_t type) throw();
	};

	/// \brief Exception thrown when attempting a numeric operation on a non numeric type.
	class NotNumericTypeError : public std::runtime_error {
	public:
		NotNumericTypeError(VariantDefines::Type_t type) throw();
	};

	/// \brief Exception thrown from path functions
	class BadPathError : public std::runtime_error {
	public:
		BadPathError(VariantDefines::Type_t type) throw();
		BadPathError(const std::string &msg) throw();
	};

	class InvalidReferenceError : public std::runtime_error {
	public:
		InvalidReferenceError(const std::string &msg) throw();
	};

	class InvalidOperationError : public std::runtime_error {
	public:
		InvalidOperationError(VariantDefines::Type_t type, const std::string &opname) throw();
	};

	std::ostream &operator <<(std::ostream &os, VariantDefines::Type_t type);

	void AddStackTrace(std::ostream &os, int level);
}
#endif
