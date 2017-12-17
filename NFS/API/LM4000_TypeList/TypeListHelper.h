#pragma once
#include "TypeList.h"

namespace lag {

	template<template<typename T2> typename F>
	struct RunForType {

		template<typename T, typename ...args, typename ...fargs>
		static void run(TypeList<T, args...> typeList, fargs... farg) {
			F<T> functor;
			functor(farg...);
			run(TypeList<args...>(), farg...);
		}

		template<typename T, typename ...fargs>
		static void run(TypeList<T> typeList, fargs... farg) {
			F<T> functor;
			functor(farg...);
		}

	};

}