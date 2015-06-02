/** \file
 * \author John Bridgman
 * \brief 
 */
#include "TestCommon.h"

using namespace libvariant;

int main(int argc, char **argv) {
	return DoRandomTest(argc, argv, YAMLEmitter, YAMLParser, true);
}
