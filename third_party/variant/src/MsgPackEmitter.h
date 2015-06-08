/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_MSGPACK_H
#define VARIANT_MSGPACK_H
#pragma once
#include <Variant/Variant.h>
#include <Variant/EmitterOutput.h>
#include <Variant/ParserInput.h>
#include <Variant/Parser.h>
#include <Variant/Emitter.h>
#include "EventBuffer.h"
#include <msgpack.h>
namespace libvariant {

	class MsgPackEmitterImpl : public EmitterImpl {
	public:
		MsgPackEmitterImpl(shared_ptr<EmitterOutput> out, Variant params);
		~MsgPackEmitterImpl();
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

		virtual Variant GetParam(const std::string &key) { return Variant::NullType; }
		virtual Variant GetParams() { return Variant::NullType; }
		virtual void SetParam(const std::string &key, Variant value) {}
	private:
		shared_ptr<EmitterOutput> output;
		shared_ptr<EventBuffer> buffer;
		msgpack_packer packer;
	};

}
#endif
