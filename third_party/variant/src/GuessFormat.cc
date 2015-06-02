/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/GuessFormat.h>
#include <ctype.h>

namespace libvariant {
	SerializeType GuessFormat(ParserInput* in) {
		unsigned len = 0;
		const char *ptr = (const char*)in->GetPtr(len);
		if (!ptr) {
#ifdef ENABLE_YAML
            return SERIALIZE_YAML;
#endif
			throw std::runtime_error(
					"libvariant::GuessFormat: Unable to guess input format, unexpected end of input."
					);
		}
		unsigned i = 0;
		while (true) {
			if (i >= len) {
				unsigned l = i+1;
				ptr = (const char*)in->GetPtr(l);
				if (!ptr) {
					throw std::runtime_error(
							"libvariant::GuessFormat: Unable to guess input format, unexpected end of input."
							);
				}
				len = l;
			}
			if (!isspace(ptr[i])) {
				if (ptr[i] == '<') {
#ifdef ENABLE_XML
					return SERIALIZE_XMLPLIST;
#else
					throw std::runtime_error("libvariant::GuessFormat: Input looks like XML but XML support is not available.");
#endif
				} else {
					// Even if the start looks like JSON it could still be YAML.
					// YAML can also deserialize everything the JSON parser can,
					// so choose YAML over JSON.
#ifdef ENABLE_YAML
					return SERIALIZE_YAML;
#else
					return SERIALIZE_JSON;
#endif
				}
			}
			++i;
		}
	}
}
