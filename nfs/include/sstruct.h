#pragma once
#include "generic.h"

/*
	sstruct doesn't handle padding well when applied wrong
	Always pad sstructmember types to 8-bit manually 
*/

namespace nfs {

	//Sizeof a value in bits

	template<typename T, bool b = std::is_base_of<sstructmember, T>::value>
	struct sstruct_BitSize {
		static constexpr u32 get = u32(sizeof(T) * 8);
	};

	template<typename T>
	struct sstruct_BitSize<T, true> {
		static constexpr u32 get = T::bits;
	};

	//Sizeof a value (in a compile time list) in bits

	template<u32 i, u32 j, typename T, typename ...args>
	struct sstruct_SizeOf {
		static constexpr u32 get = i == j ? sstruct_BitSize<T>::get : sstruct_SizeOf<i + 1, j, args...>::get;
	};

	template<u32 i, u32 j, typename T>
	struct sstruct_SizeOf<i, j, T> {
		static constexpr u32 get = i == j ? sstruct_BitSize<T>::get : 0;
	};

	//Offset of a value in bits

	template<u32 i, u32 j, u32 off, typename T, typename ...args>
	struct sstruct_OffsetOf {
		static constexpr u32 get = i == j ? off : sstruct_OffsetOf<i + 1, j, off + sstruct_BitSize<T>::get, args...>::get;
	};

	template<u32 i, u32 j, u32 off, typename T>
	struct sstruct_OffsetOf<i, j, off, T> {
		static constexpr u32 get = i == j ? off : off + sstruct_BitSize<T>::get;
	};

	//Check if a type matches
	
	template<u32 i, typename Targ, u32 j, typename T, typename ...args>
	struct sstruct_IsSame {
		static constexpr bool get = i == j ? std::is_same<T, Targ>::value : sstruct_IsSame<i + 1, Targ, j, args...>::get;
	};

	template<u32 i, typename Targ, u32 j, typename T>
	struct sstruct_IsSame<i, Targ, j, T> {
		static constexpr bool get = i == j && std::is_same<T, Targ>::value;
	};

	//Get a value from an offset

	template<typename T, bool b = std::is_base_of<sstructmember, T>::value>
	struct sstruct_GetValue {

		static T &get(u8 *buf, u32 offset) {
			return *(T*)(buf + u32(std::ceil(offset / 8.f)));
		}

	};

	template<typename T>
	struct sstruct_GetValue<T, true> {

		static T get(u8 *buf, u32 offset) {
			return T(buf + offset / 8, offset % 8);
		}

	};

	//Get a value in a struct

	template<u32 i, u32 j, bool b, typename T, typename ...args>
	struct sstruct_Get {

		static auto get(u8 *buf, u32 offset = 0) {
			return sstruct_Get<i + 1, j, i + 1 == j, args...>::get(buf, offset + sstruct_BitSize<T>::get);
		}

	};

	template<u32 i, u32 j, typename T, typename ...args>
	struct sstruct_Get<i, j, true, T, args...> {

		static auto get(u8 *buf, u32 offset = 0) {
			return sstruct_GetValue<T>::get(buf, offset);
		}

	};

	template<u32 i, u32 j, typename T>
	struct sstruct_Get<i, j, true, T> {

		static auto get(u8 *buf, u32 offset = 0) {
			return sstruct_GetValue<T>::get(buf, offset);
		}

	};

	//Compile-time math

	struct CTMath {

		static constexpr f32 floor(f32 x) {
			return f32(i32(x));
		}

		static constexpr f32 ceil(f32 x) {
			f32 floored = CTMath::floor(x);
			if (x == floored) return x;
			return x < 0 ? floored - 1 : floored + 1;
		}

	};

	//Set a value in a sstruct

	template<typename T, bool b = std::is_base_of<sstructmember, T>::value>
	struct sstruct_Set {

		static void run(u8 *buf, u32 offset, T t) {
			*(T*)(buf + u32(std::ceil(offset / 8.f))) = t;
		}

	};

	template<typename T>
	struct sstruct_Set<T, true> {

		static void run(u8 *buf, u32 offset, T t) {
			u8 *ptr = buf + offset / 8;
			T(ptr, offset % 8) = t.value();
		}

	};

	//Fill a sstruct

	template<typename T, typename ...args>
	struct sstruct_Fill {

		static void run(u8 *buf, u32 offset, T t, args... arg) {
			sstruct_Set<T>::run(buf, offset, t);
			sstruct_Fill<args...>::run(buf, offset + sstruct_BitSize<T>::get, arg...);
		}

	};

	template<typename T>
	struct sstruct_Fill<T> {

		static void run(u8 *buf, u32 offset, T t) {
			sstruct_Set<T>::run(buf, offset, t);
		}

	};

	//A small data struct member has a static constexpr u32 bits field
	//This field is used to determine where the value is located in memory in small structs
	//A small struct can have two types of values:
	//sstructmember (a struct with size in bits)
	//struct T (a struct with size in bytes)
	//Example of sstruct:
	//sstruct<bit, bit, bit, bit, nibble> byte;
	template<typename ...args>
	struct sstruct {

		static constexpr u32 argc = sizeof...(args);
		static constexpr u32 bits = sstruct_OffsetOf<0, argc, 0, args...>::get;
		static constexpr u32 bytes = u32(CTMath::ceil(bits / 8.f));

		u8 val[bytes];

		sstruct() { memset(val, 0, bytes); }
		sstruct(args... arg) : sstruct() {
			sstruct_Fill<args...>::run(val, 0, arg...);
		}

		sstruct &operator=(const sstruct &other) {
			memcpy(val, other.val, bytes);
			return *this;
		}

		template<u32 i>
		static constexpr u32 offsetOf = sstruct_OffsetOf<0, i, 0, args...>::get;

		template<u32 i>
		static constexpr u32 sizeOf = sstruct_SizeOf<0, i, args...>::get;

		template<u32 i>
		static constexpr u32 byteOffset = offsetOf<i> / 8;

		template<u32 i>
		static constexpr u32 bitOffset = offsetOf<i> % 8;

		template<u32 i>
		auto get() {
			return sstruct_Get<0, i, 0 == i, args...>::get(val, 0);
		}

	};

}