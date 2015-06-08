//=============================================================================
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
 * \brief 
 */


#ifndef  YAMLPARSER_INC
#define  YAMLPARSER_INC
#pragma once
#include <Variant/Parser.h>
#include <Variant/ParserInput.h>
#include <Variant/SharedPtr.h>
#include <yaml.h>

namespace libvariant {


	/**
	 * \brief 
	 */
	class YAMLParserImpl : public ParserImpl {
	public:
		YAMLParserImpl(shared_ptr<ParserInput> in);
		virtual ~YAMLParserImpl();

		virtual int Run();

		virtual bool Done() const;
		virtual bool Error() const;
		virtual bool Ok() const;
		virtual std::string ErrorStr() const;
		virtual void Reset();
	private:
		static int ParserReadHandler(void *data, unsigned char *buffer, size_t size, size_t *sizein);
		yaml_parser_t parser;
		shared_ptr<ParserInput> input;
		bool eof;
		bool err;
		std::string errorstr;
	};

}

#endif
