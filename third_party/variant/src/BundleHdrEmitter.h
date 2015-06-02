/** \file
 * \author John Bridgman
 * \brief A simple emitter that will emit a Bundle header
 */
#ifndef VARIANT_BUNDLEHDREMITTER_H
#define VARIANT_BUNDLEHDREMITTER_H
#pragma once
#include <Variant/Emitter.h>
#include <Variant/EmitterOutput.h>
#include <string.h>

namespace libvariant {

	/**
	 * This emitter is designed so that its output gives predictable
	 * results for the original bundle parser. This means that the parser
	 * in this library will not produce identical results. I.e.,
	 * boolian values will be converted to 0, 1.
	 *
	 * Parameters:
	 *  "precision": number of digits for a float
	 */ 
	class BundleHdrEmitterImpl : public EmitterImpl {
		enum BHEState_t {
			BHE_START,
			BHE_BEGIN,
			BHE_KEY,
			BHE_VALUE,
			BHE_LIST_START,
			BHE_LIST,
			BHE_END,
			BHE_STOP
		};
	public:
		BundleHdrEmitterImpl(shared_ptr<EmitterOutput> o, Variant params);

		virtual ~BundleHdrEmitterImpl();

		virtual void BeginDocument();
		virtual void EndDocument();
		virtual void BeginMap(int length);
		virtual void EndMap();
		virtual void BeginList(int length);
		virtual void EndList();

		virtual void EmitNull();
		virtual void EmitTrue();
		virtual void EmitFalse();
		virtual void Emit(const char *v);
		virtual void Emit(intmax_t v);
		virtual void Emit(uintmax_t v);
		virtual void Emit(double v);
		virtual void Emit(ConstBlobPtr b);

		virtual void Flush();
		virtual void Close();

		virtual Variant GetParam(const std::string &key) {
			if (key == "precision") { return numeric_precision; }
			return Variant::NullType;
		}
		virtual Variant GetParams() { return Variant().Set("precision", numeric_precision); }
		virtual void SetParam(const std::string &key, Variant value) {
			if (key == "precision") { numeric_precision = value.AsUnsigned(); }
		}
	private:

		void Value(const char *v, unsigned len) {
			switch (state) {
			case BHE_KEY:
				Write(v, len);
				Write(": ");
				state = BHE_VALUE;
				break;
			case BHE_VALUE:
				Write(v, len);
				Write("\n");
				state = BHE_KEY;
				break;
			case BHE_LIST_START:
				Write(v, len);
				state = BHE_LIST;
				break;
			case BHE_LIST:
				Write("|");
				Write(v, len);
				break;
			default:
				throw std::runtime_error("BundleHdrEmitter in invalid state to emit a scalar value.");
			}
		}

		void Write(const char *str) {
			Write(str, strlen(str));
		}

		void Write(const char *str, unsigned len) {
			unsigned num_written = 0;
			while (num_written < len) {
				num_written += output->Write(str + num_written, len - num_written);
			}
		}

		shared_ptr<EmitterOutput> output;
		BHEState_t state;
		unsigned numeric_precision;
	};
}
#endif
