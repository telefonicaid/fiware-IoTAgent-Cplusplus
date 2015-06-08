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
#include "XMLPLISTParser.h"
#include <errno.h>
#include <limits.h>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include "XMLPLISTDefs.h"
#include "Base64.h"

namespace libvariant {

	XMLPLISTParserImpl::State_t XMLPLISTParserImpl::StringToNode(const char *name) {
		if (!name) {
			return StringNode;
		} else if (strcasecmp(name, STRING_NAME) == 0) {
			return StringNode;
		} else if (strcasecmp(name, TRUE_NAME) == 0) {
			return TrueNode;
		} else if (strcasecmp(name, ROOT_NAME) == 0) {
			return RootNode;
		} else if (strcasecmp(name, LIST_NAME) == 0) {
			return ListNode;
		} else if (strcasecmp(name, FALSE_NAME) == 0) {
			return FalseNode;
		} else if (strcasecmp(name, INTEGER_NAME) == 0) {
			return IntegerNode;
		} else if (strcasecmp(name, FLOAT_NAME) == 0) {
			return FloatNode;
		} else if (strcasecmp(name, NULL_NAME) == 0) {
			return NullNode;
		} else if (strcasecmp(name, MAP_NAME) == 0) {
			return MapNode;
		} else if (strcasecmp(name, KEY_NAME) == 0) {
			return KeyNode;
		} else if (strcasecmp(name, BLOB_NAME) == 0) {
			return BlobNode;
		} else {
			return StringNode;
		}
	}

	int XMLPLISTParserImpl::Run() {
		while (!action_stack.empty()) {
			if (closed) { return 1; }
			bool loop = true;
			switch (statestack.back()) {
			case StartState:
				statestack.back() = EndState;
				break;
			case EndState:
				return 1;
			case EmptyMapNode:
				statestack.pop_back();
				TopAction()->EndMap(this);
				return 0;
			case EmptyListNode:
				statestack.pop_back();
				TopAction()->EndList(this);
				return 0;
			case RootNode:
			case MapNode:
			case ListNode:
				break;
			default:
				err = true;
				errorstr = "libvariant XML PLIST parser in an inconsistent state";
				throw std::runtime_error(errorstr);
			}
			while (loop) {
				int ret = xmlTextReaderRead(reader);
				if (ret == 1) {
					int node_type = xmlTextReaderNodeType(reader);
					const char *name = (const char*)xmlTextReaderConstName(reader);
					switch (node_type) {
					case XML_READER_TYPE_ELEMENT:
						loop = BeginElement(TopAction(), StringToNode(name));
						break;
					case XML_READER_TYPE_END_ELEMENT:
						loop = EndElement(TopAction(), StringToNode(name));
						break;
					case XML_READER_TYPE_TEXT:
						loop = ProcessText(TopAction());
						break;
					case XML_READER_TYPE_END_ENTITY:
					case XML_READER_TYPE_ATTRIBUTE:
					case XML_READER_TYPE_CDATA:
					case XML_READER_TYPE_ENTITY_REFERENCE:
					case XML_READER_TYPE_ENTITY:
					case XML_READER_TYPE_PROCESSING_INSTRUCTION:
					case XML_READER_TYPE_COMMENT:
					case XML_READER_TYPE_DOCUMENT:
					case XML_READER_TYPE_DOCUMENT_TYPE:
					case XML_READER_TYPE_DOCUMENT_FRAGMENT:
					case XML_READER_TYPE_NOTATION:
					case XML_READER_TYPE_WHITESPACE:
					case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
					case XML_READER_TYPE_XML_DECLARATION:
					case XML_READER_TYPE_NONE:
					default:
						break;
					}
				} else if (ret == 0) {
					if (statestack.back() != EndState) {
						err = true;
						errorstr = "libvariant XML PLIST parser in inconsistent state";
						throw std::runtime_error(errorstr);
					}
					return 1;
				} else {
					HandleError("xmlTextReaderRead");
				}
			}
		}
		return 0;
	}

