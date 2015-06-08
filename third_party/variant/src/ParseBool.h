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
#ifndef PARSEBOOL_H
#define PARSEBOOL_H
#pragma once
#include <string>
/**
 * \brief Parses string for a boolean value.
 *
 * Considers no, false, 0 and any abreviation
 * like "   f" to be false
 * and everything else to be true (including
 * the empty string).
 * Ignores whitespace.
 * Note "false blah" is true!
 * \return true or false
 * \param str the string to parse
 */
bool ParseBool(const std::string &str);
#endif
