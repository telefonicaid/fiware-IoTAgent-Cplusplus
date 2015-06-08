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
 */
#include "JSONParser.h"
#include "BlobMagic.h"

namespace libvariant {

	int JSONParserImpl::StaticCallback(void *ctx, int type, const struct JSON_value_struct* value) {
		JSONParserImpl *impl = (JSONParserImpl*)ctx;
		if (impl->status == S_BEGIN) {
			impl->status = S_OK;
		}
		// If an exception is thrown from here, the parser will be in an
		// inconsistent state. So, set our state to error.
		try {
			switch(type) {
			case JSON_T_ARRAY_BEGIN:    
				++impl->depth;
				impl->TopAction()->BeginList(impl, -1, 0, 0);
				break;
			case JSON_T_ARRAY_END:
				--impl->depth;
				impl->TopAction()->EndList(impl);
				break;
			case JSON_T_OBJECT_BEGIN:
				++impl->depth;
				impl->TopAction()->BeginMap(impl, -1, 0, 0);
				break;
			case JSON_T_OBJECT_END:
				--impl->depth;
				impl->TopAction()->EndMap(impl);
				break;
			case JSON_T_INTEGER:
				impl->TopAction()->Scalar(impl, intmax_t(value->vu.integer_value), 0, 0);
				break;
			case JSON_T_UNSIGNED:
				impl->TopAction()->Scalar(impl, uintmax_t(value->vu.unsigned_value), 0, 0);
				break;
			case JSON_T_FLOAT:
				impl->TopAction()->Scalar(impl, value->vu.float_value, 0, 0);
				break;
			case JSON_T_NULL:
				impl->TopAction()->Null(impl, 0, 0);
				break;
			case JSON_T_TRUE:
				impl->TopAction()->Scalar(impl, true, 0, 0);
				break;
			case JSON_T_FALSE:
				impl->TopAction()->Scalar(impl, false, 0, 0);
				break;
			case JSON_T_KEY:
				impl->TopAction()->Scalar(impl, value->vu.str.value, value->vu.str.length, 0, 0);
				break;
			case JSON_T_STRING:
				if (value->vu.str.length >= MAGIC_BLOB_LENGTH &&
						memcmp(value->vu.str.value, MAGIC_BLOB_TAG, MAGIC_BLOB_LENGTH) == 0) {
					void *ptr = 0;
					if (posix_memalign(&ptr, 64, Base64DecodeSize(value->vu.str.length)) != 0) {
						throw std::bad_alloc();
					}
					unsigned len = Base64Decode(ptr, value->vu.str.value+MAGIC_BLOB_LENGTH,
							value->vu.str.length-MAGIC_BLOB_LENGTH);
					BlobPtr blob = Blob::CreateFree(ptr, len);
					impl->TopAction()->Scalar(impl, blob, 0, 0);
				} else {
					impl->TopAction()->Scalar(impl, value->vu.str.value, value->vu.str.length, 0, 0);
				}
				break;
			default:
				break;
			}
		} catch (...) {
			impl->status = S_ERROR;
			throw;
		}
		return true;
	}

	JSONParserImpl::JSONParserImpl(shared_ptr<ParserInput> i)
		: parser(0),
		status(S_START),
		line(1),
		column(0),
		charcount(0),
		depth(0),
		input(i)
	{
			AllocParser();
	}

	JSONParserImpl::~JSONParserImpl() {
		delete_JSON_parser(parser);
		parser = 0;
	}

	void JSONParserImpl::Reset() {
		if (parser) { delete_JSON_parser(parser); }
		parser = 0;
		status = S_START;
		line = 1;
		column = 0;
		charcount = 0;
		depth = 0;
		AllocParser();
	}

	void JSONParserImpl::AllocParser() {
		JSON_config config;
		init_JSON_config(&config);
		config.callback = StaticCallback;
		config.callback_ctx = this;
		config.depth = -1;
		config.allow_comments = 1;
		parser = new_JSON_parser(&config);
	}

	void JSONParserImpl::Parse() {
		while ( (status == S_OK || status == S_BEGIN) && !action_stack.empty() ) {
			unsigned len = 0;
			const unsigned char *ptr = (const unsigned char*)input->GetPtr(len);
			const unsigned char *c = ptr;
			const unsigned char *end = ptr + len;
			if (len == 0 || !ptr) { 
				if (JSON_parser_done(parser)) { status = S_END; }
				else { status = S_ERROR; }
				return;
			}
			while ( c != end && ( status == S_OK || status == S_BEGIN) && !action_stack.empty() ) {
				++charcount;
				if (*c == '\n') {
					column = 0;
					++line;
				} else { ++column; }
				if (JSON_parser_char(parser, *c)) {
					if (depth == 0) {
						if (status == S_OK) {
							if (JSON_parser_done(parser)) { status = S_END; }
							else { status = S_ERROR; }
						}
					}
				} else if (status == S_OK) { status = S_ERROR; }
				++c;
			}
			input->Release(c - ptr);
		}
	}

	int JSONParserImpl::Run() {
		while (!action_stack.empty()) {
			switch (status) {
			case S_START:
				TopAction()->BeginDocument(this);
				status = S_BEGIN;
				return 0;
			case S_BEGIN:
				Parse();
				break;
			case S_OK:
				Parse();
				break;
			case S_ERROR:
				{
					std::ostringstream oss;
					oss << "JSONParser: An error occurred on line " << line << " column " << column;
					errorstr = oss.str();
					throw std::runtime_error(errorstr);
				}
			case S_END:
				status = S_DONE;
				TopAction()->EndDocument(this);
				return 1;
			case S_DONE:
				return 1;
			default:
				{
					std::ostringstream oss;
					oss << "JSONParser in inconsistent state (line: " << line << " column: " << column << ")";
					errorstr = oss.str();
					throw std::runtime_error("JSONParser in inconsistent state.");
				}
			}
		}
		return 0;
	}

	JSONParserImpl::Status_t JSONParserImpl::GetStatus() const { return status; }
	bool JSONParserImpl::Done() const { return status == S_DONE; }
	bool JSONParserImpl::Error() const { return status == S_ERROR; }
	bool JSONParserImpl::Ok() const { return status == S_OK; }

	std::string JSONParserImpl::ErrorStr() const { return errorstr; }

	unsigned JSONParserImpl::GetLine() const { return line; }
	unsigned JSONParserImpl::GetColumn() const { return column; }
	unsigned JSONParserImpl::GetByteCount() const { return charcount; }
}
