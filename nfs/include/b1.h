#pragma once
#include "generic.h"
#include "sstructmember.h"

namespace nfs {

	//A reference to a bit
	struct b1 : sstructmember {

		u8 *byte, off;

		constexpr b1() : byte(nullptr), off(0) {}
		constexpr b1(bool b) : byte(nullptr), off(b) {}
		constexpr b1(u8 *byte, u8 off) : byte(byte), off(off) {}

		constexpr u8 offset() const { return u8(1 << (7 - off)); }		//Offset bit
		constexpr u8 nonOffset() const { return u8(~offset()); }		//Non offset bits

		constexpr operator bool() const {
			if (byte == nullptr) return (bool)off;
			return *byte & offset();
		}

		b1 &operator&=(const b1 &other) {
			bool res = *this && other;
			*this = res;
			return *this;
		}

		b1 &operator|=(const b1 &other) {
			bool res = *this || other;
			*this = res;
			return *this;
		}

		b1 &operator^=(const b1 &other) {
			bool res = *this != other;
			*this = res;
			return *this;
		}

		b1 &operator=(bool b) {
			if (byte == nullptr) off = b;
			else if (!b) *byte &= nonOffset();
			else *byte |= offset();
			return *this;
		}

		bool operator==(const b1 &other) const {
			return value() == other.value();
		}

		bool operator!=(const b1 &other) const {
			return value() != other.value();
		}

		//Required fields
		static constexpr u32 bits = 1;
		bool value() const { return operator bool(); }

	};

	typedef bool b8;

}