/** \file
 * \author John Bridgman
 * \brief 
 */
#include "GuessScalar.h"
#include <stdlib.h>
#include <regex.h>
#include <errno.h>
#include <limits>
#include <limits.h>
#include <string.h>

namespace libvariant {

	namespace {
		// Apparently the first match result is the match of the entire pattern
		static const char *match_string = "^[[:space:]\n]*("
					"(null|Null|NULL|~)" // Null 2
					"|(true|True|TRUE)" // True  3
					"|(false|False|FALSE)" // False 4
					"|([-+]?[0-9]+)" // int10 5
					"|0o([0-7]+)" // int8 6
					"|0x([0-9a-fA-F]+)" // int16 7
					"|([-+]?((\\.[0-9]+)|([0-9]+(\\.[0-9]*)?))([eE][-+]?[0-9]+)?)" // float 8 (9,10,11,12,13)
					"|([-+]?\\.inf|\\.Inf|\\.INF)" // inf 14
					"|(\\.nan|\\.Nan|\\.NAN)" // nan 15
					")[[:space:]\n]*$";
		enum {
			MATCH_NULL = 2,
			MATCH_TRUE = 3,
			MATCH_FALSE = 4,
			MATCH_INT10 = 5,
			MATCH_INT8 = 6,
			MATCH_INT16 = 7,
			MATCH_FLOAT = 8,
			MATCH_INF = 14,
			MATCH_NAN = 15,
			MATCH_MAX = 16
		};

		class GuessRegex {
		public:
			GuessRegex();
			~GuessRegex();
			regex_t guessregex;

		};

		GuessRegex::GuessRegex() {
			int err = 0;
			memset(&guessregex, 0, sizeof(regex_t));
			err = regcomp(&guessregex, match_string, REG_EXTENDED); 
			if (err) {
				std::vector<char> buffer(8192);
				regerror(err, &guessregex, &buffer[0], buffer.size());
				std::ostringstream oss;
				oss << "libvariant library initialization failure: Failed to initialize type guessing regex with error: "
					<< &buffer[0];
				fputs(oss.str().c_str(), stderr);
				throw std::runtime_error(oss.str());
			}
		}


		GuessRegex::~GuessRegex() {
			regfree(&guessregex);
		}
	}

	void GuessScalar(const char *value, unsigned length, const char *anchor, const char *tag,
		   	ParserImpl *p, ParserActions *action) {
		static GuessRegex guessregex;
		regmatch_t match[MATCH_MAX];
		if (regexec(&guessregex.guessregex, value, MATCH_MAX, &match[0], 0) == 0) {
			if (match[MATCH_NULL].rm_so != -1) {
				action->Null(p, anchor, tag);
			} else if (match[MATCH_TRUE].rm_so != -1) {
				action->Scalar(p, true, anchor, tag);
			} else if (match[MATCH_FALSE].rm_so != -1) {
				action->Scalar(p, false, anchor, tag);
			} else if (match[MATCH_INT10].rm_so != -1) {
				long long val;
				val = strtoll(&value[match[MATCH_INT10].rm_so], 0, 10);
				if (val == LLONG_MAX && errno == ERANGE) {
					unsigned long long v;
					v = strtoull(&value[match[MATCH_INT10].rm_so], 0, 10);
					action->Scalar(p, uintmax_t(v), anchor, tag);
				} else {
					action->Scalar(p, intmax_t(val), anchor, tag);
				}
			} else if (match[MATCH_INT8].rm_so != -1) {
				unsigned long long val;
				val = strtoull(&value[match[MATCH_INT8].rm_so], 0, 8);
				action->Scalar(p, uintmax_t(val), anchor, tag);
			} else if (match[MATCH_INT16].rm_so != -1) {
				unsigned long long val;
				val = strtoull(&value[match[MATCH_INT16].rm_so], 0, 16);
				action->Scalar(p, uintmax_t(val), anchor, tag);
			} else if (match[MATCH_FLOAT].rm_so != -1) {
				double val = strtod(&value[match[MATCH_FLOAT].rm_so], 0);
				action->Scalar(p, val, anchor, tag);
			} else if (match[MATCH_INF].rm_so != -1) {
				double val = std::numeric_limits<double>::infinity();
				if (value[match[MATCH_INF].rm_so] == '-') {
					val = -val;
				}
				action->Scalar(p, val, anchor, tag);
			} else if (match[MATCH_NAN].rm_so != -1) {
				double val = std::numeric_limits<double>::quiet_NaN();
				action->Scalar(p, val, anchor, tag);
			} else {
				action->Scalar(p, value, length, anchor, tag);
			}
		} else {
			action->Scalar(p, value, length, anchor, tag);
		}
	}

}
