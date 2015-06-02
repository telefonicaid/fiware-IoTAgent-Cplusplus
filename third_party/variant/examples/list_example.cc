/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Variant.h>
#include <iostream>
using libvariant::Variant;

int main(int argc, char **argv) {
	Variant list;
	// Can assign the type to a variant to "force" it to be
	// that type. The type will loose any value it had (i.e., 
	// a list will be empty, maps will be empty, numbers will have
	// an indeterminate value, etc.)
	list = Variant::ListType;
	list[0] = true;
	list[2] = "abc";
	// Note that now the list is [true, null, "abc"]
	// Lists are extended automatically to the length that they are referred to.
	list.Append(123);
	// Appending to the list

	for (unsigned i = 0; i < list.Size(); ++i) {
		std::cout << list[i].AsString() << std::endl;
	}
	return 0;
}

