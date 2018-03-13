#pragma once
#include "generic.h"

namespace nfs {

	///Count arguments from ...args

	template<typename T, typename ...args>
	struct CountArgs {
		static constexpr u32 get() {
			return 1U + CountArgs<args...>::get();
		}
	};

	template<typename T>
	struct CountArgs<T> {
		static constexpr u32 get() {
			return 1U;
		}
	};

	///Return reference from j in ...args

	template<u32 i, u32 j, typename T, typename ...args>
	struct ReadObject {

		template<bool b = (i == j)>
		static auto &at(u8 *ptr) {
			return ReadObject<i + 1, j, args...>::at(ptr);
		}

		template<>
		static auto &at<true>(u8 *ptr) {
			return *(T*)ptr;
		}
	};

	template<u32 i, u32 j, typename T>
	struct ReadObject<i, j, T> {

		static T &at(u8 *ptr) {
			return *(T*)ptr;
		}
	};


	///Find offset of j in ...args

	template<u32 i, u32 j, typename T, typename ...args>
	struct FindOffset {
		static constexpr u32 get = i < j ? sizeof(T) + FindOffset<i + 1, j, args...>::get : 0;
	};

	template<u32 i, u32 j, typename T>
	struct FindOffset<i, j, T> {
		static constexpr u32 get = i < j ? sizeof(T) : 0;
	};


	///Run placement new everything in  ...args

	template<typename T, typename ...args>
	struct PlacementNew {

		template<typename T2, typename ...args2>
		struct PlacementNew_inter {

			static void run(u8 *ptr, T2 t2, args2 ...arg) {

				::new(ptr) T(t2);
				PlacementNew<args...>::PlacementNew_inter<args2...>::run(ptr + (u32) sizeof(T), arg...);

			}

		};

		template<typename T2>
		struct PlacementNew_inter<T2> {

			static void run(u8 *ptr, T2 t2) {

				::new(ptr) T(t2);
				PlacementNew<args...>::run(ptr + (u32) sizeof(T));

			}

		};

		template<typename ...args2>
		static void run(u8 *ptr, args2 ...arg) {
			return PlacementNew_inter<args2...>::run(ptr, arg...);
		}

		template<>
		static void run<>(u8 *ptr) {
			::new(ptr) T();
			PlacementNew<args...>::run(ptr);
		}
	};

	template<typename T>
	struct PlacementNew<T> {

		template<typename T2>
		struct PlacementNew_inter {

			static void run(u8 *ptr, T2 t2) {

				::new(ptr) T(t2);

			}

		};

		template<typename T2>
		static void run(u8 *ptr, T2 t2) {

			::new(ptr) T(t2);

		}

		static void run(u8 *ptr) {

			::new(ptr) T();

		}
	};

	///Get buffer of j in ...args

	template<u32 i, u32 j, typename T, typename ...args>
	struct GetBuffer {

		template<bool b = (i == j)>
		static Buffer at(u8 *ptr) {
			return GetBuffer<i + 1, j, args...>::at(ptr);
		}

		template<>
		static Buffer at<true>(u8 *ptr) {
			return { ((T*)ptr)->size - (u32) sizeof(T), ptr + (u32) sizeof(T) };
		}
	};

	template<u32 i, u32 j, typename T>
	struct GetBuffer<i, j, T> {

		static Buffer at(u8 *ptr) {
			return { ((T*)ptr)->size - (u32) sizeof(T), ptr + (u32) sizeof(T) };
		}
	};

	///Get the maximum size of anything in ...args

	template<typename T, typename ...args>
	struct MaxSize {

		static constexpr size_t get_inter = MaxSize<args...>::get;

		static constexpr size_t get = sizeof(T) > get_inter ? sizeof(T) : get_inter;
	};

	template<typename T>
	struct MaxSize<T> {

		static constexpr size_t get = sizeof(T);

	};

	///List of compile time args

	template<typename ...args>
	struct CompileTimeList {};

	//Class for Getting indices from a compile time list
	struct CTLHelper {

		template<typename T, u32 i, typename T2, typename ...args>
		struct Index {

			template<bool b = std::is_same<T, T2>::value>
			static constexpr u32 get() { return Index<T, i + 1, args...>::get(); }

			template<>
			static constexpr u32 get<true>() { return i; }

		};

		template<typename T, u32 i, typename T2>
		struct Index<T, i, T2> {

			template<bool b = std::is_same<T, T2>::value>
			static constexpr u32 get() { return 0xFFFFFFFFU; }

			template<>
			static constexpr u32 get<true>() { return i; }
		};

		template<typename T, u32 i, typename ...args>
		static constexpr u32 index(CompileTimeList<args...> ctl) {
			return Index<T, i, args...>::get();
		}

	};

}