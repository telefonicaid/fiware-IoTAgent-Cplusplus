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

#include <Variant/Parser.h>
#include <Variant/ParserInput.h>
#include <stdexcept>
#include "JSONParser.h"
#ifdef ENABLE_YAML
#include "YAMLParser.h"
#endif
#ifdef ENABLE_XML
#include "XMLPLISTParser.h"
#endif
#ifdef ENABLE_MSGPACK
#include "MsgPackParser.h"
#endif
#include "BundleHdrParser.h"
#include <Variant/GuessFormat.h>
namespace libvariant {

	ParserActions::~ParserActions() {}

	ParserImpl::~ParserImpl() {}

	void ParserImpl::Reset() {
		throw std::runtime_error("Parser does not support reset.");
	}

	Parser::Parser() {}
	Parser::Parser(shared_ptr<ParserImpl> pi) : impl(pi) {}
	Parser::~Parser() {}

	shared_ptr<ParserInput> CreateParserInput(const std::string &str) {
		return CreateParserInput(str.c_str(), str.length());
	}

	shared_ptr<ParserInput> CreateParserInput(const char *str) {
		return shared_ptr<ParserInput>(new ParserMemoryInput(str, strlen(str)));
	}
	shared_ptr<ParserInput> CreateParserInput(const void *ptr, unsigned len) {
		return shared_ptr<ParserInput>(new ParserMemoryInput(ptr, len));
	}
	shared_ptr<ParserInput> CreateParserInputFile(const char *filename) {
		return shared_ptr<ParserInput>(new ParserFilenameInput(filename));
	}

	shared_ptr<ParserInput> CreateParserInputFile(FILE *f) {
		return shared_ptr<ParserInput>(new ParserFileInput(f));
	}

	shared_ptr<ParserInput> CreateParserInputFile(std::streambuf *sb) {
		return shared_ptr<ParserInput>(new ParserStreambufInput(sb));
	}

	Parser CreateParser(shared_ptr<ParserInput> i, SerializeType type) {
		switch (type) {
		case SERIALIZE_XMLPLIST:
			return XMLPLISTParser(i);
		case SERIALIZE_JSON:
			return JSONParser(i);
		case SERIALIZE_BUNDLEHDR:
			return BundleHdrParser(i);
		case SERIALIZE_GUESS:
			return CreateParserGuess(i);
		case SERIALIZE_MSGPACK:
			return MsgPackParser(i);
		case SERIALIZE_YAML:
		default:
			return YAMLParser(i);
		}
	}

	Parser CreateParserGuess(shared_ptr<ParserInput> i) {
		SerializeType type = GuessFormat(i.get());
		return CreateParser(i, type);
	}

	Parser JSONParser(shared_ptr<ParserInput> i) {
		return Parser(shared_ptr<ParserImpl>(new JSONParserImpl(i)));
	}
	Parser YAMLParser(shared_ptr<ParserInput> i) {
#ifdef ENABLE_YAML
		return Parser(shared_ptr<ParserImpl>(new YAMLParserImpl(i)));
#else
		throw std::runtime_error("libvariant not compiled with YAML support.");
#endif
	}
	Parser XMLPLISTParser(shared_ptr<ParserInput> i) {
#ifdef ENABLE_XML
		return Parser(shared_ptr<ParserImpl>(new XMLPLISTParserImpl(i)));
#else
		throw std::runtime_error("libvariant not compiled with plist xml support.");
#endif
	}

	Parser BundleHdrParser(shared_ptr<ParserInput> i) {
		return Parser(shared_ptr<ParserImpl>(new BundleHdrParserImpl(i)));
	}

	Parser MsgPackParser(shared_ptr<ParserInput> i) {
#ifdef ENABLE_MSGPACK
		return Parser(shared_ptr<ParserImpl>(new MsgPackParserImpl(i)));
#else
		throw std::runtime_error("libvariant not compiled with msgpack support.");
#endif
	}
}
