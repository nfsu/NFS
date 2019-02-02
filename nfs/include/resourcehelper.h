#pragma once
#include "ntypes.h"

#define NBUO_num 0x4E42554FU

namespace nfs {

	struct ResourceHelper {

		template<u32 i, bool b, typename ...args>
		static u32 getMagicNumber(GenericResource<i, b, args...> res) {
			return i;
		}

		static u32 getMagicNumber(NBUO nbuo) {
			return NBUO_num;
		}

		template<typename T>
		static u32 getMagicNumber() {
			return getMagicNumber(T{});
		}

		template<typename T>
		static bool isType(const u8 *type) {

			const u32 &num = *(u32*)type;
			const u32 numInv = ((num & 0xFFU) << 24U) | ((num & 0xFF00U) << 8U) | ((num & 0xFF0000U) >> 8U) | ((num & 0xFF000000U) >> 16U);

			const u32 tnum = getMagicNumber(T{});

			return tnum == num || tnum == numInv;
		}

		template<typename T>
		static constexpr u32 typeId() {
			return CTLHelper::index<T, 0>(ResourceTypes{});
		}

		static constexpr u32 getMaxResourceSize() {
			return getMaxResourceSize_inter(ResourceTypes{});
		}

		static u32 getType(const u8 *type);
		static size_t getSize(u32 typeId);
		static u32 getMagicNum(u32 typeId);
		static ResourceInfo read(u8 *rom, u32 size, u8 *res);
		static std::string getName(u8 *loc, u32 magicNumber);

	private:

		///Get type from address

		template<u32 i, typename T, typename ...args>
		struct Type_inter {

			static u32 get(const u8 *type) {
				return isType<T>(type) ? i : Type_inter<i + 1, args...>::get(type);
			}

		};

		template<u32 i, typename T>
		struct Type_inter<i, T> {

			static u32 get(const u8 *type) {
				return i;
			}

		};

		template<typename ...args>
		static u32 getType_inter(CompileTimeList<args...> res, const u8 *type) {
			return Type_inter<0, args...>::get(type);
		}

		///Get size from type

		template<u32 i, typename T, typename ...args>
		struct Size_inter {

			static size_t get(u32 type) {
				return type == i ? sizeof(T) : Size_inter<i + 1, args...>::get(type);
			}

		};

		template<u32 i, typename T>
		struct Size_inter<i, T> {

			static size_t get(u32 type) {
				return sizeof(T);
			}

		};

		template<typename ...args>
		static size_t getSize_inter(CompileTimeList<args...> res, u32 type) {
			return Size_inter<0, args...>::get(type);
		}

		///Get magic number from type

		template<u32 i, typename T, typename ...args>
		struct MagicNumber_inter {

			static u32 get(u32 type) {
				return type == i ? ResourceHelper::getMagicNumber(T{}) : MagicNumber_inter<i + 1, args...>::get(type);
			}

		};

		template<u32 i, typename T>
		struct MagicNumber_inter<i, T> {

			static u32 get(u32 type) {
				return NBUO_num;
			}

		};

		template<typename ...args>
		static u32 getMagicNumber_inter(CompileTimeList<args...> res, u32 type) {
			return MagicNumber_inter<0, args...>::get(type);
		}

		///Read type

		template<typename T>
		struct Fill {

			static void run(u8 *ptr, u8 *origin, u32 size) {
				run(*(T*)ptr, origin, size);
			}

			template<u32 i, bool b, typename ...args>
			static void run(GenericResource<i, b, args...> &gr, u8 *origin, u32 size) {
				GenericHeader *head = (GenericHeader*)origin;
				gr.header = head;
				run_inter(gr, origin + sizeof(GenericHeader) + (b ? head->sections * 4 : 0), size);
			}

			template<u32 i, bool b, typename ...args>
			static void run_inter(GenericResource<i, b, args...> &gr, u8 *origin, u32 size) {
				FillContents<0, args...>::run(gr, origin, size, gr.header->sections);
			}

			template<u32 i, typename T2, typename ...args>
			struct FillContents {

				template<u32 j, bool b, typename ...args2>
				static void run(GenericResource<j, b, args2...> &gr, u8 *origin, u32 size, u32 sections) {
					gr.ptrs[i] = i < sections ? origin : nullptr;
					FillContents<i + 1, args...>::run(gr, origin + ((T2*)gr.ptrs[i])->size, size, sections);
				}

			};

			template<u32 i, typename T2>
			struct FillContents<i, T2> {

				template<u32 j, bool b, typename ...args2>
				static void run(GenericResource<j, b, args2...> &gr, u8 *origin, u32 size, u32 sections) {
					gr.ptrs[i] = i < sections ? origin : nullptr;
				}

			};

		};

		template<u32 i, typename T, typename ...args>
		struct Fill_inter {

			static void get(u32 type, u8 *ptr, u8 *origin, u32 size) {
				if (type == i) Fill<T>::run(ptr, origin, size);
				else Fill_inter<i + 1, args...>::get(type, ptr, origin, size);
			}

		};

		template<u32 i, typename T>
		struct Fill_inter<i, T> {

			static void get(u32 type, u8 *ptr, u8 *origin, u32 size) {
				NBUO &buf = *(NBUO*)ptr;
				buf.ptr = origin;
				buf.size = size;
			}

		};

		template<typename ...args>
		static void fill_inter(CompileTimeList<args...> res, u32 type, u8 *ptr, u8 *origin, u32 size) {
			Fill_inter<0, args...>::get(type, ptr, origin, size);
		}

		///Get max resource size

		template<typename ...args>
		static constexpr u32 getMaxResourceSize_inter(CompileTimeList<args...> res) {
			return MaxSize<args...>::get;
		}
	};

}