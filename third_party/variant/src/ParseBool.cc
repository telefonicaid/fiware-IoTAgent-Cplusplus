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
 */
#include "ParseBool.h"
#include <ctype.h>


bool ParseBool(const std::string &str) {
    static const char NO_STR[] = "no";
    static const char FALSE_STR[] = "false";
    bool result = true;
    std::string::const_iterator cur = str.begin();
    const std::string::const_iterator end = str.end();
    while (cur != end && isspace(*cur)) { ++cur; }
    if (cur == end) return result;
    const char *cmpstr = 0;
    if (tolower(*cur) == NO_STR[0]) {
        cmpstr = &NO_STR[1];
    } else if (tolower(*cur) == FALSE_STR[0]) {
        cmpstr = &FALSE_STR[1];
    } else if (*cur == '0') {
        ++cur;
        while (cur != end && *cur == '0') { ++cur; }
        while (cur != end && isspace(*cur)) { ++cur; }
        if (cur == end) { result = false; }
        return result;
    }
    if (cmpstr != 0) {
        ++cur;
        int i = 0;
        while (cur != end && cmpstr[i] != '\0' && tolower(*cur) == cmpstr[i]) {
            ++cur; ++i;
        }
        while (cur != end && isspace(*cur)) { ++cur; }
        if (cur == end) { result = false; }
    }
    return result;
}
