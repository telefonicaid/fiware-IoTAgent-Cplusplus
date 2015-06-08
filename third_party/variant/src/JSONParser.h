//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
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
 */
#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP
#pragma once
#include "Base64.h"
#include <deque>
#include "JSON_parser.h"
#include <new>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <Variant/Parser.h>
#include <Variant/ParserInput.h>
#include <Variant/SharedPtr.h>


namespace libvariant {
	/**
	 * A class that encapsulates the JSON_parser with the Parser interface
	 */
	class JSONParserImpl : public ParserImpl {
	public:
		enum Status_t {
			S_START,
			S_BEGIN,
			S_OK,
			S_ERROR,
			S_END,
			S_DONE
		};
		JSONParserImpl(shared_ptr<ParserInput> i);
		virtual ~JSONParserImpl();

		virtual int Run();

		/**
		 * \return the status of the parser.
		 */
		Status_t GetStatus() const;
		virtual bool Done() const;
		virtual bool Error() const;
		virtual bool Ok() const;
		virtual std::string ErrorStr() const;
		unsigned GetLine() const;
		unsigned GetColumn() const;
		unsigned GetByteCount() const;
		/**
		 * Resets the parser to the state it was in when just constructed.
		 */
		virtual void Reset();
	private:

		void AllocParser();

		void Parse();

		static int StaticCallback(void *ctx, int type, const struct JSON_value_struct* value);

		JSON_parser_struct *parser;
		Status_t status;
		unsigned line;
		unsigned column;
		unsigned charcount;
		unsigned depth;
		shared_ptr<ParserInput> input;
		std::string errorstr;
	};
}
#endif
