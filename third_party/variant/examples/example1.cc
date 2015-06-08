/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Variant.h>
#include <stdio.h>

// Import Variant from libvariant
using libvariant::Variant;

// Example strings to use
const char *example_json_string =
"{\"key1\": \"value1\",\"key2\":1.23,\"list\":[1,2,3,4]}";

const char *example_yaml_string =
"key1: value1\n"
"key2: 1.23\n"
"list: [ 1, 2, 3, 4]\n"
;

int main(int argc, char **argv) {
	// Load the example_json_string onto ex1
	Variant ex1 = libvariant::DeserializeJSON(example_json_string);
	// Load the example yaml string
	Variant ex2 = libvariant::DeserializeYAML(example_yaml_string);
	// They are the same
	if (ex1 != ex2) {
		printf("Differ!\n");
		return 1;
	}

	// Pretty print to stdout
	libvariant::SerializeJSON(stdout, ex2, true);

	return 0;
}
