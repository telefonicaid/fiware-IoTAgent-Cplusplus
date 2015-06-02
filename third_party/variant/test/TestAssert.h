/** \file
 * \author John Bridgman
 * \brief 
 */

#ifndef TEST_ASSERT_H
#define TEST_ASSERT_H
#include <sstream>
#include <stdexcept>

inline void test_assert_throw(const char *file, unsigned line, const char *expression) {
	std::ostringstream oss;
	oss << "Assert failed in " << file << " line " << line
		<< "\nExpression: " << expression;
	throw std::runtime_error(oss.str());
}

#define ASSERT(x) if (!(x)) { test_assert_throw(__FILE__, __LINE__, #x); } else (void)0

#endif
