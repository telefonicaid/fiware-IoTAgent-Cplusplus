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
 * \brief A wrapper class around a libyaml emitter
 */
#include "YAMLEmitter.h"
#include <Variant/EmitterOutput.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include <cmath>
#include "Base64.h"

namespace libvariant {

	static int WriteHandler(void *data, unsigned char *buffer, size_t size) {
		EmitterOutput *out = (EmitterOutput*)data;
		return out->Write((const void*)buffer, size);
	}

	static const char *param_keys[] = { "scalar_style", "map_style", "list_style",
		"implicit_document_begin", "implicit_document_end", "indent", "width",
		"precision", 0 };
	static const char *scalar_style_str[] = { "any", "plain", "single quoted",
		"double quoted", "literal", "folded", 0 };
	static const char *style_str[] = { "any", "block", "flow", 0 };

	YAMLEmitterImpl::Config::Config(Variant params)
		: default_scalar_style(ANY_SCALAR_STYLE),
		default_map_style(ANY_STYLE),
		default_list_style(ANY_STYLE),
		implicit_begin_document(false),
		implicit_end_document(false),
		indent(-1),
		width(-1),
		precision(std::numeric_limits<double>::digits10+2)
	{
		SetParams(params);
	}

	void YAMLEmitterImpl::Config::SetParams(Variant params) {
		if (params.IsMap()) {
			for (int i = 0; param_keys[i]; ++i) {
				const char *key = param_keys[i];
				if (params.Contains(key)) {
					SetParam((ConfigKeys)i, params[key]);
				}
			}
		}
	}

	void YAMLEmitterImpl::Config::SetParam(const std::string &key, Variant value) {
		for (int i = 0; param_keys[i]; ++i) {
			if (param_keys[i] == key) {
				SetParam((ConfigKeys)i, value);
				break;
			}
		}
	}

	void YAMLEmitterImpl::Config::SetParam(ConfigKeys index, Variant value) {
		switch (index) {
		case SCALAR_STYLE:
			{
				std::string v = value.AsString();
				for (int i = 0; scalar_style_str[i]; ++i) {
					const char *style = scalar_style_str[i];
					if (v == style) {
						default_scalar_style = (ScalarStyle)i;
						break;
					}
				}
			}
			break;
		case MAP_STYLE:
			{
				std::string v = value.AsString();
				for (int i = 0; style_str[i]; ++i) {
					const char *style = style_str[i];
					if (v == style) {
						default_map_style = (Style)i;
						break;
					}
				}
			}
			break;
		case LIST_STYLE:
			{
				std::string v = value.AsString();
				for (int i = 0; style_str[i]; ++i) {
					const char *style = style_str[i];
					if (v == style) {
						default_list_style = (Style)i;
						break;
					}
				}
			}
			break;
		case IMPLICIT_DOCUMENT_BEGIN:
			implicit_begin_document = value.AsBool();
			break;
		case IMPLICIT_DOCUMENT_END:
			implicit_end_document = value.AsBool();
			break;
		case INDENT:
			indent = value.AsInt();
			break;
		case WIDTH:
			width = value.AsInt();
			break;
		case PRECISION:
			precision = value.AsUnsigned();
			break;
		default:
			break;
		}
	}

	Variant YAMLEmitterImpl::Config::GetParams() {
		Variant ret = Variant::MapType;
		for (int i = 0; param_keys[i]; ++i) {
			const char *key = param_keys[i];
			ret[key] = GetParam((ConfigKeys)i);
		}
		return ret;
	}

	Variant YAMLEmitterImpl::Config::GetParam(const std::string &key) {
		for (int i = 0; param_keys[i]; ++i) {
			if (param_keys[i] == key) {
				return GetParam((ConfigKeys)i);
			}
		}
		return Variant::NullType;
	}

	Variant YAMLEmitterImpl::Config::GetParam(ConfigKeys index) {
		switch (index) {
		case SCALAR_STYLE:
			return scalar_style_str[index];
		case MAP_STYLE:
			return style_str[index];
		case LIST_STYLE:
			return style_str[index];
		case IMPLICIT_DOCUMENT_BEGIN:
			return implicit_begin_document;
		case IMPLICIT_DOCUMENT_END:
			return implicit_end_document;
		case INDENT:
			return indent;
		case WIDTH:
			return width;
		case PRECISION:
			return precision;
		default:
			return Variant::NullType;
		}
	}


	YAMLEmitterImpl::YAMLEmitterImpl(shared_ptr<EmitterOutput> o, Variant params)
		:out(o), closed(false), in_document(false), conf(params)
	{
		if (yaml_emitter_initialize(&emitter) == 0) {
			throw std::runtime_error("Unable to initialize YAMLEmitter.");
		}
		yaml_emitter_set_output(&emitter, WriteHandler, out.get());
		if (conf.indent > 0) {
			yaml_emitter_set_indent(&emitter, conf.indent);
		}
		if (conf.width > 0) {
			yaml_emitter_set_width(&emitter, conf.width);
		}
		try { EmitStreamStart(); }
		catch (...) {
			yaml_emitter_delete(&emitter);
			throw;
		}
	}

	YAMLEmitterImpl::~YAMLEmitterImpl() {
		try { Close(); }
		catch (const std::runtime_error &) {}
	}

	void YAMLEmitterImpl::BeginDocument() {
		BeginDocument(conf.implicit_begin_document);
	}

