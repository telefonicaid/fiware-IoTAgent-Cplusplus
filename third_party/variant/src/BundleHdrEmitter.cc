/** \file
 * \author John Bridgman
 * \brief 
 */
#include "BundleHdrEmitter.h"
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <limits>

#define BUNDLE_VERSION_KEY	"bundle.version"
#define BUNDLE_VERSION		"0.0"

namespace libvariant {

	BundleHdrEmitterImpl::BundleHdrEmitterImpl(shared_ptr<EmitterOutput> o, Variant params)
		: output(o),
		state(BHE_START),
		numeric_precision(std::numeric_limits<double>::digits10+2)
   	{
	}


	BundleHdrEmitterImpl::~BundleHdrEmitterImpl() {
		Close();
	}


	void BundleHdrEmitterImpl::BeginDocument() {
		if (state != BHE_START) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for BeginDocument");
		}
		Write(BUNDLE_VERSION_KEY ": " BUNDLE_VERSION "\n");
		state = BHE_BEGIN;
	}

	void BundleHdrEmitterImpl::EndDocument() {
		if (state == BHE_STOP) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for EndDocument");
		}
		state = BHE_STOP;
		Flush();
	}

	void BundleHdrEmitterImpl::BeginMap(int) {
		if (state == BHE_START) {
			BeginDocument();
		}
		if (state != BHE_BEGIN) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for BeginDocument");
		}
		state = BHE_KEY;
	}

	void BundleHdrEmitterImpl::EndMap() {
		if (state != BHE_KEY) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for EndMap");
		}
		state = BHE_END;
	}

	void BundleHdrEmitterImpl::BeginList(int) {
		if (state != BHE_VALUE) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for BeginList");
		}
		state = BHE_LIST_START;
	}

	void BundleHdrEmitterImpl::EndList() {
		if (state != BHE_LIST && state != BHE_LIST_START) {
			throw std::runtime_error("BundleHdrEmitter in invalid state for EndList");
		}
		Write("|\n");
		state = BHE_KEY;
	}


	void BundleHdrEmitterImpl::EmitNull() {
		Value("", 0);
	}

	void BundleHdrEmitterImpl::EmitTrue() {
		Value("1", 1);
	}

	void BundleHdrEmitterImpl::EmitFalse() {
		Value("0", 1);
	}

	void BundleHdrEmitterImpl::Emit(const char *v) {
		Value(v, strlen(v));
	}

	void BundleHdrEmitterImpl::Emit(intmax_t v) {
		std::ostringstream oss;
		oss << v;
		Value(oss.str().c_str(), oss.str().length());
	}

	void BundleHdrEmitterImpl::Emit(uintmax_t v) {
		std::ostringstream oss;
		oss << v;
		Value(oss.str().c_str(), oss.str().length());
	}

	void BundleHdrEmitterImpl::Emit(double v) {
		std::ostringstream oss;
		oss << std::setprecision(numeric_precision) << std::showpoint << v;
		Value(oss.str().c_str(), oss.str().length());
	}

	void BundleHdrEmitterImpl::Emit(ConstBlobPtr b) {
		throw std::runtime_error("BundleHdr is not able to serialize the blob type");
	}

	void BundleHdrEmitterImpl::Flush() { output->Flush(); }

	void BundleHdrEmitterImpl::Close() {
		if (state != BHE_STOP) {
			EndDocument();
		}
		Flush();
	}

}
