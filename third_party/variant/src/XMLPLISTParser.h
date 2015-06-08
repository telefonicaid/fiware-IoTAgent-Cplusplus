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
#ifndef VARIANT_XMLPLISTPARSER_H
#define VARIANT_XMLPLISTPARSER_H
#pragma once
#include <deque>
#include <libxml/xmlreader.h>
#include <Variant/Parser.h>
#include <Variant/ParserInput.h>
#include <Variant/SharedPtr.h>


namespace libvariant {

	class XMLPLISTParserImpl : public ParserImpl {
		enum State_t {
			StartState,
			NullNode,
			RootNode,
			TrueNode,
			FalseNode,
			IntegerNode,
			FloatNode,
			StringNode,
			BlobNode,
			MapNode,
			EmptyMapNode,
			KeyNode,
			ListNode,
			EmptyListNode,
			EndState
		};
	public:
		XMLPLISTParserImpl(shared_ptr<ParserInput> in);
		virtual ~XMLPLISTParserImpl();
		virtual int Run();
		virtual bool Done() const;
		virtual bool Error() const;
		virtual bool Ok() const;
		virtual std::string ErrorStr() const;
		virtual void Reset();
	private:
		bool BeginElement(ParserActions *action, State_t nodetype);
		bool ProcessText(ParserActions *action);
		bool EndElement(ParserActions *action, State_t nodetype);
		void Close();
		void HandleError(const char *fnname);

		static int do_read(void *ctx, char *buffer, int len);
		static int do_close(void *ctx);
		static State_t StringToNode(const char *name);

		std::deque<State_t> statestack;
		xmlTextReaderPtr reader;
		shared_ptr<ParserInput> input;
		bool closed;
		bool empty;
		bool err;
		std::string errorstr;
	};
}
#endif
