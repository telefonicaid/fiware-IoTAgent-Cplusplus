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
 * A simple JSON emitter class to emit valid JSON.
 */
#ifndef VARIANT_JSONEMITTER_H
#define VARIANT_JSONEMITTER_H
#pragma once
#include <string>
#include <deque>
#include <memory>
#include <vector>
#include <Variant/Emitter.h>

namespace libvariant {

	/**
	 * A simple JSON emitter class to make it simple to emit
	 * valid JSON in a program.
	 * Does minimal error checking.
	 * All strings are assumed to be UTF-8 encoded.
	 * 
	 * This class buffers with a small buffer otherwise it would internally
	 * perform many calls to write with 1 or 2 bytes.
	 * Flush automatically happens on EndMap and EndList.
	 *
	 * Supported parameters are:
	 *  "pretty": boolean
	 *  "indent": integer (number of spaces to indent when pretty is on,
	 *  default 4)
	 *  "precision": integer (number of digits for a float)
	 *  "strict": be more strict about emitting correct json
	 */
	class JSONEmitterImpl : public EmitterImpl {
		enum State_t {
			NONE,
			MAP_FIRST,
			IN_MAP_KEY,
			IN_MAP_VALUE,
			LIST_FIRST,
			IN_LIST
		};
	public:
		JSONEmitterImpl(shared_ptr<EmitterOutput> o, Variant params);

		~JSONEmitterImpl();

		virtual void BeginDocument();
		virtual void EndDocument();
		virtual void BeginMap(int length);
		virtual void EndMap();
		virtual void BeginList(int length);
		virtual void EndList();

		virtual void EmitNull();
		virtual void EmitTrue();
		virtual void EmitFalse();
		virtual void Emit(const char *v);
		virtual void Emit(intmax_t v);
		virtual void Emit(uintmax_t v);
		virtual void Emit(double v);
		virtual void Emit(ConstBlobPtr b);

		virtual void Flush();
		virtual void Close();

		virtual Variant GetParam(const std::string &key);
		virtual Variant GetParams();
		virtual void SetParam(const std::string &key, Variant value);
	private:

		void CheckSeparator();
		void EmitIndent();
		void EmitString(const char *text, unsigned len);
		void EmitRaw(const char *text);
		void EmitRaw(const char *text, unsigned len);

		shared_ptr<EmitterOutput> output;
		bool pretty;
		bool strict;
		unsigned num_indent;
		unsigned numeric_precision;
		std::deque<State_t> state;
		std::vector<char> buffer;
		unsigned buffer_len;
	};

}
#endif
