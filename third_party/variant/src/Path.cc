/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Path.h>
#include <sstream>

namespace libvariant {

	void ParsePath(Path &parsed_path, const std::string &path) {
		const char *c = path.c_str();
		const char *e = c + path.length();
		std::string frag;
		bool escape = false;
		bool index = false;
		bool key = false;
		while (c != e) {
			if (escape) {
				frag.append(c, 1);
				escape = false;
			} else if (index) {
				if (*c == ']') {
					std::istringstream iss(frag);
					unsigned long long i = 0;
					if (!(iss >> i)) {
						throw std::runtime_error("Error parsing Variant path: \"" + frag + "\" is not a number.");
					}
					parsed_path.push_back(i);
					frag.clear();
					index = false;
				} else {
					frag.append(c, 1);
				}
			} else if (key) {
				switch (*c) {
				case '/':
					parsed_path.push_back(frag);
					frag.clear();
					break;
				case '[':
					parsed_path.push_back(frag);
					frag.clear();
					key = false;
					index = true;
					break;
				case '\\':
					escape = true;
					break;
				default:
					frag.append(c, 1);
					break;
				}
			} else  {
				switch (*c) {
				case '/':
					frag.clear();
					key = true;
					break;
				case '[':
					frag.clear();
					index = true;
					break;
				default:
					frag.clear();
					key = true;
					frag.append(c, 1);
					break;
				}
			}
			++c;
		}
		if (!frag.empty()) { parsed_path.push_back(frag); }
	}

	Path ParsePath(const std::string &path) {
		Path parsed_path;
		ParsePath(parsed_path, path);
		return parsed_path;
	}

	std::string PathString(const Path &path) {
		if (path.empty()) { return "/"; }
		std::ostringstream oss;
		for (Path::const_iterator i(path.begin()), e(path.end()); i != e; ++i) {
			if (i->IsString()) {
				std::string fragment = i->AsString();
				oss << "/";
				for (unsigned i = 0; i < fragment.size(); ++i) {
					switch (fragment[i]) {
					case '/':
						oss << "\\/";
						break;
					case '[':
						oss << "\\[";
						break;
					case ']':
						oss << "\\]";
						break;
					case '\\':
						oss << "\\\\";
						break;
					default:
						oss << fragment[i];
						break;
					}
				}
			} else if (i->IsNumber()) {
				oss << "[" << i->AsUnsigned() << "]";
			} else {
				throw BadPathError("Unknown path element type");
			}
		}
		return oss.str();
	}

}
