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
#ifndef VARIANT_PARSERINPUT_H
#define VARIANT_PARSERINPUT_H
#pragma once
#include <stdio.h>
#include <iosfwd>
#include <vector>
#include <string>

namespace libvariant {

	class ParserInput {
	public:
		virtual ~ParserInput();
		// Get a pointer and length to the unprocessed data that is available
		// (return null on end of data) len being an in/out parameter,
		// the initial value of len is how much the parser wants. An input value of zero
		// in means give whatever is currently available.
		virtual const void *GetPtr(unsigned &len) = 0;

		// Tell the input that the parser is done with len bytes from the last
		// GetPtr
		virtual void Release(unsigned len) = 0;
	};

	class ParserStreamInput : public ParserInput {
	public:
		ParserStreamInput(unsigned buffer_len);
		virtual const void *GetPtr(unsigned &len);
		virtual void Release(unsigned len);
		/// return number of bytes read into ptr, 0 on EOF
		// and throw on error
		virtual unsigned Read(void *ptr, unsigned len) = 0;
	protected:
		std::vector<char> buffer;
		unsigned num;
		unsigned offset;
		bool eof;
	};

	class ParserMemoryInput : public ParserInput {
	public:
		ParserMemoryInput(const void *ptr, unsigned len);
		virtual const void *GetPtr(unsigned &len);
		virtual void Release(unsigned len);
	protected:
		const void *data_ptr;
		unsigned data_len;
		unsigned offset;
	};

	class ParserStringInput : public ParserInput {
	public:
		ParserStringInput(const std::string &str);
		virtual const void *GetPtr(unsigned &len);
		virtual void Release(unsigned len);
	protected:
		const std::string val;
		unsigned offset;
	};

	class ParserFileInput : public ParserStreamInput {
	public:
		ParserFileInput(FILE *f);
		virtual unsigned Read(void *ptr, unsigned len);
	protected:
		FILE *file;
	};

	class ParserFilenameInput : public ParserFileInput {
	public:
		ParserFilenameInput(const char *filename, const char *mode = "r");
		~ParserFilenameInput();
	};

	class ParserStreambufInput : public ParserStreamInput {
	public:
		ParserStreambufInput(std::streambuf *sb);
		unsigned Read(void *ptr, unsigned len);
	protected:
		std::streambuf *streambuf;
	};

}
#endif
