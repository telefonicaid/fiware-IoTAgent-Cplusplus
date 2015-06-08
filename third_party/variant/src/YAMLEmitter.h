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

#ifndef  YAMLEMITTER_INC
#define  YAMLEMITTER_INC
#pragma once
#include <stdint.h>
#include <sstream>
#include <string>
#include <string.h>
#include <memory>
#include <Variant/Emitter.h>
#include <yaml.h>
#include <stdexcept>

namespace libvariant {

	/**
	 * Supported parameters are (first is default):
	 * "scalar_style": "any"|"plain"|"single quoted"|"double
	 * quoted"|"literal"|"folded"
	 * "map_style": "any"|"block"|"flow"
	 * "list_style": "any"|"block"|"flow"
	 * "implicit_document_begin": false|true
	 * "implicit_document_end": false|true
	 * These two parameters are only settable on creation.
	 * Resetting afterwards has no effect.
	 * "indent": integer (number of spaces to indent)
	 * "width": integer (set prefered line width)
	 * "precision": number of digits for a float
	 */
	class YAMLEmitterImpl : public EmitterImpl {
	public:
		enum ConfigKeys {
			SCALAR_STYLE,
			MAP_STYLE,
			LIST_STYLE,
			IMPLICIT_DOCUMENT_BEGIN,
			IMPLICIT_DOCUMENT_END,
			INDENT,
			WIDTH,
			PRECISION
		};

		enum ScalarStyle {
			ANY_SCALAR_STYLE,
			PLAIN_SCALAR_STYLE,
			SINGLE_QUOTED_SCALAR_STYLE,
			DOUBLE_QUOTED_SCALAR_STYLE,
			LITERAL_SCALAR_STYLE,
			FOLDED_SCALAR_STYLE
		};

		enum Style {
			ANY_STYLE,
			BLOCK_STYLE,
			FLOW_STYLE
		};

		struct Config {
			Config(Variant params);
			void SetParams(Variant params);
			void SetParam(const std::string &key, Variant value);
			void SetParam(ConfigKeys index, Variant value);
			Variant GetParams();
			Variant GetParam(const std::string &key);
			Variant GetParam(ConfigKeys index);

			ScalarStyle default_scalar_style;
			Style default_map_style;
			Style default_list_style;
			bool implicit_begin_document;
			bool implicit_end_document;
			int indent;
			int width;
			unsigned precision;
		};

		/**
		 * Create a YAMLEmitter that uses output to write.
		 * The output object will be deleted when YAMLEmitter
		 * is destroyed.
		 */
		YAMLEmitterImpl(shared_ptr<EmitterOutput> output, Variant params);
		virtual ~YAMLEmitterImpl();

		virtual void BeginDocument();
		void BeginDocument(bool implicit);
		virtual void EndDocument();
		void EndDocument(bool implicit);
		virtual void BeginMap(int length);
		void BeginMap(int length, Style style);
		virtual void EndMap();
		virtual void BeginList(int length);
		void BeginList(int length, Style style);
		virtual void EndList();

		virtual void EmitNull();
		virtual void EmitTrue();
		virtual void EmitFalse();
		void Emit(const std::string &v, ScalarStyle style);
		virtual void Emit(const char *v);
		void Emit(const char *v, ScalarStyle style);
		virtual void Emit(intmax_t v);
		virtual void Emit(uintmax_t v);
		virtual void Emit(double v);
		virtual void Emit(ConstBlobPtr b);

		virtual void Flush();
		/** Flush, release all resources and clean up.
		 * Doing anything other than deleting this object
		 * will result in a std::runtime_error */
		virtual void Close();

		virtual Variant GetParam(const std::string &key);
		virtual Variant GetParams();
		virtual void SetParam(const std::string &key, Variant value);
		virtual void SetParams(Variant params);

	private:
		void EmitStreamStart();
		void Emit(yaml_event_t &event);

		void CheckClosed() {
			if (closed) {
				throw std::runtime_error("YAMLEmitter is closed");
			}
		}

		void CheckInDocument() {
			if (!in_document) {
				BeginDocument();
			}
		}

		void CheckOutDocument() {
			if (in_document) {
				EndDocument();
			}
		}

		shared_ptr<EmitterOutput> out;
		yaml_emitter_t emitter;
		bool closed;
		bool in_document;
		Config conf;
	};

}
#endif
