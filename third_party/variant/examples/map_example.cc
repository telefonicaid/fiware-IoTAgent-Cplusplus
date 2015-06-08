/** \file
 * \author John Bridgman
 * \brief 
 */
#include <Variant/Variant.h>
#include <assert.h>
#include <iostream>
using libvariant::Variant;

int main(int argc, char **argv) {
	Variant map;
	// The type of a Variant with the default constructor is the NullType
	assert(map.IsNull());
	// Treating a NullType as some type converts it to that type
	map["key"] = "value";
	map["doo"] = 1.2345;
	map["bar"] = 123;
	map["foo"] = true;
	Variant map2;
	map2["ubar"] = 12u;

	map.Merge(map2);
	// Set all the key value pairs in map2 into map.


	if (map.Contains("doo")) {
		// the key "doo" is in the map
	}

	Variant doodaa = map.Get("doodaa", true);
	// Get checks if doodaa is in the map and if not it returns the second
	// parameter.
	assert(doodaa.IsTrue());

	Variant val = map["blah"];
	// "blah" was not in the map so val is of NullType
	// But "blah" has been added to the map as NullType because Variant can't
	// tell if your assigning or referencing with the [] operator
	assert(val.IsNull());
	// But then we can erase it
	map.Erase("blah");

	for (Variant::MapIterator i(map.MapBegin()), e(map.MapEnd()); i!=e; ++i) {
		std::cout << i->first << ": " << i->second.AsString() << std::endl;
	}
	return 0;
}