	bool XMLPLISTParserImpl::BeginElement(ParserActions *action, State_t nodetype) {
		bool empty_node = xmlTextReaderIsEmptyElement(reader) != 0;
		switch (nodetype) {
		case NullNode:
			if (!empty_node) { statestack.push_back(nodetype); }
			action->Null(this, 0, 0);
			return !empty_node;
		case RootNode:
			statestack.push_back(nodetype);
			action->BeginDocument(this);
			return !empty_node;
		case TrueNode:
			if (!empty_node) { statestack.push_back(nodetype); }
			action->Scalar(this, true, 0, 0);
			return !empty_node;
		case FalseNode:
			if (!empty_node) { statestack.push_back(nodetype); }
			action->Scalar(this, false, 0, 0);
			return !empty_node;
		case IntegerNode:
			if (!empty_node) {
				statestack.push_back(nodetype);
				empty = true;
			} else {
				err = true;
				errorstr = "libvariant XML PLIST does not allow empty interger elements";
				throw std::runtime_error(errorstr);
			}
			return true;
		case FloatNode:
			if (!empty_node) {
				statestack.push_back(nodetype);
				empty = true;
			} else {
				err = true;
				errorstr = "libvariant XML PLIST does not allow empty real elements";
				throw std::runtime_error(errorstr);
			}
			return true;
		case StringNode:
			if (!empty_node) {
				statestack.push_back(nodetype);
				empty = true;
			} else {
				err = true;
				errorstr = "libvariant XML PLIST does not allow empty string elements";
				throw std::runtime_error(errorstr);
			}
			return true;
		case BlobNode:
			if (!empty_node) {
				statestack.push_back(nodetype);
				empty = true;
			} else {
				err = true;
				errorstr = "libvariant XML PLIST does not allow empty data elements";
				throw std::runtime_error(errorstr);
			}
			return true;
		case MapNode:
			if (empty_node) { statestack.push_back(EmptyMapNode); }
			else { statestack.push_back(nodetype); }
			action->BeginMap(this, -1, 0, 0);
			return false;
		case KeyNode:
			{
				if (empty_node) {
					err = true;
					errorstr = "libvariant XML PLIST does not allow empty key elements";
					throw std::runtime_error(errorstr);
				}
				statestack.push_back(nodetype);
				empty = true;
			}
			return true;
		case ListNode:
			if (empty_node) { statestack.push_back(EmptyListNode); }
			else { statestack.push_back(nodetype); }
			action->BeginList(this, -1, 0, 0);
			return false;
		default:
			err = true;
			errorstr = "Invalid libvariant XML PLIST parser state";
			throw std::runtime_error(errorstr);
		}
	}

	bool XMLPLISTParserImpl::ProcessText(ParserActions *action) {
		switch (statestack.back()) {
		case NullNode:
		case RootNode:
		case TrueNode:
		case FalseNode:
		case MapNode:
		case ListNode:
			return true;
		case IntegerNode:
			{
				empty = false;
				const char *value = (const char*)xmlTextReaderConstValue(reader);
				intmax_t val;
				val = strtoll(value, 0, 0);
				if (val == LLONG_MAX && errno == ERANGE) {
					uintmax_t v;
					v = strtoull(value, 0, 0);
					action->Scalar(this, v, 0, 0);
				} else {
					action->Scalar(this, val, 0, 0);
				}
			}
			return true;
		case FloatNode:
			{
				empty = false;
				const char *value = (const char*)xmlTextReaderConstValue(reader);
				double val = strtod(value, 0);
				action->Scalar(this, val, 0, 0);
			}
			return true;
		case StringNode:
			{
				empty = false;
				const char *value = (const char*)xmlTextReaderConstValue(reader);
				action->Scalar(this, value, strlen(value), 0, 0);
			}
			return true;
		case BlobNode:
			{
				empty = false;
				const char *value = (const char*)xmlTextReaderConstValue(reader);
				unsigned length = strlen(value);
				void *data = 0;
				if (posix_memalign(&data, 64, Base64DecodeSize(length)) != 0) {
					err = true;
					errorstr = "memory allocation failure";
					throw std::bad_alloc();
				}
				unsigned len = Base64Decode(data, value, length);
				action->Scalar(this, Blob::CreateFree(data, len), 0, 0);
			}
			return true;
		case KeyNode:
			{
				empty = false;
				const char *value = (const char*)xmlTextReaderConstValue(reader);
				action->Scalar(this, value, strlen(value), 0, 0);
			}
			return true;
		default:
			err = true;
			errorstr = "Invalid libvariant XML PLIST parser state";
			throw std::runtime_error(errorstr);
		}

	}

