/** \file
 * \author John Bridgman
 * \brief 
 */
#include "TestAssert.h"
#include <Variant/Variant.h>
#include <iostream>

using namespace libvariant;
using namespace std;

const char *test_xml =
"<plist>\n"
"              \n"
"<dict>\n\n"
"<key>   some value with spaces </key>\n"
"\n\n\n"
"<string>\n"
"\tSome formated\n"
"\tstring with strange stuff in it...\n"
"</string>\n"
"</dict>\n"
"</plist>\n";

int main(int argc, char **argv) {
	Variant v = DeserializeXMLPLIST(test_xml);
	SerializeJSON(cout.rdbuf(), v, true);
	cout << endl;
	Variant v2 = DeserializeXMLPLIST(SerializeXMLPLIST(v, true));
	ASSERT(v2 == v);
	return 0;
}
