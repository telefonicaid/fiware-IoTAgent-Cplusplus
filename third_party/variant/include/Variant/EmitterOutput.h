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
 *
 * The end user should not need to look at this file unless they are
 * creating custom outputs for the emitters.
 */
#ifndef VARIANT_EMITTEROUTPUT_H
#define VARIANT_EMITTEROUTPUT_H
#pragma once
#include <stdio.h>
#include <iosfwd>

namespace libvariant {

	class EmitterOutput {
	public:
		virtual ~EmitterOutput();
		/// Return the number of bytes written, or throw an exception
		virtual unsigned Write(const void *ptr, unsigned len) = 0;
		virtual void Flush() {}
		virtual unsigned NumBytesWritten() const = 0;
	};

	class EmitterFileOutput : public EmitterOutput {
	public:
		EmitterFileOutput(FILE *f, bool c);
		~EmitterFileOutput();
		virtual unsigned Write(const void *ptr, unsigned len);
		virtual void Flush();
		virtual unsigned NumBytesWritten() const { return num_bytes; }
	protected:
		FILE *file;
		unsigned num_bytes;
		bool cls;
	};

	class EmitterFilenameOutput : public EmitterFileOutput {
	public:
		EmitterFilenameOutput(const char *filename, const char *mode="w");
	};

	class EmitterStreambufOutput : public EmitterOutput {
	public:
		EmitterStreambufOutput(std::streambuf *sb, bool d);
		~EmitterStreambufOutput();
		virtual unsigned Write(const void *ptr, unsigned len);
		virtual void Flush();
		virtual unsigned NumBytesWritten() const { return num_bytes; }
	protected:
		std::streambuf *streambuf;
		unsigned num_bytes;
		bool del;
	};

	class EmitterMemoryOutput : public EmitterOutput {
	public:
		EmitterMemoryOutput(void *ptr, unsigned len, unsigned *out_len = 0);

		virtual unsigned Write(const void *ptr, unsigned len);

		virtual unsigned NumBytesWritten() const { return *out_length; }
	protected:
		void *data_ptr;
		unsigned length;
		unsigned out_length_dummy;
		unsigned *out_length;
	};

}
#endif
