/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_PARSER_H
#define VARIANT_PARSER_H
#pragma once
#include <stdexcept>
#include <stdint.h>
#include <Variant/Blob.h>
#include <Variant/ParserInput.h>
#include <Variant/Variant.h>
#include <deque>

namespace libvariant {

	class ParserImpl;

	class ParserActions {
	public:
		virtual ~ParserActions();
		virtual void BeginDocument(ParserImpl *p)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void EndDocument(ParserImpl *p)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		// If length < 0, unknown length
		virtual void BeginMap(ParserImpl *p, int length, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void EndMap(ParserImpl *p)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		// If length < 0, unknown length
		virtual void BeginList(ParserImpl *p, int length, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void EndList(ParserImpl *p)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Alias(ParserImpl *p, const char *anchor)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, double v, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, const char *str, unsigned length, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, bool v, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Null(ParserImpl *p, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, intmax_t v, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, uintmax_t v, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
		virtual void Scalar(ParserImpl *p, BlobPtr b, const char *anchor, const char *tag)
		{ throw std::runtime_error("Parser: Unexpected event"); }
	};

	/// The class that parser implementations override
	class ParserImpl {
	public:
		virtual ~ParserImpl();
		/**
		 * Attempt to parse some actions.
		 * At least one ParserActions should be on the action stack.
		 *
		 * Run is not guaranteed to be recursively callable from inside a
		 * ParserActions callback. Do not do that.
		 *
		 * return 0 on success
		 * return 1 when done
		 * return -1 if needing more data is detected (not all parsers support this; can be treated as an error)
		 * throw on error
		 */
		virtual int Run() = 0;

		/**
		 * Push a new ParserActions handler onto the action handler stack.
		 */
		virtual void PushAction(shared_ptr<ParserActions> action) { action_stack.push_back(action); }

		/**
		 * Pop the top ParserActions handler off the action handler stack and
		 * return it.
		 */
		virtual shared_ptr<ParserActions> PopAction() {
			if (action_stack.empty())
		   	{ throw std::runtime_error("Parser: Attempting to parse an actions without any action handlers"); }
			shared_ptr<ParserActions> ret = action_stack.back();
			action_stack.pop_back();
			return ret;
		}

		/**
		 * Access the top ParserActions
		 */
		ParserActions *TopAction() {
			if (action_stack.empty())
		   	{ throw std::runtime_error("Parser: Attempting to parse an actions without any action handlers"); }
		   	return action_stack.back().get();
	   	}

		/** These three functiosn can be used to query the state
		 * later.
		 * Done means that a call to Process will immediately return 1
		 * Error if Process threw an exception last
		 * Ok if everything looks good (Process never been called, or last return 0 or possibly -1)
		 */
		virtual bool Done() const = 0;
		virtual bool Error() const = 0;
		virtual bool Ok() const = 0;
		/// Return a string describing the last error suitable for presentation to a user
		virtual std::string ErrorStr() const = 0;
		/// Reset the parser
		virtual void Reset() = 0;
	protected:
		std::deque< shared_ptr< ParserActions > > action_stack;
	};

	class Parser {
	public:
		Parser();
		Parser(shared_ptr<ParserImpl> pi);
		~Parser();
		/// 0 on success, 1 when done, -1 other problem, throw on error
		int Run() { return impl->Run(); }
		/// Push an action handler onto the action stack.
		void PushAction(shared_ptr<ParserActions> action) { impl->PushAction(action); }
		/// Pop the top action handler off the stack.
		shared_ptr<ParserActions> PopAction() { return impl->PopAction(); }

		/// Query state
		bool Done() const { return impl->Done(); }
		bool Error() const { return impl->Error(); }
		bool Ok() const { return impl->Ok(); }
		/// Return a string describing the last error suitable for presentation to a user
		std::string ErrorStr() const { return impl->ErrorStr(); }
		/// Reset the parser if applicable (some types will throw)
		void Reset() { impl->Reset(); }

		/// Internal/advanced function to get the underlying implementation
		shared_ptr<ParserImpl> GetImpl() { return impl; }
	private:
		shared_ptr<ParserImpl> impl;
	};

	shared_ptr<ParserInput> CreateParserInput(const std::string &str);
	shared_ptr<ParserInput> CreateParserInput(const char *str);
	shared_ptr<ParserInput> CreateParserInput(const void *ptr, unsigned len);
	shared_ptr<ParserInput> CreateParserInputFile(const char *filename);
	shared_ptr<ParserInput> CreateParserInputFile(FILE *f);
	shared_ptr<ParserInput> CreateParserInputFile(std::streambuf *sb);

	Parser CreateParser(shared_ptr<ParserInput> i, SerializeType type);
	Parser CreateParserGuess(shared_ptr<ParserInput> i);
	Parser JSONParser(shared_ptr<ParserInput> i);
	Parser YAMLParser(shared_ptr<ParserInput> i);
	Parser XMLPLISTParser(shared_ptr<ParserInput> i);
	Parser BundleHdrParser(shared_ptr<ParserInput> i);
	Parser MsgPackParser(shared_ptr<ParserInput> i);
}
#endif
