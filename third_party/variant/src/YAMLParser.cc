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
#include "YAMLParser.h"
#include <algorithm>
#include "Base64.h"
#include "GuessScalar.h"
#include <new>
#include <stdlib.h>
#include <sstream>

namespace libvariant {

	int YAMLParserImpl::ParserReadHandler(void *data, unsigned char *buffer, size_t size, size_t *sizein) {
		YAMLParserImpl *impl = (YAMLParserImpl*)data;
		unsigned len = size;
		const void *ptr = impl->input->GetPtr(len);
		if (!ptr) {
			*sizein = 0;
		   	return 1;
	   	}
		*sizein = std::min<unsigned>(size, len);
		memcpy(buffer, ptr, *sizein);
		impl->input->Release(*sizein);
		return 1;
	}

	YAMLParserImpl::YAMLParserImpl(shared_ptr<ParserInput> in)
		: input(in), eof(false), err(false)
	{
		yaml_parser_initialize(&parser);
		yaml_parser_set_input(&parser, &ParserReadHandler, this);
	}

	YAMLParserImpl::~YAMLParserImpl() {
		yaml_parser_delete(&parser);
	}

	void ParseScalar(yaml_event_t &event, ParserImpl *p, ParserActions *action) {
		const char *value = (const char *)event.data.scalar.value;
		const char *anchor = (const char *)event.data.scalar.anchor;
		const char *tag = (const char *)event.data.scalar.tag;
		unsigned length = event.data.scalar.length;
		yaml_scalar_style_t style = event.data.scalar.style;
		if (tag && strcmp(tag, "tag:yaml.org,2002:binary") == 0) {
			void *data = 0;
			if (posix_memalign(&data, 64, Base64DecodeSize(length)) != 0) {
				throw std::bad_alloc();
			}
			unsigned len = Base64Decode(data, value, length);
			action->Scalar(p, Blob::CreateFree(data, len), anchor, tag);
			return;
		}
		switch (style) {
		case YAML_ANY_SCALAR_STYLE:
		case YAML_PLAIN_SCALAR_STYLE:
			break;
		case YAML_SINGLE_QUOTED_SCALAR_STYLE:
		case YAML_DOUBLE_QUOTED_SCALAR_STYLE:
		case YAML_LITERAL_SCALAR_STYLE:
		case YAML_FOLDED_SCALAR_STYLE:
			action->Scalar(p, value, length, anchor, tag);
			return;
		}
		if (tag) {
			action->Scalar(p, value, length, anchor, tag);
		} else {
			GuessScalar(value, length, anchor, tag, p, action);
		}
	}

	int YAMLParserImpl::Run() {
		bool loop = true;
		while (loop && !eof && !action_stack.empty()) {
			yaml_event_t event;
			if (yaml_parser_parse(&parser, &event) == 0) {
				err = true;
				std::ostringstream oss;
				oss << "Error parsing YAML: " << parser.problem << " "
					<< (parser.context ? parser.context : "<NULL context>")
					<< " at line " << parser.problem_mark.line << " column "
					<< parser.problem_mark.column << "\n";
				errorstr = oss.str();
				throw std::runtime_error(errorstr);
			}

			try {
				switch (event.type) {
				/** An empty event. */
				case YAML_NO_EVENT:
					break;

				/** A STREAM-START event. */
				case YAML_STREAM_START_EVENT:
					break;
				/** A STREAM-END event. */
				case YAML_STREAM_END_EVENT:
					eof = true;
					loop = false;
					break;

				/** A DOCUMENT-START event. */
				case YAML_DOCUMENT_START_EVENT:
					TopAction()->BeginDocument(this);
					break;
				/** A DOCUMENT-END event. */
				case YAML_DOCUMENT_END_EVENT:
					TopAction()->EndDocument(this);
					loop = false;
					break;

				/** An ALIAS event. */
				case YAML_ALIAS_EVENT:
					TopAction()->Alias(this, (const char*)event.data.alias.anchor);
					break;
				/** A SCALAR event. */
				case YAML_SCALAR_EVENT:
					ParseScalar(event, this, TopAction());
					break;

				/** A SEQUENCE-START event. */
				case YAML_SEQUENCE_START_EVENT:
					TopAction()->BeginList(this, -1, (const char*)event.data.sequence_start.anchor,
							(const char*)event.data.sequence_start.tag);
					break;
				/** A SEQUENCE-END event. */
				case YAML_SEQUENCE_END_EVENT:
					TopAction()->EndList(this);
					break;

				/** A MAPPING-START event. */
				case YAML_MAPPING_START_EVENT:
					TopAction()->BeginMap(this, -1, (const char*)event.data.mapping_start.anchor,
							(const char*)event.data.mapping_start.tag);
					break;
				/** A MAPPING-END event. */
				case YAML_MAPPING_END_EVENT:
					TopAction()->EndMap(this);
					break;

				}
			} catch (...) {
				err = true;
				errorstr = "Exception from action...";
				yaml_event_delete(&event);
				throw;
			}
			yaml_event_delete(&event);
		}
		return eof ? 1 : 0;
	}

	bool YAMLParserImpl::Done() const {
		return eof;
	}

	bool YAMLParserImpl::Error() const {
		return err;
	}

	bool YAMLParserImpl::Ok() const {
		return !err && !eof;
	}

	std::string YAMLParserImpl::ErrorStr() const {
		return errorstr;
	}

	void YAMLParserImpl::Reset() {
		eof = false;
		err = false;
		yaml_parser_delete(&parser);
		yaml_parser_initialize(&parser);
		yaml_parser_set_input(&parser, &ParserReadHandler, this);
	}
}
