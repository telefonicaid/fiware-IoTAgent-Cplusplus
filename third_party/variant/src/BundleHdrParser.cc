/** \file
 * \author John Bridgman
 * \brief A parser for the Bundle header.
 */
#include "BundleHdrParser.h"
#include <Variant/ParserInput.h>
#include <string.h>
#include <algorithm>
#include <sstream>
#include <ctype.h>
#include "GuessScalar.h"

#define BUNDLE_VERSION_KEY	"bundle.version"

namespace libvariant {

	void BundleHdrParserImpl::ReadLine() {
		line_len = 0;
		unsigned len = 0;
		while (true) {
			line = (const char *)input->GetPtr(len);
			if (!line || line_len == len) {
				line = 0;
				return;
			}
			for (;line_len < len; ++line_len) {
				if (line[line_len] == '\n') {
					return;
				}
			}
			len += 4096;
		}
	}

	void BundleHdrParserImpl::RetireLine() {
		input->Release(line_len + 1);
		line = 0;
		line_len = 0;
	}

	bool BundleHdrParserImpl::ScanLine() {
		// Fill key and value
		//if (sscanf(line, " %[^: \t#] : %[^#]", &key[0], &value[0]) >= 1) {
		if (line_len == 0) { return false; }
		bool loop = true;
		const char *c = line;
		const char *e = line + line_len;
		key.clear();
		value.clear();
		while (isspace(*c) && c != e) { ++c; }
		loop = true;
		while (loop && c != e) {
			switch (*c) {
			case ':':
			case ' ':
			case '\t':
				loop = false;
				break;
			case '#':
				return false;
			default:
				key.push_back(*c);
				++c;
				break;
			}
		}
		key.push_back('\0');
		while (isspace(*c) && c != e) { ++c; }
		if (*c != ':') {
			return false;
		}
		++c;
		while (isspace(*c) && c != e) { ++c; }
		while (c != e && *c != '#') {
			value.push_back(*c);
			++c;
		}
		value.push_back('\0');
		return true;
	}

	int BundleHdrParserImpl::Run() {
		while (!action_stack.empty()) {
			switch (state) {
			case START:
				{
					unsigned len = 0;
					const char *ptr = (const char*)input->GetPtr(len);
					if (len == 0 || !ptr) {
						state = STOP;
						return 1;
					}
					state = BEGIN_MAP;
					TopAction()->BeginDocument(this);
				}
				break;
			case BEGIN_MAP:
				state = KEY;
				TopAction()->BeginMap(this, -1, 0, 0);
				break;
			case KEY:
				while (true) {
					ReadLine();
					if (!line) {
						state = END_MAP;
						break;
					}
					++line_num;
					bool success = ScanLine();
					RetireLine();
					if (success) {
						// If it is the version key, ignore this line
						if (strcmp(&key[0], BUNDLE_VERSION_KEY) == 0) {
							continue;
						}
						// Remove whitespace from end of value
						char *e = strchr(&value[0], '\0');
						while (--e >= &value[0] && isspace(*e)) { *e = '\0'; }
						state = VALUE;
						TopAction()->Scalar(this, &key[0], strlen(&key[0]), 0, 0);
						key.clear();
						break;
					}
				}
				break;
			case VALUE:
				{
					char *list_context = 0;
					int i = 0;
					bool bar_at_end = (value.size() > 1 && value[value.size()-2] == '|');
					while (true) {
						char *v = strtok_r((i == 0 ? &value[0] : 0), "|", &list_context);
						if (!v) {
							if (i <= 1 && !bar_at_end) {
								state = KEY;
								list.clear();
								GuessScalar(&value[0], strlen(&value[0]), 0, 0, this, TopAction());
								break;
							} else {
								state = LIST;
								value.clear();
								TopAction()->BeginList(this, -1, 0, 0);
								break;
							}
						}
						list.push_back(v);
						++i;
					}
				}
				break;
			case LIST:
				if (!list.empty()) {
					char *v = list.front();
					list.pop_front();
					GuessScalar(v, strlen(v), 0, 0, this, TopAction());
				} else {
					state = KEY;
					TopAction()->EndList(this);
				}
				break;
			case END_MAP:
				state = END;
				TopAction()->EndMap(this);
				break;
			case END:
				state = STOP;
				TopAction()->EndDocument(this);
				break;
			case STOP:
				return 1;
			default:
				errorstr = "BundleHdrParser: Inconsistent state";
				throw std::runtime_error(errorstr);
			}
		}
		return 0;
	}

	void BundleHdrParserImpl::Reset() {
		state = START;
		line = 0;
		line_len = 0;
		key.clear();
		value.clear();
		list.clear();
		line_num = 0;
	}

	BundleHdrParserImpl::BundleHdrParserImpl(shared_ptr<ParserInput> i)
		: input(i),
		state(START),
		line(0),
		line_len(0),
		line_num(0)
	{}

	BundleHdrParserImpl::~BundleHdrParserImpl() {}

	unsigned BundleHdrParserImpl::GetLine() const { return line_num; }

	bool BundleHdrParserImpl::Done() const {
		return state == STOP;
	}

	bool BundleHdrParserImpl::Error() const {
		return state > STOP;
	}

	bool BundleHdrParserImpl::Ok() const {
		return state != STOP;
	}

	std::string BundleHdrParserImpl::ErrorStr() const {
		return errorstr;
	}

}
