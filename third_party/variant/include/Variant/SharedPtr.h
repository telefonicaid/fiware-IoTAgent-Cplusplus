/** \file
 * \author John Bridgman
 * \brief 
 */
#ifndef VARIANT_SHAREDPTR_H
#define VARIANT_SHAREDPTR_H
#pragma once
#include <Variant/Config.h>

#if defined(__GXX_EXPERIMENTAL_CXX0X) || __cplusplus >= 201103L || defined(HAVE_CXX11_SHAREDPTR)
// The C++11 implementation is available
#include <memory>
namespace libvariant {
	using std::shared_ptr;
	using std::dynamic_pointer_cast;
}

#elif defined(HAVE_TR1_SHAREDPTR)
// Use the tr1 implementation
#include <tr1/memory>
namespace libvariant {
	using std::tr1::shared_ptr;
	using std::tr1::dynamic_pointer_cast;
}

#elif defined(USE_BOOST_SHAREDPTR)
// Use the boost implementation
#include <boost/shared_ptr.hpp>
namespace libvariant {
	using boost::shared_ptr;
	using boost::dynamic_pointer_cast;
}

#else

#error libvariant requires a shared_ptr implementation

#endif

#endif
