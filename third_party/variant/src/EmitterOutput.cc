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

#include <Variant/EmitterOutput.h>
#include <stdexcept>
#include <streambuf>
#include <sstream>
#include <string.h>

namespace libvariant {

	EmitterOutput::~EmitterOutput() {}

	EmitterFileOutput::EmitterFileOutput(FILE *f, bool c) : file(f), num_bytes(0), cls(c) {}

	EmitterFileOutput::~EmitterFileOutput() {
		if (cls && file) {
			fclose(file);
		}
		file = 0;
	}

	unsigned EmitterFileOutput::Write(const void *ptr, unsigned len) {
		unsigned num = fwrite(ptr, 1, len, file);
		num_bytes += num;
		return num;
	}

	void EmitterFileOutput::Flush() {
	   	fflush(file);
   	}

	EmitterFilenameOutput::EmitterFilenameOutput(const char *filename, const char *mode)
		: EmitterFileOutput(fopen(filename, mode), true)
	{
		if (!file) {
			std::ostringstream oss;
			oss << "Unable to open \"" << filename << "\" with mode \""
			   	<< mode << "\"";
			throw std::runtime_error(oss.str());
		}
	}

	EmitterStreambufOutput::EmitterStreambufOutput(std::streambuf *sb, bool d)
	   	: streambuf(sb), num_bytes(0), del(d) {}

	EmitterStreambufOutput::~EmitterStreambufOutput() {
		if (del && streambuf) {
			delete streambuf;
		}
		streambuf = 0;
	}

	unsigned EmitterStreambufOutput::Write(const void *ptr, unsigned len) {
		unsigned num = streambuf->sputn((const char*)ptr, len);
		num_bytes += num;
		return num;
	}

	void EmitterStreambufOutput::Flush() {
	   	streambuf->pubsync();
   	}

	EmitterMemoryOutput::EmitterMemoryOutput(void *ptr, unsigned len, unsigned *out_len)
		: data_ptr(ptr), length(len), out_length(out_len)
   	{
		if (0 == out_length) {
			out_length = &out_length_dummy;
		}
	}

	unsigned EmitterMemoryOutput::Write(const void *ptr, unsigned len) {
		if (*out_length + len > length) {
			throw std::length_error("Emitter memory output buffer to small.");
		}
		memcpy(((char*)data_ptr + *out_length), ptr, len);
		*out_length += len;
		return len;
	}

}
