#pragma once
#include "generic.h"
#include "sstructmember.h"

namespace nfs {

	//A variable size ubyte; up to 7 bits
	template<u32 n>
	struct ux : sstructmember {

		static constexpr u8 max = (1 << n) - 1;
		static constexpr u8 left = 8 - n;

		u8 *byte, off;

		constexpr ux() : byte(nullptr), off(0) {}
		constexpr ux(u8 v) : byte(nullptr), off(v & max) {}
		constexpr ux(u8 *byte, u8 off) : byte(byte), off(off) {}

		constexpr operator u8() const {

			if (byte == nullptr) return off;

			u8 right = (off + n) % 8;
			if (right != 0 && right < left)
				return ((*byte & (max >> right)) << right) |
				((byte[1] & u8(max << (8 - right))) >> (8 - right));	//Misaligned; 2 byte fetches
			else
				return (*byte & (max << (left - off))) >> (left - off);		//Aligned; 1 byte fetch

		}

		ux &operator&=(const ux &other) {
			u8 res = *this & other;
			*this = res;
			return *this;
		}

		ux &operator|=(const ux &other) {
			u8 res = *this | other;
			*this = res;
			return *this;
		}

		ux &operator^=(const ux &other) {
			u8 res = *this ^ other;
			*this = res;
			return *this;
		}

		ux &operator+=(const ux &other) {
			u8 res = *this + other;
			*this = res;
			return *this;
		}

		ux &operator-=(const ux &other) {
			u8 res = *this - other;
			*this = res;
			return *this;
		}

		ux &operator*=(const ux &other) {
			u8 res = *this * other;
			*this = res;
			return *this;
		}

		ux &operator/=(const ux &other) {
			u8 res = *this / other;
			*this = res;
			return *this;
		}

		ux &operator%=(const ux &other) {
			u8 res = *this % other;
			*this = res;
			return *this;
		}

		ux operator<<=(u32 x) const {
			return value() << x;
		}

		ux operator>>=(u32 x) const {
			return value() >> x;
		}

		bool operator==(const ux &other) const {
			return value() == other.value();
		}

		bool operator!=(const ux &other) const {
			return value() != other.value();
		}

		bool operator>=(const ux &other) const {
			return value() >= other.value();
		}

		bool operator<=(const ux &other) const {
			return value() <= other.value();
		}

		bool operator>(const ux &other) const {
			return value() > other.value();
		}

		bool operator<(const ux &other) const {
			return value() < other.value();
		}

		ux operator~() const {
			return ~value();
		}

		ux &operator=(u8 v) {

			v &= max;

			u8 right = (off + n) % 8;

			if (byte == nullptr)
				off = v;
			else if (right != 0 && right < left) {
				*byte = (*byte & u8(~(max >> right))) | (v >> right);
				byte[1] = (*byte & u8(~u8(max << (8 - right)))) | (v << (8 - right));	//Misaligned; 2 byte stores
			} else
				*byte = *byte & u8(~(max << (left - off))) | (v << (left - off));				//Aligned; 1 byte store

			return *this;
		}

		//Required fields:
		static constexpr u32 bits = n;
		u8 value() const { return operator u8(); }

	};

	typedef ux<4> nibble;
	typedef ux<3> octal;

	typedef ux<7> u7;
	typedef ux<6> u6;
	typedef ux<5> u5;
	typedef ux<4> u4;
	typedef ux<3> u3;
	typedef ux<2> u2;

}