	bool XMLPLISTParserImpl::EndElement(ParserActions *action, State_t nodetype) {
		if (statestack.back() != nodetype) {
			err = true;
			errorstr = "libvariant XML PLIST parser in inconsistent state";
			throw std::runtime_error(errorstr);
		}
		statestack.pop_back();
		switch (nodetype) {
		case NullNode:
		case TrueNode:
		case FalseNode:
			return false;
		case IntegerNode:
		case FloatNode:
			if (empty) {
				err = true;
				errorstr = "libvariant XML PLIST parser cannot have empty numeric nodes";
				throw std::runtime_error(errorstr);
			}
			return false;
		case BlobNode:
			if (empty) {
				err = true;
				errorstr = "libvariant XML PLIST parser cannot have empty data nodes";
				throw std::runtime_error(errorstr);
			}
			return false;
		case StringNode:
		case KeyNode:
			if (empty) { action->Scalar(this, "", 0, 0, 0); }
			return false;
		case RootNode:
			action->EndDocument(this);
			return false;
		case MapNode:
			action->EndMap(this);
			return false;
		case ListNode:
			action->EndList(this);
			return false;
		default:
			err = true;
			errorstr = "Invalid libvariant XML PLIST parser state";
			throw std::runtime_error(errorstr);
		}
	}

	void XMLPLISTParserImpl::Close() {
		if (!closed) {
			closed = true;
			xmlTextReaderClose(reader);
		}
	}

	void XMLPLISTParserImpl::HandleError(const char *fnname) {
		xmlErrorPtr error = xmlGetLastError();
		err = true;
		if (error) {
			errorstr = error->message;
			throw std::runtime_error(error->message);
		} else {
			errorstr = fnname;
			throw std::runtime_error(fnname);
		}
	}

	int XMLPLISTParserImpl::do_read(void *ctx, char *buffer, int len) {
		XMLPLISTParserImpl *impl = (XMLPLISTParserImpl*)ctx;
		unsigned l = len;
		const void *ptr = impl->input->GetPtr(l);
		if (!ptr) { return 0; }
		l = std::min<unsigned>(len, l);
		memcpy(buffer, ptr, l);
		impl->input->Release(l);
		return l;
	}

	int XMLPLISTParserImpl::do_close(void *ctx) {
		XMLPLISTParserImpl *impl = (XMLPLISTParserImpl*)ctx;
		impl->Close();
		return 0;
	}

	XMLPLISTParserImpl::XMLPLISTParserImpl(shared_ptr<ParserInput> in)
		: input(in), closed(false), err(false)
   	{
		reader = xmlReaderForIO(do_read, do_close, this, 0, 0,
				XML_PARSE_NOENT|XML_PARSE_NOCDATA);
		if (!reader) {
			HandleError("xmlReaderForIO");
		}
		statestack.push_back(StartState);
	}

	XMLPLISTParserImpl::~XMLPLISTParserImpl() {
		Close();
		xmlFreeTextReader(reader);
		reader = 0;
	}

	bool XMLPLISTParserImpl::Done() const {
		return closed;
	}

	bool XMLPLISTParserImpl::Error() const {
		return err;
	}

	bool XMLPLISTParserImpl::Ok() const {
		return !closed && !err;
	}

	std::string XMLPLISTParserImpl::ErrorStr() const {
		return errorstr;
	}

	void XMLPLISTParserImpl::Reset() {
		Close();
		xmlFreeTextReader(reader);
		reader = 0;
		reader = xmlReaderForIO(do_read, do_close, this, 0, 0,
				XML_PARSE_NOENT|XML_PARSE_NOCDATA);
		if (!reader) {
			HandleError("xmlReaderForIO");
		}
		statestack.push_back(StartState);
		closed = false;
		err = false;

	}
}
