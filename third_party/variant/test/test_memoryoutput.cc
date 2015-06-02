/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Variant.h>
#include <stdio.h>
using namespace libvariant;

int main(int argc, char **argv) {
	Variant v;
	v["a key"] = "some value";
	v["another key"] = 1234u;
	std::vector<char> buffer(8192);
	printf("JSON\n");
	unsigned len = SerializeJSON(&buffer[0], buffer.size(), v);
	fwrite(&buffer[0], len, 1, stdout);
#ifdef ENABLE_YAML
	printf("\nYAML\n");
	len = SerializeYAML(&buffer[0], buffer.size(), v);
	fwrite(&buffer[0], len, 1, stdout);
#endif
#ifdef ENABLE_XML
	printf("\nPLIST\n");
	len = SerializeXMLPLIST(&buffer[0], buffer.size(), v);
	fwrite(&buffer[0], len, 1, stdout);
#endif
	return 0;
}
