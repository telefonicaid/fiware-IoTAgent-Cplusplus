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

#include "JSONEmitter.h"
#include <Variant/EmitterOutput.h>
#include <stdio.h>
#include <string.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <cmath>
#include "Base64.h"
#include "BlobMagic.h"

using namespace std;

// We do some small buffering optimizations so that we aren't calling
// Write over and over again with very small amounts.

#define MAX_BUFFER_LEN 512u
#define BUFFER_THRESHOLD 128u

namespace libvariant {

	JSONEmitterImpl::JSONEmitterImpl(shared_ptr<EmitterOutput> o, Variant params)
		: output(o), pretty(false), strict(false), num_indent(4),
	   	numeric_precision(std::numeric_limits<double>::digits10+2),
	   	buffer(MAX_BUFFER_LEN, 0), buffer_len(0)
	{
		if (params.IsMap()) {
			params.GetInto(pretty, "pretty", pretty);
			params.GetInto(strict, "strict", strict);
			params.GetInto(num_indent, "indent", num_indent);
			params.GetInto(numeric_precision, "precision", numeric_precision);
		}
	}

	JSONEmitterImpl::~JSONEmitterImpl() {
		Close();
	}

	void JSONEmitterImpl::BeginDocument() {
	}

	void JSONEmitterImpl::EndDocument() {
	}

	void JSONEmitterImpl::BeginMap(int length) {
		CheckSeparator();
		EmitRaw("{");
		state.push_back(MAP_FIRST);
	}

	void JSONEmitterImpl::EndMap() {
		state.pop_back();
		EmitIndent();
		EmitRaw("}");
		Flush();
	}

	void JSONEmitterImpl::BeginList(int length) {
		CheckSeparator();
		EmitRaw("[");
		state.push_back(LIST_FIRST);
	}

	void JSONEmitterImpl::EndList() {
		state.pop_back();
		EmitIndent();
		EmitRaw("]");
		Flush();
	}

	void JSONEmitterImpl::EmitNull() {
		CheckSeparator();
		EmitRaw("null");
	}

	void JSONEmitterImpl::EmitTrue() {
		CheckSeparator();
		EmitRaw("true");
	}

	void JSONEmitterImpl::EmitFalse() {
		CheckSeparator();
		EmitRaw("false");
	}

	void JSONEmitterImpl::Emit(const char *v) {
		CheckSeparator();
		EmitString(v, strlen(v));
	}

	void JSONEmitterImpl::Emit(intmax_t v) {
		CheckSeparator();
		std::ostringstream oss;
		oss << v;
		EmitRaw(oss.str().c_str(), oss.str().length());
	}

	void JSONEmitterImpl::Emit(uintmax_t v) {
		CheckSeparator();
		std::ostringstream oss;
		oss << v;
		EmitRaw(oss.str().c_str(), oss.str().length());
	}

	void JSONEmitterImpl::Emit(double v) {
		CheckSeparator();
		std::ostringstream oss;
		if (strict && (std::isnan(v) || std::isinf(v))) {
			throw std::runtime_error("JSONEmitter: The JSON specification does not permit"
					" numeric types that cannot be represented as a series of digits.");
		} else if (std::isnan(v) || std::isinf(v)) {
			oss << "null";
		} else {
			oss << std::setprecision(numeric_precision) << std::showpoint << v;
		}
		EmitRaw(oss.str().c_str(), oss.str().length());
	}

	void JSONEmitterImpl::Emit(ConstBlobPtr b) {
		if (strict) {
			throw std::runtime_error("JSONEmitter: Cannot serialize a BlobType"
					" in strict JSON mode.");
		}
		CheckSeparator();
		EmitRaw("\"");
		EmitRaw(MAGIC_BLOB_JSONSTR);
		std::vector<char> buf(Base64EncodeSize(b->GetTotalLength(), 0));
		unsigned len = Base64Encode(&buf[0], b->GetIOVec(), b->GetNumBuffers(), 0);
		EmitRaw(&buf[0], len);
		EmitRaw("\"");
	}

