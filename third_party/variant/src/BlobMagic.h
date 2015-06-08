/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_BLOBMAGIC_H
#define VARIANT_BLOBMAGIC_H
#pragma once

// We implement blobs by prepending the encoding with a special value
// of \0BLOB to identify it for those serialization formats that do not
// provide a method to distinguish strings and binary data.

// Raw encoding
#define MAGIC_BLOB_TAG "\0BLOB"
// Length of raw encoding
#define MAGIC_BLOB_LENGTH 5
// The string encoding for json
#define MAGIC_BLOB_JSONSTR "\\u0000BLOB"

#endif
