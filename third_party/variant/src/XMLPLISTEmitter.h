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
 * \brief  An emitter for the XMLPLIST subset that Variant supports.
 * Depends on libxml2.
 */
#ifndef VARIANT_XMLPLISTEMITTER_H
#define VARIANT_XMLPLISTEMITTER_H
#pragma once
#include <Variant/Emitter.h>
#include <libxml/xmlwriter.h>
#include <stdexcept>
#include <deque>

namespace libvariant {

	/**
	 * Supported params are:
	 *
	 * "pretty": true|false
	 * "indent": integer (amount to indent for pretty)
	 * "precision": number of digits for a float
	 */
	class XMLPLISTEmitterImpl : public EmitterImpl {
		enum State {
			NONE,
			EXPECT_MAP_KEY,
			EXPECT_MAP_VALUE,
			IN_LIST
		};
	public:

		XMLPLISTEmitterImpl(shared_ptr<EmitterOutput> out, Variant params);
		~XMLPLISTEmitterImpl();

		virtual void BeginDocument();
		virtual void EndDocument();
		virtual void BeginMap(int length);
		virtual void EndMap();
		virtual void BeginList(int length);
		virtual void EndList();

		virtual void EmitNull();
		virtual void EmitTrue();
		virtual void EmitFalse();
		virtual void Emit(const char *v);
		virtual void Emit(intmax_t v);
		virtual void Emit(uintmax_t v);
		virtual void Emit(double v);
		virtual void Emit(ConstBlobPtr b);

		virtual void Flush();
		virtual void Close();

		virtual Variant GetParam(const std::string &key);
		virtual Variant GetParams();
		virtual void SetParam(const std::string &key, Variant value);
	private:
		static int DoWrite(void *ctx, const char *buffer, int len);
		static int DoClose(void *ctx);
	
		void HandleError(const char *fnname) {
			xmlErrorPtr error = xmlGetLastError();
			if (error) {
				throw std::runtime_error(error->message);
			} else {
				throw std::runtime_error(fnname);
			}
		}

		void CheckClosed() {
			if (closed) { throw std::runtime_error("Cannot operate on a closed XMLPLISTEmitter."); }
		}

		void CheckMapKey(bool complex_type) {
			if (state.back() == EXPECT_MAP_VALUE) {
				state.back() = EXPECT_MAP_KEY;
			} else if (state.back() == EXPECT_MAP_KEY) {
				if (complex_type) {
					throw std::runtime_error("XMLPLISTEmitter does not support non trivial types as keys");
				}
				state.back() = EXPECT_MAP_VALUE;
			}
		}

		void StartElement(const char *name, bool complex_type=true) {
			CheckClosed();
			if (xmlTextWriterStartElement(writer, (const xmlChar*)name) < 0) {
				HandleError("xmlTextWriterStartElement");
			}
			CheckMapKey(complex_type);
		}

		void CloseElement() {
			CheckClosed();
			if (xmlTextWriterEndElement(writer) < 0) {
				HandleError("xmlTextWriterEndElement");
			}
		}

		void WriteText(const char *ptr) {
			if (xmlTextWriterWriteString(writer, (const xmlChar*)ptr) < 0) {
				HandleError("xmlTextWriterWriteString");
			}
		}

		void UpdateParams();
		xmlTextWriterPtr writer;
		shared_ptr<EmitterOutput> output;
		bool pretty;
		unsigned indent;
		unsigned precision;
		bool closed;
		std::deque<State> state;
	};
}
#endif
