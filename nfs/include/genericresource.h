#pragma once
#include "templatemagic.h"

namespace nfs {

	struct GenericHeader {
		u32 magicNumber;					//MagicNumber; NCLR, NCGR, NSCR, etc.
		u32 c_constant;
		u32 size;							//Size of the header; including contents
		u16 c_headerSize;
		u16 sections;						//Number of sections
	};

	struct GenericSection {
		u32 magicNumber;					//MagicNumber; TTLP, PMCP, etc.
		u32 size;							//Size of section; including contents
	};

	template<u32 magicNumber, typename ...args>
	struct GenericResource {

		GenericHeader *header = nullptr;
		u8 *ptrs[CountArgs<args...>::get()] = {};

		//Get section struct
		template<u32 i>
		auto &at() {

			static_assert(i < CountArgs<args...>::get(), "GenericResource::get<i> out of bounds");

			return ReadObject<0, i, args...>::at(ptrs[i]);
		}

		//Get data of section
		template<u32 i>
		Buffer get() {

			static_assert(i < CountArgs<args...>::get(), "GenericResource::get<i> out of bounds");

			return GetBuffer<0, i, args...>::at(ptrs[i]);
		}

		//Get identifier
		static constexpr u32 getMagicNumber() {
			return magicNumber;
		}

	};

	struct ResourceInfo {
		size_t size;
		u32 type;
		u32 magicNumber;
	};
}