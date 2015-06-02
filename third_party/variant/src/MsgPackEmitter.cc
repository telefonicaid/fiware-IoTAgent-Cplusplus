/** \file
 * \author John Bridgman
 * \brief 
 */

#include "MsgPackEmitter.h"
#include "BlobMagic.h"

static void msgpack_pack_string(msgpack_packer *packer, const char *str, unsigned len) {
	msgpack_pack_raw(packer, len);
	msgpack_pack_raw_body(packer, str, len);
}

namespace libvariant {

	int MsgPackPackerWrite(void* data, const char* buf, unsigned int len) {
		return static_cast<EmitterOutput*>(data)->Write(buf, len);
	}

	MsgPackEmitterImpl::MsgPackEmitterImpl(shared_ptr<EmitterOutput> out, Variant params)
		: output(out)
	{
		msgpack_packer_init(&packer, output.get(), MsgPackPackerWrite);
	}

	MsgPackEmitterImpl::~MsgPackEmitterImpl() {
		Close();
	}


	void MsgPackEmitterImpl::BeginDocument() {
	}

	void MsgPackEmitterImpl::EndDocument() {
		Flush();
	}

	void MsgPackEmitterImpl::BeginMap(int length) {
		if (buffer) { buffer->BeginMap(length); }
		else if (length < 0) {
			buffer.reset(new EventBuffer);
			buffer->BeginMap(length);
		} else {
			msgpack_pack_map(&packer, length);
		}
	}

	void MsgPackEmitterImpl::EndMap() {
		if (buffer) {
			buffer->EndMap();
			if (buffer->GetLevel() <= 0) {
				shared_ptr<EventBuffer> buf = buffer;
				buffer.reset();
				buf->Playback(this);
			}
		}
	}

	void MsgPackEmitterImpl::BeginList(int length) {
		if (buffer) { buffer->BeginList(length); }
		else if (length < 0) {
			buffer.reset(new EventBuffer);
			buffer->BeginList(length);
		} else {
			msgpack_pack_array(&packer, length);
		}
	}

	void MsgPackEmitterImpl::EndList() {
		if (buffer) {
			buffer->EndList();
			if (buffer->GetLevel() <= 0) {
				shared_ptr<EventBuffer> buf = buffer;
				buffer.reset();
				buf->Playback(this);
			}
		}
	}

	void MsgPackEmitterImpl::EmitNull() {
		if (buffer) { buffer->EmitNull(); }
		else { msgpack_pack_nil(&packer); }
	}

	void MsgPackEmitterImpl::EmitTrue() {
		if (buffer) { buffer->EmitTrue(); }
		else { msgpack_pack_true(&packer); }
	}

	void MsgPackEmitterImpl::EmitFalse() {
		if (buffer) { buffer->EmitFalse(); }
		else { msgpack_pack_false(&packer); }
	}

	void MsgPackEmitterImpl::Emit(const char *v) {
		if (buffer) { buffer->Emit(v); }
		else { msgpack_pack_string(&packer, v, strlen(v)); }
	}

	void MsgPackEmitterImpl::Emit(intmax_t v) {
		if (buffer) { buffer->Emit(v); }
		else { msgpack_pack_int64(&packer, v); }
	}

	void MsgPackEmitterImpl::Emit(uintmax_t v) {
		if (buffer) { buffer->Emit(v); }
		else { msgpack_pack_uint64(&packer, v); }
	}

	void MsgPackEmitterImpl::Emit(double v) {
		if (buffer) { buffer->Emit(v); }
		else { msgpack_pack_double(&packer, v); }
	}

	void MsgPackEmitterImpl::Emit(ConstBlobPtr b) {
		if (buffer) { buffer->Emit(b); }
		else {
			msgpack_pack_raw(&packer, b->GetTotalLength() + MAGIC_BLOB_LENGTH);
			msgpack_pack_raw_body(&packer, MAGIC_BLOB_TAG, MAGIC_BLOB_LENGTH);
			for (int i = 0, num = b->GetNumBuffers(); i < num; ++i) {
				msgpack_pack_raw_body(&packer, b->GetPtr(i), b->GetLength(i));
			}
		}
	}

	void MsgPackEmitterImpl::Flush() {
		output->Flush();
	}

	void MsgPackEmitterImpl::Close() {
		output->Flush();
	}

}
