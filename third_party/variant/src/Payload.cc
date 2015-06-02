/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Payload.h>
#include <Variant/Path.h>
#include <sstream>
#include <stdexcept>
#include <string.h>

#define PADDING_MOD 32

namespace libvariant {

	struct PayloadEmitState {
		Emitter e; 
		Path::iterator len_end;
		Path::iterator data_end;
	   	uintmax_t payload_len;
	};

	void PayloadEmit(
			const Variant &v,
		   	Path::iterator len_i,
			bool on_len_path,
		   	Path::iterator data_i,
			bool on_data_path,
			PayloadEmitState &state)
	{
		if (on_len_path && len_i == state.len_end) {
		   	state.e.Emit(state.payload_len);
			return;
	   	}
		if (on_data_path && data_i == state.data_end) {
			state.e.EmitNull();
			return;
		}

		bool output_len = false;
		unsigned index = 0;
		unsigned length = 0;

		switch (v.GetType()) {
		case Variant::NullType:
			state.e.EmitNull();
			break;
		case Variant::BoolType:
			if (v.AsBool()) {
				state.e.EmitTrue();
			} else {
				state.e.EmitFalse();
			}
			break;
		case Variant::IntegerType:
			state.e.Emit(v.AsInt());
			break;
		case Variant::UnsignedType:
			state.e.Emit(v.AsUnsigned());
			break;
		case Variant::FloatType:
			state.e.Emit(v.AsDouble());
			break;
		case Variant::StringType:
			state.e.Emit(v.AsString());
			break;
		case Variant::ListType:
			length = v.Size();
			if (on_len_path && length <= len_i->AsUnsigned()) { length = len_i->AsUnsigned() + 1; }
			state.e.BeginList(length);
			for (Variant::ConstListIterator i(v.ListBegin()), e(v.ListEnd()); i != e; ++i, ++index) {
				bool on_len = on_len_path && index == len_i->AsUnsigned();
				bool on_data = on_data_path && index == data_i->AsUnsigned();
				PayloadEmit(*i,
					   	(on_len ? len_i + 1 : len_i), on_len,
					   	(on_data ? data_i + 1 : data_i), on_data,
					   	state);
			}
			if (on_len_path && len_i->AsUnsigned() < index) {
				while (len_i->AsUnsigned() < index) {
					state.e.EmitNull();
					++index;
				}
				if (len_i+1 != state.len_end) {
					Variant tmp;
					Path newpath(len_i+1, state.len_end);
					tmp.SetPath(newpath, state.payload_len);
					state.e << tmp;
				} else {
					state.e.Emit(state.payload_len);
				}
			}
			state.e.EndList();
			break;
		case Variant::MapType:
			length = v.Size();
			if (on_len_path && !v.Contains(len_i->AsString())) { length++; }
			if (on_data_path && v.Contains(data_i->AsString())) { length--; }
			state.e.BeginMap(length);
			for (Variant::ConstMapIterator i(v.MapBegin()), e(v.MapEnd()); i != e; ++i) {
				bool on_len = on_len_path && i->first == len_i->AsString();
				if (on_len) { output_len = true; }
				bool on_data = on_data_path && i->first == data_i->AsString();
				if ( !(on_data && data_i + 1 == state.data_end) ) {
					state.e.Emit(i->first);
					PayloadEmit(i->second,
							(on_len ? len_i + 1 : len_i), on_len,
							(on_data ? data_i + 1 : data_i), on_data,
							state);
				}
			}
			if (on_len_path && !output_len) {
				state.e.Emit(len_i->AsString());
				if (len_i+1 != state.len_end) {
					Variant tmp;
					Path newpath(len_i+1, state.len_end);
					tmp.SetPath(newpath, state.payload_len);
					state.e << tmp;
				} else {
					state.e.Emit(state.payload_len);
				}
			}
			state.e.EndMap();
			break;
		case Variant::BlobType:
			state.e << v.AsBlob();
			break;
		default:
			throw UnknownTypeError(v.GetType());
		}
	}

