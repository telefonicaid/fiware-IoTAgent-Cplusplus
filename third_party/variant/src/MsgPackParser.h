/** \file
 * \author John Bridgman
 * \brief 
 */

#ifndef VARIANT_MSGPACKPARSER_H
#define VARIANT_MSGPACKPARSER_H
#pragma once
#include <Variant/ParserInput.h>
#include <Variant/Parser.h>
#include <msgpack.h>

struct vmpu_context;

namespace libvariant {

	class MsgPackParserImpl : public ParserImpl {
	public:
		enum State_t {
			START,
			OK,
			END,
			DONE,
			ERROR
		};

		MsgPackParserImpl(shared_ptr<ParserInput> in);
		virtual ~MsgPackParserImpl();

		virtual int Run();

		virtual bool Done() const { return state == DONE; }
		virtual bool Error() const { return state == ERROR; }
		virtual bool Ok() const { return state == OK; }
		/// Return a string describing the last error suitable for presentation to a user
		virtual std::string ErrorStr() const { return errorstr; }
		/// Reset the parser
		virtual void Reset();
	private:
		State_t state;
		shared_ptr<ParserInput> input;
		shared_ptr<vmpu_context> ctx;
		std::string errorstr;
		unsigned bytecount;
	};
}

#endif
