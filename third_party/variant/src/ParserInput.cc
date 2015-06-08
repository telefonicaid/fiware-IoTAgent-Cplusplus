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

#include <Variant/ParserInput.h>
#include <stdexcept>
#include <streambuf>
#include <sstream>
#include <string.h>
#include <errno.h>

namespace libvariant {

	//----------------------------------------------------------------------
	// ParserInput

	ParserInput::~ParserInput() {}

	//----------------------------------------------------------------------
	// ParserStreamInput

	ParserStreamInput::ParserStreamInput(unsigned buffer_len)
		: buffer(buffer_len),
		num(0),
		offset(0),
		eof(false)
	{

	}

	const void *ParserStreamInput::GetPtr(unsigned &len) {
		if (eof) {
			len = num;
			if (num == 0) { return 0; }
			return &buffer[offset];
		}
		if (len <= num && num > 0) {
			len = num;
			return &buffer[offset];
		}
		if (len + offset > buffer.size()) {
			if (len > buffer.size()) {
				std::vector<char> newbuffer(len);
				memmove(&newbuffer[0], &buffer[offset], num);
				std::swap(newbuffer, buffer);
			} else {
				memmove(&buffer[0], &buffer[offset], num);
			}
			offset = 0;
		}
		while (!eof && (num < len || num == 0)) {
			unsigned to_read = buffer.size() - num - offset;
			unsigned num_read = Read(&buffer[num + offset], to_read);
			if (num_read < to_read) { eof = true; }
			num += num_read;
		}
		len = num;
		if (num == 0 && eof) { return 0; }
		return &buffer[offset];
	}

	void ParserStreamInput::Release(unsigned len) {
		if (len > num) {
			throw std::runtime_error("ParserStreamInput: trying to release more than was aquired.");
		} else if (len == num) {
			num = 0;
			offset = 0;
		} else {
			offset += len;
			num -= len;
		}
	}

	//----------------------------------------------------------------------
	// ParserMemoryInput

	ParserMemoryInput::ParserMemoryInput(const void *ptr, unsigned len)
	   	: data_ptr(ptr), data_len(len), offset(0) {}

	const void *ParserMemoryInput::GetPtr(unsigned &len) {
		if (offset > data_len) {
			throw std::length_error("Parser input buffer underflow.");
		}
		len = data_len - offset;
		if (len > 0) {
			return (const void*)((char*)data_ptr + offset);
		} else {
			return 0;
		}
	}

	void ParserMemoryInput::Release(unsigned len) {
		offset += len;
	}

	//----------------------------------------------------------------------
	// ParserFileInput
	
	ParserFileInput::ParserFileInput(FILE *f)
		: ParserStreamInput(8192), file(f)
	{
	}

	unsigned ParserFileInput::Read(void *ptr, unsigned len) {
		unsigned num_read = fread(ptr, 1, len, file);
		int error = errno;
		if (num_read) return num_read;
		if (ferror(file)) {
			std::ostringstream oss;
			oss << "Error reading from file: ";
			oss << strerror(error);
			throw std::runtime_error(oss.str());
		}
		return 0;
	}

	//----------------------------------------------------------------------
	// ParserFilenameInput

	ParserFilenameInput::ParserFilenameInput(const char *filename, const char *mode)
		: ParserFileInput(fopen(filename, mode))
	{
		if (!file) {
			std::ostringstream oss;
			oss << "Unable to open " << filename;
			throw std::runtime_error(oss.str());
		}
	}

	ParserFilenameInput::~ParserFilenameInput() {
		fclose(file);
	}

	//----------------------------------------------------------------------
	// ParserStreambufInput

	ParserStreambufInput::ParserStreambufInput(std::streambuf *sb)
		: ParserStreamInput(8192), streambuf(sb)
	{
	}

	unsigned ParserStreambufInput::Read(void *ptr, unsigned len) {
		return streambuf->sgetn((char*)ptr, len);
	}
}

