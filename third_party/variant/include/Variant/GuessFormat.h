/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_GUESSFORMAT_H
#define VARIANT_GUESSFORMAT_H
#pragma once
#include <Variant/Variant.h>
#include <Variant/Parser.h>
namespace libvariant {

	///
	// Try to guess the format of the input without removing any
	// input from the input object.
	//
	// Currently only looks at the first non-whitespace character
	// and if it is '<' then says XMLPLIST, otherwise says YAML
	// if enabled otherwise JSON.
	//
	SerializeType GuessFormat(ParserInput* in);
}
#endif
