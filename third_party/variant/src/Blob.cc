/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Blob.h>
#include <stdlib.h>
#include <new>
#include <string.h>
#include <algorithm>

namespace libvariant {

	static void MallocFree(void *ptr, void *) {
		free(ptr);
	}

	shared_ptr<Blob> Blob::Create(void *ptr, unsigned len, BlobFreeFunc ffunc, void *context)
	{
		struct iovec iov = { ptr, len };
		return shared_ptr<Blob>(new Blob(&iov, 1, ffunc, context));
	}

	BlobPtr Blob::Create(struct iovec *iov, unsigned iov_len, BlobFreeFunc ffunc, void *context) {
		return BlobPtr(new Blob(iov, iov_len, ffunc, context));
	}

	shared_ptr<Blob> Blob::CreateCopy(const void *ptr, unsigned len) {
		struct iovec iov = { (void*)ptr, len };
		return CreateCopy(&iov, 1);
	}

	BlobPtr Blob::CreateCopy(const struct iovec *iov, unsigned iov_len) {
		unsigned len = 0;
		for (unsigned i = 0; i < iov_len; ++i) { len += iov[i].iov_len; }
		void *data = 0;
#ifdef __APPLE__
		// TODO: Remove when apple fixes this error.
		if (posix_memalign(&data, 64, std::max(len, 1u)) != 0) {
			throw std::bad_alloc();
		}
#else
		if (posix_memalign(&data, 64, len) != 0) {
			throw std::bad_alloc();
		}
#endif
		for (unsigned i = 0, copied = 0; i < iov_len; ++i) {
			memcpy((char*)data + copied, iov[i].iov_base, iov[i].iov_len);
			copied += iov[i].iov_len;
		}
		struct iovec v = { data, len };
		return shared_ptr<Blob>(new Blob(&v, 1, MallocFree, 0));
	}

	shared_ptr<Blob> Blob::CreateFree(void *ptr, unsigned len) {
		struct iovec iov = { ptr, len };
		return CreateFree(&iov, 1);
	}

	BlobPtr Blob::CreateFree(struct iovec *iov, unsigned iov_len) {
		return shared_ptr<Blob>(new Blob(iov, iov_len, MallocFree, 0));
	}

	shared_ptr<Blob> Blob::CreateReferenced(void *ptr, unsigned len) {
		struct iovec iov = { ptr, len };
		return CreateReferenced(&iov, 1);
	}

	BlobPtr Blob::CreateReferenced(struct iovec *iov, unsigned iov_len) {
		return shared_ptr<Blob>(new Blob(iov, iov_len, 0, 0));
	}

	Blob::Blob(struct iovec *v, unsigned l, BlobFreeFunc f, void *c)
		: iov(v, v+l), free_func(f), ctx(c)
	{
	}

	Blob::~Blob() {
		if (free_func) {
			for (unsigned i = 0; i < iov.size(); ++i) {
				free_func(iov[i].iov_base, ctx);
			}
		}
		iov.clear();
		free_func = 0;
		ctx = 0;
	}

	shared_ptr<Blob> Blob::Copy() const {
		return CreateCopy(&iov[0], iov.size());
	}

	unsigned Blob::GetTotalLength() const {
		unsigned size = 0;
		for (unsigned i = 0; i < iov.size(); ++i) {
			size += iov[i].iov_len;
		}
		return size;
	}

	int Blob::Compare(ConstBlobPtr other) const {
		unsigned our_offset = 0;
		unsigned oth_offset = 0;
		unsigned i = 0, j = 0;
		while (i < GetNumBuffers() && j < other->GetNumBuffers()) {
			unsigned len = std::min(GetLength(i) - our_offset, other->GetLength(j) - oth_offset);
			int res = memcmp((char*)(GetPtr(i)) + our_offset, (char*)(other->GetPtr(j)) + oth_offset, len);
			if (res != 0) { return res; }
			our_offset += len;
			if (our_offset >= GetLength(i)) {
				our_offset = 0;
				++i;
			}
			oth_offset += len;
			if (oth_offset >= other->GetLength(j)) {
				oth_offset = 0;
				++j;
			}
		}
		return 0;
	}
}

