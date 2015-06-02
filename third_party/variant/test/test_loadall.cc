/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Variant.h>
#include <stdio.h>

using namespace libvariant;

const char test_string[] = "---\n"
"key1: value1\n"
"sublist: [ 1,2 ,3, 4]\n"
"...\n"
"---\n"
"second document\n"
"...\n"
"---\n"
"- a\n- b\n- c\n"
"...\n"
"--- last document\n";

int main(int argc, char **argv) {

	LoadAllIterator iter = DeserializeYAMLAll(test_string);
	LoadAllIterator end;
	while (iter != end) {
		Variant v = *iter;
		SerializeYAML(stdout, v);
		++iter;
	}
	return 0;
}