	void SerializeWithPayload(shared_ptr<EmitterOutput> out, Variant v, SerializeType type, Variant params)
	{
		if (!v.IsMap()) {
			throw std::runtime_error("libvariant: Serializing with payload requires root element to be a map.");
		}

		Path dpath;
		Path lpath;
		if (type == SERIALIZE_BUNDLEHDR) {
			ParsePath(dpath, params.Get("data_path", VARIANT_BUNDLE_PAYLOAD_DATA_PATH).AsString());
			ParsePath(lpath, params.Get("length_path", VARIANT_BUNDLE_PAYLOAD_LENGTH_PATH).AsString());
		} else {
			ParsePath(dpath, params.Get("data_path", VARIANT_PAYLOAD_DATA_PATH).AsString());
			ParsePath(lpath, params.Get("length_path", VARIANT_PAYLOAD_LENGTH_PATH).AsString());
		}
		bool ignore_payload = params.Get("ignore_payload", false).AsBool();
		uintmax_t payload_length = 0;

		bool payload_exists = v.HasPath(dpath);
		BlobPtr blob;
		if (payload_exists) {
			blob = v.GetPath(dpath).AsBlob();
			payload_length = blob->GetTotalLength();
		} else if (ignore_payload) {
			payload_length = v.GetPath(lpath, 0).AsUnsigned();
		}
		payload_length = params.Get("payload_length", payload_length).AsUnsigned();


		Emitter emitter = CreateEmitter(out, type, params);
		PayloadEmitState state;
		state.e = emitter;
		state.len_end = lpath.end();
		state.data_end = dpath.end();
		state.payload_len = payload_length;

		emitter.BeginDocument();
		PayloadEmit(v, lpath.begin(), true, dpath.begin(), true, state);
		emitter.EndDocument();
		emitter.Flush();

		// plus 1 because we want a payload to be aligned when adding 1 null byte
		unsigned num = PADDING_MOD - (out->NumBytesWritten() + 1) % PADDING_MOD;
		std::vector<char> buf(num, '\n');
		buf.push_back('\0');
		unsigned written = 0;
		while (written < num) {
			written += out->Write(&buf[written], buf.size() - written);
		}

		if (payload_exists && !ignore_payload) {
			unsigned num_slabs = blob->GetNumBuffers();
			for (unsigned slab = 0; slab < num_slabs; ++slab) {
				unsigned to_write = blob->GetLength(slab);
				char *ptr = (char*)blob->GetPtr(slab);
				while (to_write > 0) {
					unsigned num = out->Write(ptr, to_write);
					ptr += num;
					to_write -= num;
				}
			}
			out->Flush();
		}
	}

	class ProxyInput : public ParserInput {
	public:
		ProxyInput(shared_ptr<ParserInput> i);
		virtual ~ProxyInput();
		virtual const void *GetPtr(unsigned &len);
		virtual void Release(unsigned len);
	private:
		shared_ptr<ParserInput> base_input;
		unsigned scanned_to;
	};

	ProxyInput::ProxyInput(shared_ptr<ParserInput> i)
		: base_input(i), scanned_to(0)
	{}

	ProxyInput::~ProxyInput() {}

	const void *ProxyInput::GetPtr(unsigned &len) {
		const void *retptr = base_input->GetPtr(len);
		if (retptr) {
			const char *ptr = (const char*)retptr;
			if (ptr[scanned_to] != '\0') {
				scanned_to += strnlen(&ptr[scanned_to], len-scanned_to);

			}
			len = std::min(len, scanned_to);
		}
		return retptr;
	}

	void ProxyInput::Release(unsigned len) {
		scanned_to -= len;
		base_input->Release(len);
	}

