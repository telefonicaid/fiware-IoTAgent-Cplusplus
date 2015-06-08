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
#ifndef VARIANT_PAYLOAD_H
#define VARIANT_PAYLOAD_H
#pragma once
#include <Variant/Variant.h>
#include <Variant/Emitter.h>
#include <Variant/Parser.h>
namespace libvariant {

	// Serialize and Deserialize a header, null byte, and a binary payload
	//
	// All restrictions of the given serialization format are required.
	//
	// Parameters for payload data location:
	// "length_path": path to where the payload length should be
	// placed for serialization and where the payload length should be read
	// from for deserialization
	// Defaults to: 
#define VARIANT_PAYLOAD_LENGTH_PATH "payload/length"
#define VARIANT_BUNDLE_PAYLOAD_LENGTH_PATH "payload.length"
	// "data_path": path to where the payload is located for
	// serialization and path to store payload for deserialization
	// Defaults to:
#define VARIANT_PAYLOAD_DATA_PATH "payload/data"
#define VARIANT_BUNDLE_PAYLOAD_DATA_PATH "payload.data"
	// The defaults are the values for compatibility with the bundle format.
	//
	// Serialize only parameters:
	// "ignore_payload": true|false (default false)
	// Don't output the payload, only the header, alignment and null byte.
	// If the payload exists in the Variant, use the length for the payload length
	// value, if not look in parameters for the payload length value, if not there
	// look in the Variant for the payload length.
	// "payload_length": num
	// Payload length override, put this number in as the payload length
	// regardless of what size the blob is.
	// Note that unless ingnore_payload is true, the emitter will try to output the whole
	// payload (and no more) regardless of what this number is.

	void SerializeWithPayload(shared_ptr<EmitterOutput> out, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	Variant DeserializeWithPayload(shared_ptr<ParserInput> in, SerializeType type,
		   	Variant params = Variant::NullType, bool be_safe=true);

	std::string SerializeWithPayload(Variant v, SerializeType type, Variant params = Variant::NullType);
	void SerializeWithPayload(const std::string &filename, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	void SerializeWithPayload(FILE *f, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	void SerializeWithPayload(std::streambuf *sb, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);
	unsigned SerializeWithPayload(void *ptr, unsigned len, Variant v, SerializeType type,
		   	Variant params = Variant::NullType);

	Variant DeserializeWithPayload(const std::string &str, SerializeType type,
			Variant params = Variant::NullType);
	/// If the be_safe parameter is false, then it is considered OK
	// to __/NOT/__ copy the payload and just reference it in the BlobPtr.
	// This is only safe when you know your buffer is going to stick around
	// until after your done with the resulting Variant.
	// (This is to support zero copy)
	// No null terminated string version provided as these formats contain nulls.
	Variant DeserializeWithPayload(const void *ptr, unsigned len, SerializeType type,
			Variant params = Variant::NullType, bool be_safe=true);
	Variant DeserializeWithPayloadFile(const char *filename, SerializeType type,
			Variant params = Variant::NullType);
	Variant DeserializeWithPayloadFile(FILE *f, SerializeType type,
			Variant params = Variant::NullType);
	Variant DeserializeWithPayloadFile(std::streambuf *sb, SerializeType type,
			Variant params = Variant::NullType);


	/// Bundle is a simple format used internally at ARL for passing around
	// a textual key value map plus a binary payload. The payload is often a very large
	// array of numbers that would produce unacceptable overhead to serialize into text.
	// These serialize routines assume that the passed in Variant is of MapType
	// and if a key named "payload.data" exists it is to be written as the payload
	// of the Bundle.
	//
	// If a key "payload.data" does not exist, these are almost the same as using the BundleHdrEmitter
	// except it adds a null byte at the end.
	inline std::string SerializeBundle(Variant v)
   	{ return SerializeWithPayload(v, SERIALIZE_BUNDLEHDR); }
	inline void SerializeBundle(const std::string &filename, Variant v)
   	{ SerializeWithPayload(filename, v, SERIALIZE_BUNDLEHDR); }
	inline void SerializeBundle(FILE *f, Variant v)
   	{ SerializeWithPayload(f, v, SERIALIZE_BUNDLEHDR); }
	inline void SerializeBundle(std::streambuf *sb, Variant v)
   	{ SerializeWithPayload(sb, v, SERIALIZE_BUNDLEHDR); }
	inline unsigned SerializeBundle(void *ptr, unsigned len, Variant v)
   	{ return SerializeWithPayload(ptr, len, v, SERIALIZE_BUNDLEHDR); }

	inline Variant DeserializeBundle(const std::string &str)
   	{ return DeserializeWithPayload(str, SERIALIZE_BUNDLEHDR); }
	/// If the be_safe parameter is false, then it is considered OK
	// to __/NOT/__ copy the payload and just reference it in the BlobPtr.
	// This is only safe when you know your buffer is going to stick around
	// until after your done with the resulting Variant.
	// (This is to support zero copy)
	// No null terminated string version provided as bundles contain nulls.
	inline Variant DeserializeBundle(const void *ptr, unsigned len, bool be_safe=true)
	{ return DeserializeWithPayload(ptr, len, SERIALIZE_BUNDLEHDR, Variant::NullType, be_safe); }
	inline Variant DeserializeBundleFile(const char *filename)
   	{ return DeserializeWithPayloadFile(filename, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleFile(FILE *f) { return DeserializeWithPayloadFile(f, SERIALIZE_BUNDLEHDR); }
	inline Variant DeserializeBundleFile(std::streambuf *sb)
   	{ return DeserializeWithPayloadFile(sb, SERIALIZE_BUNDLEHDR); }

}
#endif
