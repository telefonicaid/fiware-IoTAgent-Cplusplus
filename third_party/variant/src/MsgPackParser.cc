/** \file
 * \author John Bridgman
 * \brief 
 */

#include "MsgPackParser.h"
#include "BlobMagic.h"
#include <msgpack/unpack_define.h>
#include <sstream>

// vmpu = variant msg pack unpack

struct vmpu_user {
	libvariant::MsgPackParserImpl *impl;
};

struct vmpu_object {
	vmpu_object() : count(0) {}
	uint64_t count;
};

#define msgpack_unpack_struct(name) \
struct vmpu ## name

#define msgpack_unpack_func(ret, name) \
ret vmpu ## name

#define msgpack_unpack_callback(name) \
vmpu_callback ## name

#define msgpack_unpack_object vmpu_object

#define msgpack_unpack_user vmpu_user


typedef struct vmpu_context vmpu_context;

static void vmpu_init(vmpu_context* ctx);

//static vmpu_object vmpu_data(vmpu_context* ctx);

static int vmpu_execute(vmpu_context* ctx,
		const char* data, size_t len, size_t* off);


static inline vmpu_object vmpu_callback_root(vmpu_user* u)
{ return vmpu_object(); }

static inline int vmpu_callback_uint8(vmpu_user* u, uint8_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, uint64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_uint16(vmpu_user* u, uint16_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, uint64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_uint32(vmpu_user* u, uint32_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, uint64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_uint64(vmpu_user* u, uint64_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, uint64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_int8(vmpu_user* u, int8_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, int64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_int16(vmpu_user* u, int16_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, int64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_int32(vmpu_user* u, int32_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, int64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_int64(vmpu_user* u, int64_t d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, int64_t(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_float(vmpu_user* u, float d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, double(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_double(vmpu_user* u, double d, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, double(d), 0, 0);
	return 0;
}

static inline int vmpu_callback_nil(vmpu_user* u, vmpu_object* o)
{
	u->impl->TopAction()->Null(u->impl, 0, 0);
	return 0;
}

static inline int vmpu_callback_true(vmpu_user* u, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, true, 0, 0);
	return 0;
}

static inline int vmpu_callback_false(vmpu_user* u, vmpu_object* o)
{
	u->impl->TopAction()->Scalar(u->impl, false, 0, 0);
	return 0;
}

static inline int vmpu_callback_array(vmpu_user* u, unsigned int n, vmpu_object* o)
{
	u->impl->TopAction()->BeginList(u->impl, n, 0, 0);
	o->count = n;
	if (n == 0) {
		u->impl->TopAction()->EndList(u->impl);
	}
	return 0;
}

static inline int vmpu_callback_array_item(vmpu_user* u, vmpu_object* c, vmpu_object o)
{
	c->count--;
	if (c->count == 0) {
		u->impl->TopAction()->EndList(u->impl);
	}
	return 0;
}

static inline int vmpu_callback_map(vmpu_user* u, unsigned int n, vmpu_object* o)
{
	u->impl->TopAction()->BeginMap(u->impl, n, 0, 0);
	o->count = n;
	if (n == 0) {
		u->impl->TopAction()->EndMap(u->impl);
	}
	return 0;
}

static inline int vmpu_callback_map_item(vmpu_user* u, vmpu_object* c,
		vmpu_object k, vmpu_object v)
{
	c->count--;
	if (c->count == 0) {
		u->impl->TopAction()->EndMap(u->impl);
	}
	return 0;
}

static inline int vmpu_callback_raw(vmpu_user* u, const char* b, const char* p,
		unsigned int l, vmpu_object* o)
{
	if (l >= MAGIC_BLOB_LENGTH && memcmp(MAGIC_BLOB_TAG, p, MAGIC_BLOB_LENGTH) == 0) {
		u->impl->TopAction()->Scalar(u->impl,
			   	libvariant::Blob::CreateCopy(p + MAGIC_BLOB_LENGTH, l - MAGIC_BLOB_LENGTH), 0, 0);
	} else {
		u->impl->TopAction()->Scalar(u->impl, p, l, 0, 0);
	}
	return 0;
}

#include <msgpack/unpack_template.h>

namespace libvariant {

	MsgPackParserImpl::MsgPackParserImpl(shared_ptr<ParserInput> in)
		: state(START), input(in),
		ctx(new vmpu_context),
		bytecount(0)
	{
		vmpu_init(ctx.get());
		ctx->user.impl = this;
	}

	MsgPackParserImpl::~MsgPackParserImpl()
	{}

	int MsgPackParserImpl::Run() {
		while (!action_stack.empty()) {
			unsigned len = 0;
			const char *ptr =  0; 
			size_t off = 0;
			int ret = 0;
			switch (state) {
			case START:
				TopAction()->BeginDocument(this);
				state = OK;
				break;
			case OK:
				while (state == OK) {
					if (off > 0) { len = 0; }
					else { len += 1; }
					ptr = (const char *)input->GetPtr(len);
					if (!ptr || len == 0) {
						throw std::runtime_error("unexpected end of data");
					}
					off = 0;
					ret = vmpu_execute(ctx.get(), ptr, len, &off);
					if (ret < 0) {
						std::ostringstream oss;
						oss << "MsgPack: A parse error occured at byte offset "
							<< off + bytecount;
						errorstr =  oss.str();
						state = ERROR;
						throw std::runtime_error(errorstr);
					} else {
						bytecount += off;
						input->Release(off);
						if (ret > 0) {
							state = END;
						}
					}
				}
				break;
			case END:
				TopAction()->EndDocument(this);
				state = DONE;
				break;
			case DONE:
				return 1;
			case ERROR:
				throw std::runtime_error(errorstr);
			}
		}
		return 0;
	}

	void MsgPackParserImpl::Reset() {
		state = START;
		vmpu_init(ctx.get());
		ctx->user.impl = this;
	}
}
