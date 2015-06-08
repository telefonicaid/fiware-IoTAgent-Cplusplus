/** \file
 * \author John Bridgman
 * \brief A parser for the Bundle header.
 */
#ifndef VARIANT_BUNDLEHDRPARSER_H
#define VARIANT_BUNDLEHDRPARSER_H
#pragma once
#include <Variant/Parser.h>
#include <Variant/SharedPtr.h>
#include <deque>
namespace libvariant {

	/**
	 * A simple parser for the Bundle header.
	 *
	 * Maintanance programmer note: BundleParserImpl
	 * relies on a lot of the implementation details of this class.
	 */ 
	class BundleHdrParserImpl : public ParserImpl {
	protected:
		enum State_t {
			START,
			BEGIN_MAP,
			KEY,
			VALUE,
			LIST,
			END_MAP,
			END,
			STOP
		};

	public:
		BundleHdrParserImpl(shared_ptr<ParserInput> i);
		virtual ~BundleHdrParserImpl();

		virtual int Run();

		unsigned GetLine() const;
		/**
		 * Resets the parser to the state it was in when just constructed.
		 */
		virtual void Reset();

		virtual bool Done() const;
		virtual bool Error() const;
		virtual bool Ok() const;
		virtual std::string ErrorStr() const;
	protected:
		void ReadLine();
		void RetireLine();
		bool ScanLine();

		shared_ptr<ParserInput> input;
		State_t state;
		const char *line;
		unsigned line_len;
		std::vector<char> key;
		std::vector<char> value;
		std::deque<char*> list;
		unsigned line_num;
		std::string errorstr;
	};

}
#endif
