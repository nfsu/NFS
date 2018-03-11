#pragma once
#include "generic.h"

namespace nfs {

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

	template<typename T, typename ...args>
	struct MaxSize {

		static constexpr size_t get_inter = MaxSize<args...>::get;

		static constexpr size_t get = sizeof(T) > get_inter ? sizeof(T) : get_inter;
	};

	template<typename T>
	struct MaxSize<T> {

		static constexpr size_t get = sizeof(T);

	};


	template<typename ...args>
	struct CompileTimeList {};

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