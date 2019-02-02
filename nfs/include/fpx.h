#include "generic.h"

namespace nfs {

	//Floating Point fiXed
	template<typename T, u32 fract, bool useSign = true, typename flp = f32> 
	struct fpx {

		T val;

		constexpr fpx() : val(0) {}
		constexpr fpx(flp val) : val(enc(val)) { }
		constexpr fpx(T val) : val(val) { }

		constexpr operator flp() const {
			return dec(val);
		}

		fpx &operator*=(const fpx &other) {
			return *this = operator flp() * other.operator flp();
		}

		fpx &operator/=(const fpx &other) {
			return *this = operator flp() / other.operator flp();
		}

		fpx &operator+=(const fpx &other) {
			return *this = operator flp() + other.operator flp();
		}

		fpx &operator-=(const fpx &other) {
			return *this = operator flp() - other.operator flp();
		}

		constexpr bool operator>(const fpx &other) const {
			return operator flp() > other.operator flp();
		}

		constexpr bool operator>=(const fpx &other) const {
			return operator flp() >= other.operator flp();
		}

		constexpr bool operator<(const fpx &other) const {
			return operator flp() < other.operator flp();
		}

		constexpr bool operator<=(const fpx &other) const {
			return operator flp() <= other.operator flp();
		}

		constexpr bool operator!=(const fpx &other) const {
			return !operator==(other);
		}

		constexpr bool operator==(const fpx &other) const {
			return val == other.val;
		}

		static constexpr u32 bytes = u32(sizeof(T));
		static constexpr u32 bits = bytes * 8;
		static constexpr u32 sign = useSign;
		static constexpr u32 fraction = fract;
		static constexpr u32 integer = bits - useSign - fraction;

		static constexpr u32 maxInteger = u32(1 << integer) - 1;

		static constexpr u32 signMask = !useSign ? 0 : u32(1 << (bits - 1));
		static constexpr u32 fractMask = u32(1 << fract) - 1;
		static constexpr u32 intMask = T(u32(-1) & ~signMask & ~fractMask);

		static constexpr T enc(flp val) {

			T res = 0;

			if (val < 0) {
				res |= signMask;
				val = -val;
			}

			flp nt = u32(val);
			flp frc = val - nt;

			res |= (T(nt) >= maxInteger ? maxInteger : T(nt)) << fract;
			res |= T(frc * (fractMask + 1));

			return res;
		}

		static constexpr flp dec(T val) {
			flp fval = flp((val & intMask) >> fract) + flp(val & fractMask) / (fractMask + 1);
			return val & signMask ? -fval : fval;
		}

	};

	typedef fpx<u16, 12> fpx1_3_12;		//signed fixed floating point (12 fraction)
	typedef fpx<u16, 6> fpx1_3_6;		//signed fixed floating point (6 fraction)
	typedef fpx<u16, 9> fpx1_0_9;		//signed fixed floating point (9 fraction)
	typedef fpx<u16, 4> fpx1_11_4;		//signed fixed floating point (4 fraction)
	typedef fpx<u32, 12> fpx1_19_12;	//signed fixed floating point (12 fraction)

}