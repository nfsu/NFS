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

		template<u32 i>
		auto &get() {
			return nfs::ReadObject<0, i, args...>::at(data + nfs::FindOffset<0, i, args...>::get);
		}

		template< template<typename T2> typename F, typename ...args2>
		void run(args2... arg) {
			Run_inter<F, args...>::run(arg...);
		}

	private:

		template< template<typename T2> typename F, typename T, typename ...args>
		struct Run_inter {

			template<typename ...args2>
			static void run(args2... arg) {
				F<T>::run(arg...);
				Run_inter<F, args...>::run(arg...);
			}
		};

		template< template<typename T2> typename F, typename T>
		struct Run_inter<F, T> {

			template<typename ...args2>
			static void run(args2... arg) {
				F<T>::run(arg...);
			}

		};
	};

}