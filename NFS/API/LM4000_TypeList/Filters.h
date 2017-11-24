#pragma once
#include <type_traits>
#include "Checks.h"

namespace lag
{
	template<typename Add, typename ... Types>
	using AddUnique = typename std::conditional<
		ContainsType<Add, Types...>,
		TypeList<Types...>,
		TypeList<Types..., Add>>::type;

	template<typename ... Types>
	struct MakeUnique;

	template<typename First, typename ... Types>
	struct MakeUnique<First, Types...>
	{
		template<typename ... TypeListTypes>
		static constexpr auto eval(TypeList<TypeListTypes...>)
			->decltype(MakeUnique<Types...>::eval(AddUnique<First, TypeListTypes...>{}))
		{
			return {};
		}
	};

	template<>
	struct MakeUnique<>
	{
		template<typename ... TypeListTypes>
		static constexpr auto eval(TypeList<TypeListTypes...>)
			->TypeList<TypeListTypes...>
		{
			return {};
		}
	};

	using EmptyTypeList = TypeList<>;
	template<typename ... Types>
	constexpr auto make_unique(TypeList<Types...>)
		->decltype(MakeUnique<Types...>::eval(EmptyTypeList{}))
	{
		return {};
	}

	//add two typelists together
	template<typename ... A, typename ... B>
	constexpr auto make_combined(TypeList<A...>, TypeList<B...>)
		-> TypeList<A..., B...> { return {}; }
}