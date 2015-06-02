/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_BASE64_H
#define VARIANT_BASE64_H
#pragma once
#include <sys/uio.h>
namespace libvariant {
	/// Encode the binary blob of len pointed to by inptr
	//into a base64 string encoding in the buffer pointed to by
	//outptr. The buffer for outptr should be at least Base64EncodeSize(len)
	//in length, returns the actual length of the output
	unsigned Base64Encode(char *outptr, const void *inptr, unsigned len, unsigned cpl = 72);
	unsigned Base64Encode(char *outptr, const struct iovec *iov, unsigned iov_len, unsigned cpl = 72);
	unsigned Base64EncodeSize(unsigned len, unsigned cpl=72);
	/// Decode a base64 encoded string of length len to a binary blob
	//The blob will be no larger than Base64DecodeSize(len)
	//returns the actual size of the blob
	unsigned Base64Decode(void *outptr, const char *inptr, unsigned len);
	unsigned Base64DecodeSize(unsigned len);
}
#endif