	Variant DeserializeWithPayload(shared_ptr<ParserInput> in, SerializeType type, Variant params, bool be_safe)
	{
		Path dpath;
		Path lpath;
		if (type == SERIALIZE_BUNDLEHDR) {
			ParsePath(dpath, params.Get("data_path", VARIANT_BUNDLE_PAYLOAD_DATA_PATH).AsString());
			ParsePath(lpath, params.Get("length_path", VARIANT_BUNDLE_PAYLOAD_LENGTH_PATH).AsString());
		} else {
			ParsePath(dpath, params.Get("data_path", VARIANT_PAYLOAD_DATA_PATH).AsString());
			ParsePath(lpath, params.Get("length_path", VARIANT_PAYLOAD_LENGTH_PATH).AsString());
		}

		shared_ptr<ParserInput> input(new ProxyInput(in));
		Parser parser = CreateParser(input, type);
		Variant ret = ParseVariant(parser);

		// Read any remaining bytes up to and including a terminating null if present.
		bool loop = true;
		while (loop) {
			unsigned len = 1;
			const char *ptr = (const char *)in->GetPtr(len);
			if (len > 0) {
				unsigned i;
				for (i = 0; i < len && loop; ++i) {
					if (ptr[i] == '\0') {
						loop = false;
					}
				}
				in->Release(i);
			} else { loop = false; }
		}

		uintmax_t payload_length = ret.GetPath(lpath, 0u).AsUnsigned();

		if (payload_length > 0) {
			// Note: make a copy when be_safe is true, otherwise just
			// reference the buffer that the input returns. This is only
			// a valid thing to do when the input is a memory buffer input
			unsigned len = payload_length;
			const void *ptr = in->GetPtr(len);
			if (len < payload_length) {
				throw std::runtime_error("libvariant: Error parsing payload, "
						"unexpected EOF encountered when reading payload");
			}
			BlobPtr payload;
			if (be_safe) {
				payload = Blob::CreateCopy(ptr, payload_length);
			} else {
				payload = Blob::CreateReferenced((void*)ptr, payload_length);
			}
			in->Release(payload_length);
			ret.SetPath(dpath, payload);
		}
		return ret;
	}

	std::string SerializeWithPayload(Variant v, SerializeType type, Variant params) {
		std::ostringstream oss;
		SerializeWithPayload(oss.rdbuf(), v, type, params);
		return oss.str();
	}

	void SerializeWithPayload(const std::string &filename, Variant v, SerializeType type,
		   	Variant params) {
		shared_ptr<EmitterOutput> o = CreateEmitterOutput(filename.c_str());
		SerializeWithPayload(o, v, type, params);
	}

	void SerializeWithPayload(FILE *f, Variant v, SerializeType type,
		   	Variant params) {
		shared_ptr<EmitterOutput> o = CreateEmitterOutput(f);
		SerializeWithPayload(o, v, type, params);
	}

	void SerializeWithPayload(std::streambuf *sb, Variant v, SerializeType type,
		   	Variant params) {
		shared_ptr<EmitterOutput> o = CreateEmitterOutput(sb);
		SerializeWithPayload(o, v, type, params);
	}

	unsigned SerializeWithPayload(void *ptr, unsigned len, Variant v, SerializeType type,
		   	Variant params) {
		unsigned out_len = 0;
		shared_ptr<EmitterOutput> o = CreateEmitterOutput(ptr, len, &out_len);
		SerializeWithPayload(o, v, type, params);
		return out_len;
	}

	Variant DeserializeWithPayload(const std::string &str, SerializeType type, Variant params) {
		return DeserializeWithPayload(str.c_str(), str.length(), type, params);
	}

	Variant DeserializeWithPayload(const void *ptr, unsigned len, SerializeType type,
		   	Variant params, bool be_safe)
   	{
		shared_ptr<ParserInput> i = CreateParserInput(ptr, len);
		return DeserializeWithPayload(i, type, params, be_safe);
	}

	Variant DeserializeWithPayloadFile(const char *filename, SerializeType type, Variant params) {
		shared_ptr<ParserInput> i = CreateParserInputFile(filename);
		return DeserializeWithPayload(i, type, params);
	}

	Variant DeserializeWithPayloadFile(FILE *f, SerializeType type, Variant params) {
		shared_ptr<ParserInput> i = CreateParserInputFile(f);
		return DeserializeWithPayload(i, type, params);
	}

	Variant DeserializeWithPayloadFile(std::streambuf *sb, SerializeType type, Variant params) {
		shared_ptr<ParserInput> i = CreateParserInputFile(sb);
		return DeserializeWithPayload(i, type, params);
	}

}
