#pragma once
#include<type_traits>

//utility
namespace otmp
{
	template<class T>
	struct wrap
	{
		using type = T;
	};
	namespace deteil
	{
		template<class T>
		struct unwrap_impl
		{
			using type = typename T::type;
		};
	}
	template<class T>
	using unwrap = typename deteil::unwrap_impl<T>::type;

	template<class T>
	using identity = typename wrap<T>::type;

	namespace deteil
	{
		template<class R, class T, class F>
		struct cond_impl
			:std::conditional<!!R::value, T, F>
		{};
	}
	
	template<class R, class T, class F>
	using cond = typename deteil::cond_impl<R, T, F>::type;

	template<template<class...>class Func>
	struct lift
	{
		template<class...T>
		struct apply
			:Func<T...>
		{};
	};

	template<template<class...>class Func>
	struct self
	{
		template<class...T>
		struct apply
			:wrap<Func<T...>>
		{};
	};
}

//index_sequence
namespace otmp
{
	template<std::size_t ...Idxs>
	struct index_sequence
	{};
	namespace deteil
	{
		template<std::size_t N>
		struct make_index_sequence;
		
		template<std::size_t N>
		struct make_index_sequence
		{
			template<std::size_t...Idxs>
			static auto trans(index_sequence<Idxs...>, ...)
				->index_sequence<Idxs..., (N / 2 + Idxs)...>;
			template<std::size_t...Idxs>
			static auto trans(index_sequence<Idxs...>, std::integral_constant<bool, 1>)
				->index_sequence<Idxs..., (N / 2 + Idxs)..., N - 1>;
		public:
			using type = decltype(trans(typename make_index_sequence<N / 2>::type{}, std::integral_constant<bool, N % 2>{}));
		};
		template<>
		struct make_index_sequence<0>
		{
			using type = index_sequence<>;
		};
	}
	template<std::size_t N>
	using make_index_sequence = typename deteil::make_index_sequence<N>::type;
}
namespace otmp
{
	namespace deteil
	{
		template<std::size_t, class>
		struct Pair{};
		template<class...>
		struct List_impl;
		template<std::size_t...Idx, class...T>
		struct List_impl<index_sequence<Idx...>, T...>
			:Pair<Idx, T>...
		{};
	}

	template<typename...T>
	struct List
		:deteil::List_impl<make_index_sequence<sizeof...(T)>, T...>
	{};

	namespace deteil
	{
		template<std::size_t>
		struct do_nothing{ do_nothing(...); };

		template<class list>
		struct getLengthImpl
		{
			template<class...T>
			static std::integral_constant<std::size_t, (sizeof...(T))> impl(List<T...>);
			using type = decltype(impl(list{}));
		};
		template<std::size_t N, class list>
		struct at_impl
		{
			template<class T>
			static T impl(deteil::Pair<N, T>);
			using type = decltype(impl(list{}));
		};
		
		template<class listL, class listR>
		struct concat_impl
		{
			template<class...A, class...B>
			static List<A..., B...>impl(List<A...>, List<B...>);
			using type = decltype(impl(listL{}, listR{}));
		};
		
		template<class list>
		struct make_index_List_impl
		{
			template<std::size_t...Idxs>
			List<std::integral_constant<std::size_t, Idxs>...>impl(index_sequence<Idxs...>);
			using type = decltype(impl(list{}));
		};
	}

	template<class list>
	using getLength = unwrap<deteil::getLengthImpl<list>>;
	
	template<class list>
	using is_emptyList = cond<getLength<list>, std::false_type, std::true_type>;

	template<std::size_t N, class list>
	using at = unwrap<deteil::at_impl<N, list>>;

	template<class listL, class listR>
	using concat = unwrap<deteil::concat_impl<listL, listR>>;
	
	template<class list>
	using head = at<0, list>;
	
	template<std::size_t N>
	using make_index_List = unwrap<deteil::make_index_List_impl<make_index_sequence<N>>>;

}

//HiOrderFunction
namespace otmp
{
	template<class Func, class...Args>
	struct eval
		:Func::template apply<Args...>
	{};
	
	namespace deteil
	{
		template<class list, class Func>
		struct map_impl
		{
			template<class...T>
			static auto impl(List<T...>)
				->List<unwrap<eval<Func, T>>...>;
			using type = decltype(impl(list{}));
		};
		
		template<class list, class Func>
		struct apply_impl
		{
			template<class...T>
			static auto impl(List<T...>)
				->typename eval<Func, T...>::type;
			using type = decltype(impl(list{}));
		};
	}

	template<class list, class Func>
	using map = unwrap<deteil::map_impl<list, Func>>;


	template<class list, class Func>
	using apply = unwrap<deteil::apply_impl<list, Func>>;

	template<std::size_t>
	struct Arg{};

	template<class Func, class...Args>
	struct bind
	{
		template<class...T>
		class apply
		{
			template<class U>
			struct impl :wrap<U>	{};
			template<std::size_t N>
			struct impl<Arg<N>>:wrap<at<N, List<T...>>>{};
		public:
			using type = otmp::apply<map<List<Args...>, lift<impl>>, Func>;
		};
	};

	template<class Func, class...Args>
	struct carry
	{
		template<class...T>
		struct apply
			:eval<Func, Args..., T...>
		{};
	};

	template<class Func, class...Args>
	struct rcarry
	{
		template<class...T>
		struct apply
			:eval<Func, T..., Args...>
		{};
	};

	namespace deteil
	{
		template<class list, class Func>
		struct fold_impl
		{
			template<std::size_t Begin, std::size_t Len>
			struct impl : eval<Func, unwrap<impl<Begin, Len / 2>>, unwrap<impl<Begin + Len / 2, Len - Len / 2>> >{};

			template<std::size_t Begin>
			struct impl<Begin, 1> : wrap<at<Begin, list>>{};

			template<std::size_t Begin>
			struct impl<Begin, 0>;

			using type = typename impl<0, getLength<list>::value>::type;
		};
	}
	
	template<class list, class Func>
	using fold1 = unwrap<deteil::fold_impl<list, Func>>;

	template<class list, class Func, class Default>
	using fold = unwrap< cond<is_emptyList<list>, wrap<Default>, eval<self<fold1>, list, Func> > > ;

	namespace deteil
	{
		template<class list, class Func>
		struct filter_if_impl
		{
			template<class T>
			using impl = cond<unwrap<eval<Func, T>>, List<T>, List<>>;
			using type = fold<map<list, self<impl>>, self<concat>, List<>>;
		};
	}
	
	template<class list, class Func>
	using filter_if = unwrap<deteil::filter_if_impl<list, Func>>;
}

namespace otmp
{
	namespace deteil
	{
		template<class list, bool unit>
		struct logic_impl
		{
			template<class...T>
			static std::integral_constant<bool, unit> impl(List<T*...>);
			static std::integral_constant<bool, unit> impl(List<>);
			static std::integral_constant<bool, !unit> impl(...);
			using type = decltype(impl(list{}));
		};
	}

	template<class list, class Func>
	using all_of = unwrap<deteil::logic_impl<map<map<list, Func>, rcarry<self<cond>, void*, void>>, true>>;
	template<class list, class Func>
	using any_of = unwrap<deteil::logic_impl<map<map<list, Func>, rcarry<self<cond>, void, void*>>, false>>;
}

namespace otmp
{
	template<class list>
	using cat = fold<list, self<concat>, List<>>;
}