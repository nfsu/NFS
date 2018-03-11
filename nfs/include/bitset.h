#pragma once

#include "generic.h"

namespace oi {

	class boolRef {

	public:

		boolRef(u32 *ptr, u32 off);
		operator bool() const;

		boolRef &operator=(bool b);
		bool getValue() const;

	private:

		u32 *loc;
		u32 offset;

	};

	class Bitset {

	protected:


		template<typename b, typename ...args>
		struct BitsetArgs {
			static constexpr u32 get = 1 + BitsetArgs<args...>::get;
		};

		template<typename b>
		struct BitsetArgs<b> {
			static constexpr u32 get = 1;
		};

	public:

		~Bitset();
		Bitset();
		Bitset(u32 size);
		Bitset(u32 size, bool defVal);
		Bitset(const Bitset &other);

		template<typename ...args>
		Bitset(bool b, args... arg): Bitset(BitsetArgs<bool, args...>::get) {
			BitsetSet<0>::run(*this, b, arg...);
		}

		Bitset(Buffer buf, u32 bits);

		operator std::string();
		std::string toString();

		Bitset &operator=(const Bitset &other);
		bool operator==(const Bitset &other) const;

		boolRef operator[](u32 at);
		bool getValue(u32 at) const;

		u32 getSize() const;

		operator Buffer();
		Buffer asBuffer();

		//Returns a Bitset at the location; usage:
		//Bitset b(8);
		//Bitset firstNibble = b(0, 4);
		//Bitset secondNibble = b(4, 8);
		Bitset subset(u32 at, u32 length) const;

		//Copy the contents from 'other' bitset from start to start + length
		//into this buffer at offset to offset + length
		//Returns false if it was out of range
		//Example:
		//Bitset b(4, true);
		//Bitset b2(8);
		//b2(b, 4, 4);
		bool operator()(const Bitset &other, u32 start, u32 length, u32 offset);

		//Copies a bitset; see operator(const Bitset &, u32, u32, u32)
		bool boolcpy(const Bitset &other, u32 start, u32 length, u32 offset);

		//Copies a bitset; see operator()(const Bitset&, u32, u32, u32)
		bool operator()(const Bitset &other, u32 offset);

		//Copies a bitset; see operator()(const Bitset&, u32, u32, u32)
		bool boolcpy(const Bitset &other, u32 offset);
		
	protected:

		Bitset &copy(const Bitset &other);

		template<u32 offset>
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

		u32 allocated, stored;
		u32 *bits;

	};

}