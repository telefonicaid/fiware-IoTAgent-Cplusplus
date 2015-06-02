/** \file
 * \author John Bridgman
 * \brief A simple routine that "guesses" what type a scalar is.
 */
#ifndef VARIANT_GUESSSCALAR_H
#define VARIANT_GUESSSCALAR_H
#pragma once
#include <Variant/Parser.h>
namespace libvariant {
	void GuessScalar(const char *value, unsigned length, const char *anchor, const char *tag,
		   	ParserImpl *p, ParserActions *action);
}
#endif
