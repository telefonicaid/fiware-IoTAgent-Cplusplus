/** \file
 * \author John Bridgman
 * \brief 
 */
#include "Base64.h"

namespace libvariant {

	unsigned Base64Encode(char *outptr, const void *inptr, unsigned len, unsigned cpl) {
		struct iovec iov = { (void*)inptr, len };
		return Base64Encode(outptr, &iov, 1, cpl);
	}

	class iov_iterator {
	public:
		iov_iterator() : iov(0), len(0), slab(0), offset(0) {}
		iov_iterator(const iov_iterator &o) : iov(o.iov), len(o.len), slab(o.slab), offset(o.offset) {}
		iov_iterator(const struct iovec *v, unsigned l) : iov(v), len(l), slab(0), offset(0) {}
		iov_iterator &operator++() {
			++offset;
			if (offset >= iov[slab].iov_len) {
				++slab;
				offset = 0;
			}
			return *this;
		}
		iov_iterator operator++(int) {
			iov_iterator o = *this;
			++(*this);
			return o;
		}

		char operator *() {
			return *((char*)(iov[slab].iov_base) + offset);
		}

		bool operator==(const iov_iterator &o) {
			return (slab == o.slab && offset == o.offset);
		}

		bool is_end() { return slab >= len; }
		iov_iterator end() {
			iov_iterator o = *this;
			while (!o.is_end()) { ++o; }
			return o;
		}

	private:
		const struct iovec *iov;
		unsigned len;
		unsigned slab;
		unsigned offset;
	};

	unsigned Base64Encode(char *outptr, const struct iovec *iov, unsigned iov_len, unsigned cpl) {
		static const char encoding[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		iov_iterator i = iov_iterator(iov, iov_len);
		iov_iterator e = i.end();
		unsigned step_count = 0;
		char *o = outptr;
#define PUSH_VALUE()\
		do {\
			if (result > 63) { *o++ = '='; }\
			else { *o++ = encoding[(int)result]; }\
		} while (0)

		char result;
		char fragment;
		while (true) {
			// Step A
			if (i == e) { break; }
			fragment = *i++;
			result = (fragment & 0x0FC) >> 2;
			PUSH_VALUE();
			result = (fragment & 0x003) << 4;
			// Step B
			if (i == e) {
				PUSH_VALUE();
				*o++ = '=';
				*o++ = '=';
				break;
			}
			fragment = *i++;
			result |= (fragment & 0x0f0) >> 4;
			PUSH_VALUE();
			result = (fragment & 0x00f) << 2;
			// Step C
			if (i == e) {
				PUSH_VALUE();
				*o++ = '=';
				break;
			}
			fragment = *i++;
			result |= (fragment & 0x0c0) >> 6;
			PUSH_VALUE();
			result	= (fragment & 0x03f) >> 0;
			PUSH_VALUE();
			++step_count;
			if (cpl > 0 && step_count == cpl/4) {
				*o++ = '\n';
				step_count = 0;
			}
		}
#undef PUSH_VALUE
		if (cpl > 0) { *o++ = '\n'; }
		*o = '\0';
		return o - outptr;
	}

	unsigned Base64EncodeSize(unsigned len, unsigned cpl) {
		// Every 3 bytes turns into 4 bytes
		unsigned res = (len*4)/3+4;
		// There are 2 extra bytes every CPL output bytes plus 2 extras at the end
		if (cpl > 0) { res += 2*(res/cpl + 1); }
		else { res+= 1; }
		return res;
	}

	unsigned Base64Decode(void *outptr, const char *inptr, unsigned len) {
	static const signed char decoding[] = {62,-1,-1,-1,63,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-2,-1,-1,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,-1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51};
    static const signed char decoding_size = sizeof(decoding);
	const signed char *i = (const signed char*)inptr;
	const signed char *e = i + len;
	signed char *o = (signed char*)outptr;
	signed char fragment;
	signed char result;
#define DECODE_VALUE()\
		do {\
			if (i == e) { return o - (signed char*)outptr; }\
			signed char val = *i++ - 43;\
			if (val < 0 || val > decoding_size) { fragment = -1; }\
			else { fragment = decoding[(int)val]; }\
		} while (fragment < 0)

		while (true) {
			// Step A
			DECODE_VALUE();
            result = (fragment & 0x03f) << 2;
			// Step B
			DECODE_VALUE();
            result |= (fragment & 0x030) >> 4;
			*o++ = result;
            result = (fragment & 0x00f) << 4;
			// Step C
			DECODE_VALUE();
            result |= (fragment & 0x03c) >> 2;
			*o++ = result;
            result = (fragment & 0x003) << 6;
			// Step D
			DECODE_VALUE();
            result |= (fragment & 0x03f);
			*o++ = result;
		}
#undef DECODE_VALUE
	}

	unsigned Base64DecodeSize(unsigned len) {
		return (len*3)/4 + 1;
	}
}
