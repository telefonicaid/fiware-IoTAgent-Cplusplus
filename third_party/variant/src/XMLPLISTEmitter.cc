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

#include "XMLPLISTEmitter.h"
#include <Variant/EmitterOutput.h>
#include <sstream>
#include <iomanip>
#include <limits>
#include "XMLPLISTDefs.h"
#include "Base64.h"

namespace libvariant {

	int XMLPLISTEmitterImpl::DoWrite(void *ctx, const char *buffer, int len) {
		XMLPLISTEmitterImpl *impl = (XMLPLISTEmitterImpl*)ctx;
		return impl->output->Write(buffer, len);
	}

	int XMLPLISTEmitterImpl::DoClose(void *ctx) {
		XMLPLISTEmitterImpl *impl = (XMLPLISTEmitterImpl*)ctx;
		impl->output->Flush();
		return 0;
	}

	XMLPLISTEmitterImpl::XMLPLISTEmitterImpl(shared_ptr<EmitterOutput> out, Variant params)
		: output(out), pretty(false), indent(4),
			precision(std::numeric_limits<double>::digits10+2), closed(false)
	{
		if (params.IsMap()) {
			params.GetInto(pretty, "pretty", pretty);
			params.GetInto(indent, "indent", indent);
			params.GetInto(precision, "precision", precision);
		}
		xmlOutputBufferPtr xmloutput = xmlOutputBufferCreateIO(&XMLPLISTEmitterImpl::DoWrite,
				&XMLPLISTEmitterImpl::DoClose,
				(void*)this, 0);
		if (xmloutput == 0) {
			HandleError("xmlOutputBufferCreateIO");
		}
		writer = xmlNewTextWriter(xmloutput);
		if (writer == 0) {
			xmlOutputBufferClose(xmloutput);
			HandleError("xmlNewTextWriter");
		}
		UpdateParams();
		if (xmlTextWriterStartDocument(writer, 0, 0, "yes") < 0) {
			xmlFreeTextWriter(writer);
			writer = 0;
			HandleError("xmlTextWriterStartDocument");
		}
		state.push_back(NONE);
	}

	XMLPLISTEmitterImpl::~XMLPLISTEmitterImpl() {
		if (!closed) {
			xmlTextWriterEndDocument(writer);
			xmlTextWriterFlush(writer);
			closed = true;
		}
		xmlFreeTextWriter(writer);
		writer = 0;
	}

	void XMLPLISTEmitterImpl::BeginDocument() {
		StartElement(ROOT_NAME);
	}

	void XMLPLISTEmitterImpl::EndDocument() {
		CloseElement();
	}

	void XMLPLISTEmitterImpl::BeginMap(int) {
		StartElement(MAP_NAME);
		state.push_back(EXPECT_MAP_KEY);
	}

	void XMLPLISTEmitterImpl::EndMap() {
		CloseElement();
		state.pop_back();
	}

	void XMLPLISTEmitterImpl::BeginList(int) {
		StartElement(LIST_NAME);
		state.push_back(IN_LIST);
	}

	void XMLPLISTEmitterImpl::EndList() {
		CloseElement();
		state.pop_back();
	}

	void XMLPLISTEmitterImpl::EmitNull() {
		StartElement(NULL_NAME);
		CloseElement();
	}

	void XMLPLISTEmitterImpl::EmitTrue() {
		StartElement(TRUE_NAME);
		CloseElement();
	}

	void XMLPLISTEmitterImpl::EmitFalse() {
		StartElement(FALSE_NAME);
		CloseElement();
	}

	void XMLPLISTEmitterImpl::Emit(const char *v) {
		if (state.back() == EXPECT_MAP_KEY) {
			StartElement(KEY_NAME, false);
			WriteText(v);
			CloseElement();
		} else {
			StartElement(STRING_NAME);
			WriteText(v);
			CloseElement();
		}
	}

	void XMLPLISTEmitterImpl::Emit(intmax_t v) {
		StartElement(INTEGER_NAME);
		std::ostringstream oss;
		oss << v;
		WriteText(oss.str().c_str());
		CloseElement();
	}

	void XMLPLISTEmitterImpl::Emit(uintmax_t v) {
		StartElement(INTEGER_NAME);
		std::ostringstream oss;
		oss << v;
		WriteText(oss.str().c_str());
		CloseElement();
	}

	void XMLPLISTEmitterImpl::Emit(double v) {
		StartElement(FLOAT_NAME);
		std::ostringstream oss;
		oss << std::setprecision(precision) << std::showpoint << v;
		WriteText(oss.str().c_str());
		CloseElement();
	}

	void XMLPLISTEmitterImpl::Emit(ConstBlobPtr b) {
		StartElement(BLOB_NAME);
		std::vector<char> buf(Base64EncodeSize(b->GetTotalLength()));
		Base64Encode(&buf[0], b->GetIOVec(), b->GetNumBuffers());
		WriteText(&buf[0]);
		CloseElement();
	}

	void XMLPLISTEmitterImpl::Flush() {
		CheckClosed();
		if (xmlTextWriterFlush(writer) < 0) {
			HandleError("xmlTextWriterFlush");
		}
		output->Flush();
	}

	void XMLPLISTEmitterImpl::Close() {
		if (!closed) {
			if (xmlTextWriterEndDocument(writer) < 0) {
				HandleError("xmlTextWriterEndDocument");
			}
			Flush();
			closed = true;
		}
	}

	Variant XMLPLISTEmitterImpl::GetParam(const std::string &key) {
		if (key == "pretty") { return pretty; }
		if (key == "indent") { return indent; }
		if (key == "precision") { return precision; }
		return Variant::NullType;
	}

	Variant XMLPLISTEmitterImpl::GetParams() {
		Variant ret;
		ret["pretty"] = pretty;
		ret["indent"] = indent;
		ret["precision"] = precision;
		return ret;
	}

	void XMLPLISTEmitterImpl::SetParam(const std::string &key, Variant value) {
		if (key == "pretty") { pretty = value.AsBool(); }
		else if (key == "indent") { indent = value.AsUnsigned(); }
		else if (key == "precision") { precision = value.AsUnsigned(); }
		UpdateParams();
	}

	void XMLPLISTEmitterImpl::UpdateParams() {
		if (pretty) {
			xmlTextWriterSetIndent(writer, indent);
		} else {
			xmlTextWriterSetIndent(writer, 0);
		}
	}

}
