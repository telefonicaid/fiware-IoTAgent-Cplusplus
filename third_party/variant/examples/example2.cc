/** \file
 * \author John Bridgman
 * \brief 
 */

#include <Variant/Variant.h>
#include <stdio.h>

using libvariant::Variant;

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("Usage: %s input_file\n", *argv);
		return 1;
	}
	const char *file_name = argv[1];
	Variant v = libvariant::DeserializeYAMLFile(file_name);

	// Now lets examine the variant
	if (v.IsMap()) {
		printf("The variant is a map of length %u with keys:\n", v.Size());
		bool contains_complex_type = false;
		// Maps are internally just std::map<std::string, Variant>
		for (Variant::MapIterator i(v.MapBegin()), e(v.MapEnd()); i!=e; ++i) {
			printf("\"%s\" ", i->first.c_str());
			switch (i->second.GetType()) {
			case Variant::MapType:
			case Variant::ListType:
				contains_complex_type = true;
				break;
			default:
				break;
			}
		}
		printf("\n");
		if (contains_complex_type) {
			printf("The map contains non scalar types.\n");
		}
	} else if (v.IsList()) {
		printf("The variant is a list of length %u\n", v.Size());
		// Lists are also internally std::vector<Variant>
		// Can use v.ListBegin and v.ListEnd for iterators
		// and Variant::ListIterator for the iterator type
	} else {
		printf("The read in variant is neither a list nor a map.\nIts string value is: %s\n", v.AsString().c_str());
	}

	
	return 0;
}
