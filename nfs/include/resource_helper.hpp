#pragma once
#include "ntypes.hpp"

#define NBUO_num 0x4E42554F

namespace nfs {

	struct ResourceHelper {

		template<usz i, bool b, typename ...args>
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
		static bool isType(const u8 *type, usz len) {

			u32 num;

			switch (len) {

				case 0: 
					num = 0;
					break;

				case 1:
					num = *(const u8*)type;
					break;

				case 2:
					num = *(const u16*)type;
					break;

				case 3: 
					num = (*(const u16*)type) + (u32(*(const u8*)(type + 2)) << 16);
					break;

				default:
					num = *(const u32*) type;
					break;
			}

			const u32 numInv =
				(num << 24) | 
				((num & 0xFF00) << 8) | 
				((num & 0xFF0000) >> 8) | 
				((num & 0xFF000000) >> 24);

			const u32 tnum = getMagicNumber(T{});

			return tnum == num || tnum == numInv;
		}

		template<typename T>
		static constexpr u32 typeId() {
			return CTLHelper::index<T, 0>(ResourceTypes{});
		}

		static constexpr usz getMaxResourceSize() {
			return getMaxResourceSize_inter(ResourceTypes{});
		}

		static u32 getType(const u8 *type, u32 size);
		static usz getSize(u32 typeId);
		static u32 getMagicNum(u32 typeId);
		static ResourceInfo read(u8 *rom, usz size, u8 *res);
		static String getName(u8 *loc, usz len, u32 magicNumber, bool flip = true);

	private:

		//Get type from address

		template<usz i, typename T, typename ...args>
		struct Type_inter {
			static u32 get(const u8 *type, usz size) {
				return isType<T>(type, size) ? i : Type_inter<i + 1, args...>::get(type, size);
			}
		};

		template<usz i, typename T>
		struct Type_inter<i, T> {
			static u32 get(const u8 *type, usz) {
				return i;
			}
		};

		template<typename ...args>
		static u32 getType_inter(CompileTimeList<args...> res, const u8 *type, usz size) {
			return Type_inter<0, args...>::get(type, size);
		}

		//Get size from type

		template<usz i, typename T, typename ...args>
		struct Size_inter {
			static usz get(u32 type) {
				return type == i ? sizeof(T) : Size_inter<i + 1, args...>::get(type);
			}
		};

		template<usz i, typename T>
		struct Size_inter<i, T> {
			static usz get(u32 type) {
				return sizeof(T);
			}
		};

		template<typename ...args>
		static usz getSize_inter(CompileTimeList<args...> res, u32 type) {
			return Size_inter<0, args...>::get(type);
		}

		//Get magic number from type

		template<usz i, typename T, typename ...args>
		struct MagicNumber_inter {
			static u32 get(u32 type) {
				return type == i ? ResourceHelper::getMagicNumber(T{}) : MagicNumber_inter<i + 1, args...>::get(type);
			}
		};

		template<usz i, typename T>
		struct MagicNumber_inter<i, T> {
			static u32 get(u32 type) {
				return NBUO_num;
			}
		};

		template<typename ...args>
		static u32 getMagicNumber_inter(CompileTimeList<args...> res, u32 type) {
			return MagicNumber_inter<0, args...>::get(type);
		}

		//Read type

		template<typename T>
		struct Fill {

			static void run(u8 *ptr, u8 *origin, usz size) {
				run(*(T*)ptr, origin, size);
			}

			template<usz i, bool b, typename ...args>
			static void run(GenericResource<i, b, args...> &gr, u8 *origin, usz size) {
				GenericHeader *head = (GenericHeader*)origin;
				gr.header = head;
				run_inter(gr, origin + sizeof(GenericHeader) + (b ? head->sections * 4 : 0), size);
			}

			template<usz i, bool b, typename ...args>
			static void run_inter(GenericResource<i, b, args...> &gr, u8 *origin, usz size) {
				FillContents<0, args...>::run(gr, origin, size, gr.header->sections);
			}

			template<usz i, typename T2, typename ...args>
			struct FillContents {

				template<usz j, bool b, typename ...args2>
				static void run(GenericResource<j, b, args2...> &gr, u8 *origin, usz size, usz sections) {
					gr.ptrs[i] = i < sections ? origin : nullptr;
					FillContents<i + 1, args...>::run(gr, origin + ((T2*)gr.ptrs[i])->size, size, sections);
				}
			};

			template<usz i, typename T2>
			struct FillContents<i, T2> {

				template<usz j, bool b, typename ...args2>
				static void run(GenericResource<j, b, args2...> &gr, u8 *origin, usz size, usz sections) {
					gr.ptrs[i] = i < sections ? origin : nullptr;
				}
			};
		};

		template<usz i, typename T, typename ...args>
		struct Fill_inter {

			static void get(u32 type, u8 *ptr, u8 *origin, usz size) {
				if (type == i) Fill<T>::run(ptr, origin, size);
				else Fill_inter<i + 1, args...>::get(type, ptr, origin, size);
			}
		};

		template<u32 i, typename T>
		struct Fill_inter<i, T> {

			static void get(u32 type, u8 *ptr, u8 *origin, usz size) {
				*(NBUO*)ptr = { size, origin };
			}
		};

		template<typename ...args>
		static void fill_inter(CompileTimeList<args...> res, u32 type, u8 *ptr, u8 *origin, usz size) {
			Fill_inter<0, args...>::get(type, ptr, origin, size);
		}

		///Get max resource size

		template<typename ...args>
		static constexpr usz getMaxResourceSize_inter(CompileTimeList<args...> res) {
			return MaxSize<args...>::get;
		}
	};

}