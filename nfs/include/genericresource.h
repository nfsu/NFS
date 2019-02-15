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

	template<u32 magicNumber, bool sectionOffsets, typename ...args>
	struct GenericResource {

		static constexpr u32 elements() {
			return CountArgs<args...>::get();
		}

		GenericHeader *header = nullptr;
		u8 *ptrs[elements()] = {};

		//Get section struct
		template<u32 i>
		auto &at() {

			static_assert(i < elements(), "GenericResource::get<i> out of bounds");

			return ReadObject<0, i, args...>::at(ptrs[i]);
		}

		template<u32 i>
		bool contains() {

			static_assert(i < elements(), "GenericResource::get<i> out of bounds");

			return ptrs[i] != nullptr;
		}

		//Get data of section
		template<u32 i>
		Buffer get() {

			static_assert(i < elements(), "GenericResource::get<i> out of bounds");

			return GetBuffer<0, i, args...>::at(ptrs[i]);
		}

		Buffer toBuffer() {
			return Buffer::alloc(size(), (u8*) header);
		}

		//Gives the size if all sections are present
		static constexpr u32 maxSize() {
			return FindOffset<elements(), elements(), args...>();
		}

		u32 size() {
			return header->size;
		}

		u32 headerSize() {
			return header->c_headerSize;
		}

		u32 contentSize() {
			return size() - headerSize();
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