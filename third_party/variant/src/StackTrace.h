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
 * \brief function for printing out a stack trace
 * Note that we need to pass -rdynamic to the linker to
 * be able to see the function names with these functions.
 * \author John Bridgman
 */

#ifndef STACKTRACE_H
#define STACKTRACE_H
#pragma once
#include <string>
#ifndef STACKTRACE_MAXTRACE 
#define STACKTRACE_MAXTRACE 100
#endif
/**
 * Prints the stack trace to stderr
 * Should work even if we are out of memory
 */
void PrintStack();

/**
 * Returns a std::string to the stack trace.
 * May fail if out of memory.
 * \param ignore the number of top stacks to ignore. This is
 * useful because this function is often used in error messages
 * and it is useless garbage to report the stack from the error
 * handler.
 */
std::string GetStack(unsigned ignore = 0);

#endif