	void YAMLEmitterImpl::BeginDocument(bool implicit) {
		CheckClosed();
		CheckOutDocument();
		yaml_event_t event;
		if (yaml_document_start_event_initialize(&event, 0, 0, 0, implicit ? 1 : 0) == 0) {
			throw std::runtime_error("Error initializing document start event.");
		}
		Emit(event);
		in_document = true;
	}

	void YAMLEmitterImpl::EndDocument() {
		EndDocument(conf.implicit_end_document);
	}

	void YAMLEmitterImpl::EndDocument(bool implicit) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_document_end_event_initialize(&event, implicit ? 1 : 0) == 0) {
			throw std::runtime_error("Error initializing document end event.");
		}
		Emit(event);
		in_document = false;
	}

	void YAMLEmitterImpl::BeginMap(int length) {
		BeginMap(length, conf.default_map_style);
	}

	void YAMLEmitterImpl::BeginMap(int, Style style) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_mapping_start_event_initialize(&event, 0, 0, 1,
					(yaml_mapping_style_t)style) == 0) {
			throw std::runtime_error("Unable to initialize mapping start event");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::EndMap() {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_mapping_end_event_initialize(&event) == 0) {
			throw std::runtime_error("Unable to initialize mapping end event.");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::BeginList(int length) {
		BeginList(length, conf.default_list_style);
	}

	void YAMLEmitterImpl::BeginList(int, Style style) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_sequence_start_event_initialize(&event, 0, 0, 1,
					(yaml_sequence_style_t)style) == 0) {
			throw std::runtime_error("Unable to initialize sequence start event");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::EndList() {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_sequence_end_event_initialize(&event) == 0) {
			throw std::runtime_error("Unable to initialize sequence end event.");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::EmitNull() {
		Emit("null", PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::EmitTrue() {
		Emit("true", PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::EmitFalse() {
		Emit("false", PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::Emit(const std::string &v, ScalarStyle style) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_scalar_event_initialize(&event, 0, 0, (yaml_char_t*)v.c_str(), v.length(), 1, 1,
					(yaml_scalar_style_t)style) == 0) {
			throw std::runtime_error("Unable to initialize scalar event.");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::Emit(const char *v) {
		Emit(v, ANY_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::Emit(const char *v, ScalarStyle style) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		if (yaml_scalar_event_initialize(&event, 0, 0, (yaml_char_t*)v, strlen(v), 1, 1,
					(yaml_scalar_style_t)style) == 0) {
			throw std::runtime_error("Unable to initialize scalar event.");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::Emit(intmax_t v) {
		std::ostringstream oss;
		oss << v;
		Emit(oss.str(), PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::Emit(uintmax_t v) {
		std::ostringstream oss;
		oss << v;
		Emit(oss.str(), PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::Emit(double v) {
		std::ostringstream oss;
		if (v != v) {
			oss << ".nan";
		} else if (std::abs(v) == std::numeric_limits<double>::infinity()) {
			oss << (v < 0 ? "-" : "") << ".inf";
		} else {
			oss << std::setprecision(conf.precision) << std::showpoint << v;
		}
		Emit(oss.str(), PLAIN_SCALAR_STYLE);
	}

	void YAMLEmitterImpl::Emit(ConstBlobPtr b) {
		CheckClosed();
		CheckInDocument();
		yaml_event_t event;
		std::vector<char> buf(Base64EncodeSize(b->GetTotalLength()));
		unsigned len = Base64Encode(&buf[0], b->GetIOVec(), b->GetNumBuffers(), 0);
		if (yaml_scalar_event_initialize(&event, 0, (yaml_char_t*)"tag:yaml.org,2002:binary", (yaml_char_t*)&buf[0], len, 0, 0,
					(yaml_scalar_style_t)ANY_SCALAR_STYLE) == 0) {
			throw std::runtime_error("Unable to initialize scalar event.");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::Flush() {
		CheckClosed();
		if (yaml_emitter_flush(&emitter) == 0) {
			throw std::runtime_error(emitter.problem);
		}
	}

	void YAMLEmitterImpl::Close() {
		if (!closed) {
			CheckOutDocument();
			yaml_event_t event;
			if (yaml_stream_end_event_initialize(&event) == 0) {
				throw std::runtime_error("Unable to initialize stream end event.");
			}
			Emit(event);
			Flush();
			yaml_emitter_delete(&emitter);
			out.reset();
			closed = true;
		}
	}

	Variant YAMLEmitterImpl::GetParam(const std::string &key) {
		return conf.GetParam(key);
	}

	Variant YAMLEmitterImpl::GetParams() {
		return conf.GetParams();
	}

	void YAMLEmitterImpl::SetParam(const std::string &key, Variant value) {
		conf.SetParam(key, value);
	}

	void YAMLEmitterImpl::SetParams(Variant params) {
		conf.SetParams(params);
	}

	void YAMLEmitterImpl::EmitStreamStart() {
		yaml_event_t event;
		if (yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING) == 0) {
			throw std::runtime_error("Unable to initialize stream start event");
		}
		Emit(event);
	}

	void YAMLEmitterImpl::Emit(yaml_event_t &event) {
		if (yaml_emitter_emit(&emitter, &event) == 0) {
			yaml_event_delete(&event);
			throw std::runtime_error(emitter.problem);
		}
	}

}
