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
#ifndef VARIANT_EMITTER_H
#define VARIANT_EMITTER_H
#pragma once
#include <stdint.h>
#include <vector>
#include <map>
#include <string>
#include <Variant/Variant.h>
#include <Variant/Blob.h>
#include <Variant/EmitterOutput.h>
#include <iosfwd>
#include <stdio.h>

namespace libvariant {

	/**
	 * Class to override to implement an Emitter
	 */
	class EmitterImpl {
	public:
		virtual ~EmitterImpl();

		virtual void BeginDocument() = 0;
		virtual void EndDocument() = 0;
		virtual void BeginMap(int length) = 0;
		virtual void EndMap() = 0;
		virtual void BeginList(int length) = 0;
		virtual void EndList() = 0;

		virtual void EmitNull() = 0;
		virtual void EmitTrue() = 0;
		virtual void EmitFalse() = 0;
		virtual void Emit(const char *v) = 0;
		virtual void Emit(intmax_t v) = 0;
		virtual void Emit(uintmax_t v) = 0;
		virtual void Emit(double v) = 0;
		virtual void Emit(ConstBlobPtr b) = 0;

		virtual void Flush() = 0;
		virtual void Close() = 0;

		virtual Variant GetParam(const std::string &key) = 0;
		virtual Variant GetParams() = 0;
		virtual void SetParam(const std::string &key, Variant value) = 0;
		virtual void SetParams(Variant params);
	};

	/**
	 * Actual emitter class.
	 */
	class Emitter {
	public:
		Emitter();
		Emitter(shared_ptr<EmitterImpl> i);
		~Emitter();

		Emitter &BeginDocument();
		Emitter &EndDocument();

		//Some emitter implementations require the length of the map at the
		//beginning. Such implementations should buffer events if length == -1
		//Implementations are free to ignore the length.
		Emitter &BeginMap() { return BeginMap(-1); }
		Emitter &BeginMap(int length);
		Emitter &EndMap();
		//Some emitter implementations require the length of the list at the
		//beginning. Such implementations should buffer events if length == -1
		Emitter &BeginList() { return BeginList(-1); }
		Emitter &BeginList(int length);
		Emitter &EndList();

		Emitter &EmitNull();
		Emitter &EmitTrue();
		Emitter &EmitFalse();
		Emitter &Emit(bool v) { return (v ? EmitTrue() : EmitFalse()); }
		Emitter &Emit(const std::string &v);
		Emitter &Emit(const char *v);
		Emitter &Emit(intmax_t v);
		Emitter &Emit(uintmax_t v);
		Emitter &Emit(double v);
		Emitter &Emit(ConstBlobPtr b);

		void Flush();
		void Close();

		/// Get the value of parameters of the parser (e.g. pretty printing,
		// styles, etc.)
		// See individual emitter below for supported values.
		Variant GetParam(const std::string &key);
		/// Get a dict of all parameters
		Variant GetParams();
		/// Set the value of parameters.
		// See individual emitter below for supported values.
		void SetParam(const std::string &key, Variant value);
		/// Set all parameters from a dict
		void SetParams(Variant params);
		/// Internal/advanced function to get the underlying implementation
		shared_ptr<EmitterImpl> GetImpl() { return impl; }
	private:
		shared_ptr<EmitterImpl> impl;
	};

	template<typename T>
	inline Emitter &operator<<(Emitter &e, const std::vector<T> &v) {
		e.BeginList(v.size());
		for (typename std::vector<T>::const_iterator i(v.begin()), end(v.end()); i!=end; i++) {
			e << *i;
		}
		e.EndList();
		return e;
	}

	template<typename T>
	inline Emitter &operator<<(Emitter &e, const std::map<std::string, T> &v) {
		e.BeginMap(v.size());
		for (typename std::map<std::string, T>::const_iterator i(v.begin()), end(v.end()); i!=end; ++i) {
			e.Emit(i->first);
			e << i->second;
		}
		e.EndMap();
		return e;
	}
	inline Emitter &operator<<(Emitter &e, bool v) { e.Emit(v); return e; }
	inline Emitter &operator<<(Emitter &e, char v) { e.Emit(intmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, short v) { e.Emit(intmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, int v) { e.Emit(intmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, long v) { e.Emit(intmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, long long v) { e.Emit(intmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, unsigned char v) { e.Emit(uintmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, unsigned short v) { e.Emit(uintmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, unsigned int v) { e.Emit(uintmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, unsigned long v) { e.Emit(uintmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, unsigned long long v) { e.Emit(uintmax_t(v)); return e; }
	inline Emitter &operator<<(Emitter &e, const char *v) { e.Emit(v); return e; }
	inline Emitter &operator<<(Emitter &e, const std::string &v) { e.Emit(v); return e; }
	inline Emitter &operator<<(Emitter &e, float v) { e.Emit(double(v)); return e; }
	inline Emitter &operator<<(Emitter &e, double v) { e.Emit(v); return e; }
	inline Emitter &operator<<(Emitter &e, ConstBlobPtr v) { e.Emit(v); return e; }
	
	shared_ptr<EmitterOutput> CreateEmitterOutput(void *ptr, unsigned len, unsigned *out_len=0);
	shared_ptr<EmitterOutput> CreateEmitterOutput(const char *filename);
	shared_ptr<EmitterOutput> CreateEmitterOutput(FILE *f);
	shared_ptr<EmitterOutput> CreateEmitterOutput(std::streambuf *sb);

	/**
	 * Create an Emiter that Emits the format specified by type.
	 * Supported parameters are dependent on type (see below).
	 */
	Emitter CreateEmitter(shared_ptr<EmitterOutput> o, SerializeType type, Variant params = Variant::NullType);

	/**
	 * Create an Emitter that Emits the JSON format.
	 *
	 * Supported parameters are:
	 *  "pretty": boolean
	 *  "indent": integer (number of spaces to indent when pretty is on,
	 *  default 4)
	 *  "precision": integer (number of digits for floats)
	 *  "strict": be more strict about emitting correct json
	 */
	Emitter JSONEmitter(shared_ptr<EmitterOutput> o, Variant params = Variant::NullType);

	/**
	 * Supported parameters are (first is default):
	 *
	 * "scalar_style": "any"|"plain"|"single quoted"|"double
	 * quoted"|"literal"|"folded"
	 *
	 * "map_style": "any"|"block"|"flow"
	 *
	 * "list_style": "any"|"block"|"flow"
	 *
	 * "implicit_document_begin": false|true
	 *
	 * "implicit_document_end": false|true
	 *
	 * "indent": integer (number of spaces to indent)
	 *
	 * "width": integer (set prefered line width)
	 *
	 * "precision": integer (number of digits for floats)
	 */
	Emitter YAMLEmitter(shared_ptr<EmitterOutput> o, Variant params = Variant::NullType);

	/**
	 * Supported params are:
	 *
	 * "pretty": true|false
	 * "indent": integer (amount to indent for pretty)
	 * "precision": integer (number of digits for floats)
	 */
	Emitter XMLPLISTEmitter(shared_ptr<EmitterOutput> o, Variant params = Variant::NullType);
	/** 
	 * Supported params are:
	 *
	 * "precision": integer (number of digits for floats)
	 */
	Emitter BundleHdrEmitter(shared_ptr<EmitterOutput> o, Variant params = Variant::NullType);

	/**
	 * No parameters supported.
	 */
	Emitter MsgPackEmitter(shared_ptr<EmitterOutput> o, Variant params = Variant::NullType);
}
#endif
