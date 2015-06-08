/** \file
 * \author John Bridgman
 * \brief 
 */
#include <iostream>
#include <Variant/Variant.h>
#include <string.h>

using namespace libvariant;

const char example_string[] = "--- { id: [ 1,2 ,3, 4], names: [ foo, bar, gar, mar ], twiddle: true, swizzle: 1.24456e-4,"
" subs: [1, 2, [ 1, 2, [ {a: b}, 2 ]]] }";

const char styles[] = "any\0block\0flow\0";

int main(int argc, char **argv) {
	Variant val = Deserialize(example_string, SERIALIZE_YAML);
	std::cout << "--- Pretty printing JSON to stdout using generic serialize function\n";
	Variant param;
	param["pretty"] = true;
	param["indent"] = 2;
	Serialize(std::cout.rdbuf(), val, SERIALIZE_JSON, param);
	std::cout << std::endl;
	param.Clear();
	//param["width"] = 10;
	for (const char *mstyle = &styles[0];*mstyle; mstyle += strlen(mstyle) + 1) {
		for (const char *lstyle = &styles[0];*lstyle; lstyle += strlen(lstyle) + 1) {
			param["map_style"] = mstyle;
			param["list_style"] = lstyle;
			std::cout << "--- Pretty print YAML to stdout using generic serilize function\n"
				<< "With params: ";
			Serialize(std::cout.rdbuf(), param, SERIALIZE_JSON);
			std::cout << std::endl;
			Serialize(std::cout.rdbuf(), val, SERIALIZE_YAML, param);
			std::cout << std::endl;
		}
	}

	return 0;
}
