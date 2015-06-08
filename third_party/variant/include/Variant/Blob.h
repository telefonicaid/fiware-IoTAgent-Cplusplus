/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_BLOB_H
#define VARIANT_BLOB_H
#pragma once
#include <Variant/SharedPtr.h>
#include <vector>
#include <sys/uio.h>

namespace libvariant {
	class Blob;
	typedef shared_ptr<Blob> BlobPtr;
	typedef shared_ptr<const Blob> ConstBlobPtr;

	typedef void (*BlobFreeFunc)(void *ptr, void *ctx);

	class Blob {
	public:

		/// Reference the buffer and call ffunc when Blob no longer references it
		static BlobPtr Create(void *ptr, unsigned len, BlobFreeFunc ffunc, void *context);
		/// Will call ffunc on each buffer in iov
		static BlobPtr Create(struct iovec *iov, unsigned iov_len, BlobFreeFunc ffunc, void *context);
		/// Copy the data into a newly allocatd buffer
		static BlobPtr CreateCopy(const void *ptr, unsigned len);
		static BlobPtr CreateCopy(const struct iovec *iov, unsigned iov_len);
		/// Reference the buffer and when done call free to free the buffer
		static BlobPtr CreateFree(void *ptr, unsigned len);
		static BlobPtr CreateFree(struct iovec *iov, unsigned iov_len);
		/// Reference the buffer, assume static or other allocation lifetime where
		// calling a free function is not required
		static BlobPtr CreateReferenced(void *ptr, unsigned len);
		static BlobPtr CreateReferenced(struct iovec *iov, unsigned iov_len);
		~Blob();

		BlobPtr Copy() const;

		void *GetPtr(unsigned i) { return iov[i].iov_base; }
		const void *GetPtr(unsigned i) const { return iov[i].iov_base; }
		unsigned GetLength(unsigned i) const { return iov[i].iov_len; }
		struct iovec *GetIOVec() { return &iov[0]; }
		const struct iovec *GetIOVec() const { return &iov[0]; }
		unsigned GetNumBuffers() const { return iov.size(); }
		unsigned GetTotalLength() const;

		int Compare(ConstBlobPtr other) const;

	private:
		Blob();
		Blob(const Blob&);
		Blob(struct iovec *v, unsigned l, BlobFreeFunc f, void *c);
		Blob operator=(const Blob&);

		std::vector<struct iovec> iov;
		BlobFreeFunc free_func;
		void *ctx;
	};

}
#endif