	void JSONEmitterImpl::Flush() {
		if (buffer_len > 0) {
			unsigned num_written = 0;
			while (num_written < buffer_len) {
				num_written += output->Write(&buffer[num_written], buffer_len - num_written);
			}
			buffer_len = 0;
		}
		output->Flush();
	}

	void JSONEmitterImpl::Close() {
		Flush();
	}

	Variant JSONEmitterImpl::GetParam(const std::string &key) {
		if (key == "pretty") { return pretty; }
		if (key == "indent") { return num_indent; }
		if (key == "precision") { return numeric_precision; }
		return Variant::NullType;
	}

	Variant JSONEmitterImpl::GetParams() {
		Variant ret;
		ret["pretty"] = pretty;
		ret["indent"] = num_indent;
		ret["precision"] = numeric_precision;
		return ret;
	}

	void JSONEmitterImpl::SetParam(const std::string &key, Variant value) {
		if (key == "pretty") {
			pretty = value.AsBool();
		}
		if (key == "indent") {
			num_indent = value.AsUnsigned();
		}
		if (key == "precision") {
			numeric_precision = value.AsUnsigned();
		}
	}

	void JSONEmitterImpl::CheckSeparator() {
		if (state.empty()) return;
		switch (state.back()) {
		case MAP_FIRST:
			state.back() = IN_MAP_VALUE;
			EmitIndent();
			break;
		case IN_MAP_KEY:
			state.back() = IN_MAP_VALUE;
			EmitRaw(",");
			EmitIndent();
			break;
		case IN_MAP_VALUE:
			state.back() = IN_MAP_KEY;
			EmitRaw(": ");
			break;
		case LIST_FIRST:
			state.back() = IN_LIST;
			EmitIndent();
			break;
		case IN_LIST:
			EmitRaw(",");
			EmitIndent();
			break;
		default:
			break;
		}
	}

	void JSONEmitterImpl::EmitIndent() {
		if (!pretty) return;
		EmitRaw("\n");
		unsigned indent = state.size() * num_indent;
		if (indent > BUFFER_THRESHOLD) {
			Flush();
			memset(&buffer[buffer_len], ' ', BUFFER_THRESHOLD);
			unsigned num_written = 0;
			while (num_written < indent) {
				num_written += output->Write(&buffer[buffer_len], min(indent - num_written, BUFFER_THRESHOLD));
			}
		} else {
			if (indent + buffer_len > MAX_BUFFER_LEN) {
				Flush();
			}
			memset(&buffer[buffer_len], ' ', indent);
			buffer_len += indent;
		}
	}

	void JSONEmitterImpl::EmitString(const char *text, unsigned len) {
		EmitRaw("\"");
		const char *end = text + len;
		while (text != end) {
			if (*text == '\\') {
				EmitRaw("\\\\");
			} else if (*text == '\"') {
				EmitRaw("\\\"");
			//} else if (*text == '/') {
				//EmitRaw("\\/");
			} else if (*text == '\b') {
				EmitRaw("\\b");
			} else if (*text == '\f') {
				EmitRaw("\\f");
			} else if (*text == '\n') {
				EmitRaw("\\n");
			} else if (*text == '\r') {
				EmitRaw("\\r");
			} else if (*text == '\t') {
				EmitRaw("\\t");
			} else if (*text < 0)   {
				EmitRaw(text, 1);
			} else if (*text < 0x20) {
				char buffer[7];
				int val = (unsigned char)*text;
				snprintf(buffer, sizeof(buffer), "\\u%4.4x", val);
				EmitRaw(&buffer[0]);
			} else {
				EmitRaw(text, 1);
			}
			++text;
		}
		EmitRaw("\"");
	}

	void JSONEmitterImpl::EmitRaw(const char *text) {
		EmitRaw(text, strlen(text));
	}
	void JSONEmitterImpl::EmitRaw(const char *text, unsigned len) {
		if (len > BUFFER_THRESHOLD) {
			Flush();
			unsigned num_written = 0;
			while (num_written < len) {
				num_written += output->Write(&text[num_written], len - num_written);
			}
		} else {
			if (len + buffer_len > MAX_BUFFER_LEN) {
				Flush();
			}
			memcpy(&buffer[buffer_len], text, len);
			buffer_len += len;
		}
	}

}
