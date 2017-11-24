#pragma once
#include "TypeListClass.h"

namespace lag
{
	template<typename T, typename ... Types>
	struct ListFunc;

	template<typename T, typename First, typename ... Types>
	struct ListFunc<T, First, Types...>
	{
		static void func(T &t)
		{
			t.func<First>();
			ListFunc<T, Types...>::func(t);
		}
	};

	template<typename T>
	struct ListFunc<T>
	{
		static void func(T &t)
		{}
	};

	template<typename ... Types, typename T>
	void execute_typelist(TypeList<Types...>, T &t)
	{
		ListFunc<T, Types...>::func(t);
	}
}