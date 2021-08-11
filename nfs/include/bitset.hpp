#pragma once
#include "generic.hpp"

namespace oi {

	class boolRef {

	public:

		boolRef(u32 *ptr, usz off);
		operator bool() const;

		boolRef &operator=(bool b);
		bool getValue() const;

	private:

		u32 *loc;
		usz offset;
	};

	class Bitset {

	public:

		~Bitset();
		Bitset();
		Bitset(usz size);
		Bitset(usz size, bool defVal);
		Bitset(const Bitset &other);

		template<typename ...args>
		Bitset(bool b, args... arg): Bitset(sizeof...(arg) + 1) {
			BitsetSet<0>::run(*this, b, arg...);
		}

		Bitset(Buffer buf, usz bits);

		operator String();
		String toString();

		Bitset &operator=(const Bitset &other);
		bool operator==(const Bitset &other) const;

		boolRef operator[](usz at);
		bool getValue(usz at) const;

		usz getSize() const;

		operator Buffer();
		Buffer asBuffer();

		//Returns a Bitset at the location; usage:
		//Bitset b(8);
		//Bitset firstNibble = b(0, 4);
		//Bitset secondNibble = b(4, 8);
		Bitset subset(usz at, usz length) const;

		//Copy the contents from 'other' bitset from start to start + length
		//into this buffer at offset to offset + length
		//Returns false if it was out of range
		//Example:
		//Bitset b(4, true);
		//Bitset b2(8);
		//b2(b, 4, 4);
		bool operator()(const Bitset &other, usz start, usz length, usz offset);

		//Copies a bitset; see operator(const Bitset &, usz, usz, usz)
		bool boolcpy(const Bitset &other, usz start, usz length, usz offset);

		//Copies a bitset; see operator()(const Bitset&, usz, usz, usz)
		bool operator()(const Bitset &other, usz offset);

		//Copies a bitset; see operator()(const Bitset&, usz, usz, usz)
		bool boolcpy(const Bitset &other, usz offset);
		
	protected:

		Bitset &copy(const Bitset &other);

		template<usz offset>
		struct BitsetSet {

			template<typename ...args>
			static void run(Bitset &bs, bool b, args... arg) {
				bs[offset] = b;
				BitsetSet<offset + 1>::run(bs, arg...);
			}

			static void run(Bitset &bs, bool b) {
				bs[offset] = b;
			}

		};

	private:

		usz allocated, stored;
		u32 *bits;
	};

}