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
#ifndef VARIANT_PATH_H
#define VARIANT_PATH_H
#pragma once
#include <deque>
#include <string>
#include <Variant/Exceptions.h>
namespace libvariant {

	// A simple xpath like path functionality for Variant
	//
	// Map keys are specified using a "/" then the key.
	// List indeces are specified using [index]
	// A "/" at the end of the path is ignored.
	// Characters "\/[]" must all be escaped with "\"
	//
	// For example:
	// {
	//   "key1": [ 1, 2, 3, { "blork": true } ],
	//   "key2": {
	//     "subkey": [ "a", { "furple": 1 }, 2 ],
	//     "sub/key2": true
	//     }
	// }
	//
	// Then all paths enumerated are:
	// Path						Value
	// "/"						Identity
	// "/key1"					[ 1, 2, 3, { "blork": true } ],
	// "/key1[0]"				1
	// "/key1[1]"				2
	// "/key1[2]"				3
	// "/key1[3]"				{ "blork": true }
	// "/key1[3]/blork"			true
	// "/key2"					{ "subkey": [ "a", { "furple": 1 }, 2 ], "sub/key2": true  }
	// "/key2/subkey"			[ "a", { "furple": 1 }, 2 ]
	// "/key2/subkey[0]"		"a"
	// "/key2/subkey[1]"		{ "furple": 1 }
	// "/key2/subkey[1]/furple"	1
	// "/key2/subkey[2]"		2
	// "/key2/sub\/key2"		true
	//
	// For all these functions PathList must act like a vector<Variant>
	// It may be of some other type (like deque<Variant>)

	class PathElement {
	public:
		enum Type_t { INDEX, KEY };
		PathElement(const std::string &k) : type(KEY), key(k) {}
		PathElement(const char *k) : type(KEY), key(k) {}
		PathElement(unsigned i) : type(INDEX), index(i) {}

		bool IsNumber() const { return type == INDEX; }
		unsigned AsUnsigned() const {
		   	if (type != INDEX) throw BadPathError("Variant path element is not a number");
			return index;
		}
		bool IsString() const { return type == KEY; }
		const std::string &AsString() const {
			if (type != KEY) throw BadPathError("Variant path element is not a key");
			return key;
		}
	private:
		Type_t type;
		unsigned index;
		std::string key;
	};

	typedef std::deque<PathElement> Path;

	//!
	// Parse the path string into path tokens.
	void ParsePath(Path &parsed_path, const std::string &path);
	Path ParsePath(const std::string &path);

	//! Take the set of path tokens and turn it into a path string
	std::string PathString(const Path &path);

}
#endif
