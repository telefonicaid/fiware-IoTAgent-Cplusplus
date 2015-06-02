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
 * \brief functino for printing out a stack trace
 * lookup the backtrace etc. in the man pages.
 * \author John Bridgman
 */
#include "StackTrace.h"
#include <execinfo.h>
#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <cstdio>

void PrintStack() {
    void* stack[STACKTRACE_MAXTRACE];
    size_t size = backtrace(stack, STACKTRACE_MAXTRACE);
    backtrace_symbols_fd(stack, size, 2);
}

std::string GetStack(unsigned ignore) {
    void* stack[STACKTRACE_MAXTRACE];
    size_t size = backtrace(stack, STACKTRACE_MAXTRACE);
    char **bt = backtrace_symbols(stack, size);
    if (!bt) {
        perror("backtrace_symbols");
        return "";
    }
    std::ostringstream oss;
    for (unsigned i = ignore; i < size; ++i) {
        oss << bt[i] << '\n';
    }
    free(bt);
    return oss.str();
}


