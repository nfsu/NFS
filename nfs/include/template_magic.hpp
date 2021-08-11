#pragma once
#include "generic.hpp"

namespace nfs {

	///Return reference from j in ...args
	
	template<usz i, usz j, typename T, typename ...args> struct ReadObject;
	
	template<bool isCorrect, usz i, usz j, typename T, typename ...args>
	struct ReadObject_inter {
		static auto &at(u8 *ptr) {
			return ReadObject<i + 1, j, args...>::at(ptr);
		}
	};

	template<usz i, usz j, typename T, typename ...args>
	struct ReadObject_inter<true, i, j, T, args...> {
		static auto &at(u8 *ptr) {
			return *(T*)ptr;
		}
	};

	template<usz i, usz j, typename T, typename ...args>
	struct ReadObject {
		static auto &at(u8 *ptr) {
			return ReadObject_inter<i == j, i, j, T, args...>::at(ptr);
		}
	};

	template<usz i, usz j, typename T>
	struct ReadObject<i, j, T> {
		static T &at(u8 *ptr) {
			return *(T*)ptr;
		}
	};

	///Find offset of j in ...args

	template<usz i, usz j, typename T, typename ...args>
	struct FindOffset {
		static constexpr usz get = i < j ? sizeof(T) + FindOffset<i + 1, j, args...>::get : 0;
	};

	template<usz i, usz j, typename T>
	struct FindOffset<i, j, T> {
		static constexpr usz get = i < j ? sizeof(T) : 0;
	};

	///Run placement new everything in  ...args

	template<typename T, typename ...args> struct PlacementNew;

	template<typename T, typename ...args>
	struct PlacementNew_inter {

		template<typename T2, typename ...args2>
		static void run(u8 *ptr, T2 &t2, args2 &...arg) {
			::new(ptr) T(t2);
			PlacementNew<args...>::run(ptr + sizeof(T), arg...);
		}
	};

	template<typename T>
	struct PlacementNew_inter<T> {

		template<typename T2>
		static void run(u8 *ptr, T2 &t2) {
			::new(ptr) T(t2);
		}
	};

	template<typename T, typename ...args>
	struct PlacementNew {

		static void run(u8 *ptr) {
			::new(ptr) T();
			PlacementNew<args...>::run(ptr + sizeof(T));
		}

		template<typename ...args2>
		static void run(u8 *ptr, args2 &...arg) {
			PlacementNew_inter<T, args...>::run(ptr, arg...);
		}
	};

	template<typename T>
	struct PlacementNew<T> {

		static void run(u8 *ptr) {
			::new(ptr) T();
		}

		template<typename T2>
		static void run(u8 *ptr, T2 &t2) {
			::new(ptr) T(t2);
		}
	};

	///Get buffer of j in ...args

	template<usz i, usz j, typename T, typename ...args> struct GetBuffer;
	
	template<bool isCorrect, usz i, usz j, typename T, typename ...args>
	struct GetBuffer_inter {
		static Buffer at(u8 *ptr) {
			return GetBuffer<i + 1, j, args...>::at(ptr);
		}
	};

	template<usz i, usz j, typename T, typename ...args>
	struct GetBuffer_inter<true, i, j, T, args...> {
		static Buffer at(u8 *ptr) {
			return { ((T*)ptr)->size - sizeof(T), ptr + sizeof(T) };
		}
	};

	template<usz i, usz j, typename T, typename ...args>
	struct GetBuffer {
		static Buffer at(u8 *ptr) {
			return GetBuffer_inter<i == j, i, j, T, args...>::at(ptr);
		}
	};

	template<usz i, usz j, typename T>
	struct GetBuffer<i, j, T> {
		static Buffer at(u8 *ptr) {
			return { ((T*)ptr)->size - sizeof(T), ptr + sizeof(T) };
		}
	};

	///Get the maximum size of anything in ...args

	template<typename T, typename ...args>
	struct MaxSize {
		static constexpr usz get_inter = MaxSize<args...>::get;
		static constexpr usz get = sizeof(T) > get_inter ? sizeof(T) : get_inter;
	};

	template<typename T>
	struct MaxSize<T> {
		static constexpr usz get = sizeof(T);
	};

	///List of compile time args

	template<typename ...args>
	struct CompileTimeList {};

	//Class for Getting indices from a compile time list
	struct CTLHelper {

		template<typename T, usz i, typename T2, typename ...args>
		struct Index {
			static constexpr usz get() { return std::is_same_v<T, T2> ? i : Index<T, i + 1, args...>::get(); }
		};

		template<typename T, usz i, typename T2>
		struct Index<T, i, T2> {
			static constexpr usz get() { return std::is_same_v<T, T2> ? i : usz_MAX; }
		};

		template<typename T, usz i, typename ...args>
		static constexpr usz index(CompileTimeList<args...> ctl) {
			return Index<T, i, args...>::get();
		}
	};
}