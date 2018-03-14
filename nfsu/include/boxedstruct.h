#pragma once

#include <generic.h>
#include <templatemagic.h>

namespace nfsu {

	template<typename ...args>
	struct TBoxedStruct {

		static constexpr u32 types() { return nfs::CountArgs<args...>::get(); }
		static constexpr u32 dataSize() { return nfs::FindOffset<0, types(), args...>::get; }

		u8 data[dataSize()];

		TBoxedStruct() {
			nfs::PlacementNew<args...>::run(data);
		}

		template<typename ...arg2>
		TBoxedStruct(arg2 ...arg) {
			nfs::PlacementNew<args...>::run(data, arg...);
		}

		TBoxedStruct(const TBoxedStruct &other) {
			copy(other);
		}

		TBoxedStruct &operator=(const TBoxedStruct &other) {
			copy(other);
			return *this;
		}

		template<u32 i>
		auto &get() {
			return nfs::ReadObject<0, i, args...>::at(data + nfs::FindOffset<0, i, args...>::get);
		}

		template<u32 i>
		const auto &find() const {
			return nfs::ReadObject<0, i, args...>::at(const_cast<u8*>(data) + nfs::FindOffset<0, i, args...>::get);
		}

		template< template<typename T2> typename F, typename ...args2>
		void run(args2... arg) {
			Run_inter<F, 0, args...>::run(arg...);
		}

	private:

		void copy(const TBoxedStruct &toCopy) {
			Copy_inter<0, args...>::run(*this, toCopy, 0);
		}

		template<u32 i, typename T2, typename ...args2>
		struct Copy_inter {

			static void run(TBoxedStruct &og, const TBoxedStruct &toCopy, u32 offset) {

				::new(og.data + offset) T2(toCopy.find<i>());
				Copy_inter<i + 1, args2...>::run(og, toCopy, offset + sizeof(T2));

			}

		};

		template<u32 i, typename T2>
		struct Copy_inter<i, T2> {

			static void run(TBoxedStruct &og, const TBoxedStruct &toCopy, u32 offset) {
				::new(og.data + offset) T2(toCopy.find<i>());
			}

		};

		template< template<typename T2> typename F, u32 i, typename T, typename ...args>
		struct Run_inter {

			template<typename ...args2>
			static void run(TBoxedStruct &boxs, args2... arg) {
				F<T>::run(boxs.get<i>(), arg...);
				Run_inter<F, i + 1, args...>::run(boxs, arg...);
			}
		};

		template< template<typename T2> typename F, u32 i, typename T>
		struct Run_inter<F, i, T> {

			template<typename ...args2>
			static void run(TBoxedStruct &boxs, args2... arg) {
				F<T>::run(boxs.get<i>(), arg...);
			}

		};
	};

}