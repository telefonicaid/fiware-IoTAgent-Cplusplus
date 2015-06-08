/** \file
 * \author John Bridgman
 * \brief A simple convert utility that can read in
 * the json, yaml or xml formats and then write out
 * any of those formats.
 */
#include <Variant/Variant.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

using namespace libvariant;

int main(int argc, char **argv) {
	enum Op {
		NOOP,
		JSON, YAML, XML
	};
	SerializeType input = SERIALIZE_YAML;
	SerializeType output = SERIALIZE_YAML;
	Variant params = Variant::MapType;
	params["pretty"] = true;
	while (true) {
		int c = getopt(argc, argv, "jyxJYXhp:");
		if (c == -1) { break; }
		switch (c) {
		case 'j':
			input = SERIALIZE_JSON;
			break;
		case 'y':
			input = SERIALIZE_YAML;
			break;
		case 'x':
			input = SERIALIZE_XMLPLIST;
			break;
		case 'J':
			output = SERIALIZE_JSON;
			break;
		case 'Y':
			output = SERIALIZE_YAML;
			break;
		case 'X':
			output = SERIALIZE_XMLPLIST;
			break;
		case 'p':
			params["pretty"] = Variant(optarg).AsBool();
			break;
		case 'h':
		default:
			printf("Usage: %s [options] infile outfile\n", *argv);
			printf("Options:\n"
					"\t-j|-y|-x	Specify the input format, JSON, YAML or XML plist\n"
					"\t-J|-Y|-X Specify the output format, JSON, YAML or XML plist\n"
					"\t-p true|false Set pretty printing (default true)\n"
					"Leaving out the input or putput file or specifying it as -\n"
					"will use stdin or stdout instead.\n");
			return 0;
		}
	}

	FILE *in = stdin;
	const char *in_file = 0;
	if (optind < argc) {
		in_file = argv[optind];
		if (strlen(in_file) == 1 && in_file[0] == '-') {
			in_file = 0;
		}
	}
	if (in_file) {
		in = fopen(in_file, "r");
		if (!in) {
			fprintf(stderr, "Unable to open %s\n", in_file);
			return 1;
		}
	}
	FILE *out = stdout;
	const char *out_file = 0;
	if (optind + 1 < argc) {
		out_file = argv[optind + 1];
		if (strlen(out_file) == 1 && out_file[0] == '-') {
			out_file = 0;
		}
	}
	if (out_file) {
		out = fopen(in_file, "w");
		if (!out) {
			fprintf(stderr, "Unable to open %s\n", out_file);
			return 1;
		}
	}

	Variant v;
	v = DeserializeFile(in, input);

	Serialize(out, v, output, params);

	return 0;
}

