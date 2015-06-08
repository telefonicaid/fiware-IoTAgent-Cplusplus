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
#include <Variant/Emitter.h>
#include "JSONEmitter.h"
#ifdef ENABLE_YAML
#include "YAMLEmitter.h"
#endif
#ifdef ENABLE_XML
#include "XMLPLISTEmitter.h"
#endif
#ifdef ENABLE_MSGPACK
#include "MsgPackEmitter.h"
#endif
#include "BundleHdrEmitter.h"
#include <Variant/EmitterOutput.h>

namespace libvariant {
	EmitterImpl::~EmitterImpl() {}

	void EmitterImpl::SetParams(Variant params) {
		if (params.IsMap()) {
			for (Variant::MapIterator i(params.MapBegin()), e(params.MapEnd()); i != e; ++i) {
				SetParam(i->first, i->second);
			}
		}
	}

	Emitter::Emitter() {}

	Emitter::Emitter(shared_ptr<EmitterImpl> i)
		: impl(i)
	{}

	Emitter::~Emitter() {}

	Emitter &Emitter::BeginDocument() {
		impl->BeginDocument();
		return *this;
	}

	Emitter &Emitter::EndDocument() {
		impl->EndDocument();
		return *this;
	}

	Emitter &Emitter::BeginMap(int length) {
		impl->BeginMap(length);
		return *this;
	}

	Emitter &Emitter::EndMap() {
		impl->EndMap();
		return *this;
	}

	Emitter &Emitter::BeginList(int length) {
		impl->BeginList(length);
		return *this;
	}

	Emitter &Emitter::EndList() {
		impl->EndList();
		return *this;
	}

	Emitter &Emitter::EmitNull() {
		impl->EmitNull();
		return *this;
	}

	Emitter &Emitter::EmitTrue() {
		impl->EmitTrue();
		return *this;
	}

	Emitter &Emitter::EmitFalse() {
		impl->EmitFalse();
		return *this;
	}

	Emitter &Emitter::Emit(const std::string &v) {
		impl->Emit(v.c_str());
		return *this;
	}

	Emitter &Emitter::Emit(const char *v) {
		impl->Emit(v);
		return *this;
	}

	Emitter &Emitter::Emit(intmax_t v) {
		impl->Emit(v);
		return *this;
	}

	Emitter &Emitter::Emit(uintmax_t v) {
		impl->Emit(v);
		return *this;
	}

	Emitter &Emitter::Emit(double v) {
		impl->Emit(v);
		return *this;
	}

	Emitter &Emitter::Emit(ConstBlobPtr b) {
		impl->Emit(b);
		return *this;
	}

	void Emitter::Flush() {
		impl->Flush();
	}

	void Emitter::Close() {
		impl->Close();
	}

	Variant Emitter::GetParam(const std::string &key) {
		return impl->GetParam(key);
	}

	Variant Emitter::GetParams() {
		return impl->GetParams();
	}

	void Emitter::SetParam(const std::string &key, Variant value) {
		impl->SetParam(key, value);
	}

	void Emitter::SetParams(Variant params) {
		impl->SetParams(params);
	}

	shared_ptr<EmitterOutput> CreateEmitterOutput(void *ptr, unsigned len, unsigned *out_len) {
		return shared_ptr<EmitterOutput>(new EmitterMemoryOutput(ptr, len, out_len));
	}

	shared_ptr<EmitterOutput> CreateEmitterOutput(const char *filename) {
		return shared_ptr<EmitterOutput>(new EmitterFilenameOutput(filename));
	}

	shared_ptr<EmitterOutput> CreateEmitterOutput(FILE *f) {
		return shared_ptr<EmitterOutput>(new EmitterFileOutput(f, false));
	}

	shared_ptr<EmitterOutput> CreateEmitterOutput(std::streambuf *sb) {
		return shared_ptr<EmitterOutput>(new EmitterStreambufOutput(sb, false));
	}


	Emitter CreateEmitter(shared_ptr<EmitterOutput> o, SerializeType type, Variant params) {
		switch (type) {
		case SERIALIZE_JSON:
			return JSONEmitter(o, params);
		case SERIALIZE_XMLPLIST:
			return XMLPLISTEmitter(o, params);
		case SERIALIZE_BUNDLEHDR:
			return BundleHdrEmitter(o, params);
		case SERIALIZE_YAML:
			return YAMLEmitter(o, params);
		case SERIALIZE_MSGPACK:
			return MsgPackEmitter(o, params);
		default:
			throw std::runtime_error("libvariant: Unsupported emitter serialization type.");
		}
	}

	Emitter JSONEmitter(shared_ptr<EmitterOutput> o, Variant params) {
		return Emitter(shared_ptr<EmitterImpl>(new JSONEmitterImpl(o, params)));
	}

	Emitter YAMLEmitter(shared_ptr<EmitterOutput> o, Variant params) {
#ifdef ENABLE_YAML
		return Emitter(shared_ptr<EmitterImpl>(new YAMLEmitterImpl(o, params)));
#else
		throw std::runtime_error("libvariant not compiled with YAML support.");
#endif
	}

	Emitter XMLPLISTEmitter(shared_ptr<EmitterOutput> o, Variant params) {
#ifdef ENABLE_XML
		return Emitter(shared_ptr<EmitterImpl>(new XMLPLISTEmitterImpl(o, params)));
#else
		throw std::runtime_error("libvariant not compiled with plist xml support.");
#endif
	}

	Emitter BundleHdrEmitter(shared_ptr<EmitterOutput> o, Variant params) {
		return Emitter(shared_ptr<EmitterImpl>(new BundleHdrEmitterImpl(o, params)));
	}

	Emitter MsgPackEmitter(shared_ptr<EmitterOutput> o, Variant params) {
#ifdef ENABLE_MSGPACK
		return Emitter(shared_ptr<EmitterImpl>(new MsgPackEmitterImpl(o, params)));
#else
		throw std::runtime_error("libvariant not compiled with msgpack support.");
#endif
	}
}
