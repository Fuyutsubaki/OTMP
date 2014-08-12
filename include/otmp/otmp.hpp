#ifndef MAGIC_OTMP_HPP
#define MAGIC_OTMP_HPP
#include<type_traits>


namespace otmp
{
	template<typename...T>
	struct List
	{};
}
//utility
namespace otmp
{
	template<class T>
	struct identity
	{
		using type = T;
	};

	//!!workaround!!
	//typename T::typeをテンプレートエイリアスで使うとバグる
	//template<class T>
	//using unbox_t = typename unbox<T>::type;
	template<class T>
	struct unbox
	{
		using type = typename T::type;
	};
	template<class T>
	using unbox_t = typename unbox<T>::type;


	template<bool r>
	struct Bool_
		:std::conditional<r, std::true_type, std::false_type>
	{};
	template<bool r>
	using Bool_t = unbox_t<Bool_<r>>;


	template<class R, class T, class F>
	struct if_
		:std::conditional<!!R::value, T, F>
	{};
	template<class R, class T, class F>
	using if_t = unbox_t<if_<R, T, F>>;


	template<template<class...>class Func>
	struct lift
	{
		template<class...T>
		struct apply
			:Func<T...>
		{};
	};

	template<template<class...>class Func>
	struct lift_c
	{
		template<class...T>
		struct apply
			:identity<Func<T...>>
		{};
	};
}
//index_sequence
namespace otmp
{
	template<std::size_t ...Idxs>
	struct index_sequence
	{};
	template<std::size_t N>
	struct make_index_sequence;
	template<std::size_t N>
	using make_index_sequence_t = typename make_index_sequence<N>::type;
	template<std::size_t N>
	struct make_index_sequence
	{
		template<std::size_t...Idxs>
		static auto trans(index_sequence<Idxs...>, ...)
			->index_sequence<Idxs..., (N / 2 + Idxs)...>;
		template<std::size_t...Idxs>
		static auto trans(index_sequence<Idxs...>, Bool_<true>)
			->index_sequence<Idxs..., (N / 2 + Idxs)..., N - 1>;
	public:
		using type = decltype(trans(make_index_sequence_t<N / 2>{}, Bool_<N % 2>{}));
	};
	template<>
	struct make_index_sequence<0>
	{
		using type = index_sequence<>;
	};
}
//List Helper
namespace otmp
{
	namespace deteil
	{
		template<std::size_t>
		struct do_nothing{ do_nothing(...); };
	}
	
	template<class list>
	class getLength
	{
		template<class>
		struct impl;
		template<class...T>
		struct impl<List<T...>>
		{
			using type = std::integral_constant<std::size_t, (sizeof...(T))>;
		};
	public:
		using type = typename impl<list>::type;
	};
	template<class list>
	using getLength_t = unbox_t<getLength<list>>;


	template<class list>
	struct is_emptyList
		:if_<getLength_t<list>, std::false_type, std::true_type>
	{};
	template<class list>
	using is_emptyList_t = unbox_t<is_emptyList<list>>;


	template<std::size_t N, class list>
	class at
	{
		static_assert(N < getLength<list>::type::value, "out of range");
		template<class T, class IdxSeq>struct impl;
		template<class...T, std::size_t ...Idxs>
		struct impl<List<T...>, index_sequence<Idxs...>>
		{
			template<class U>
			static U trans(deteil::do_nothing<Idxs>..., U, ...);
			using type = unbox_t<decltype(trans(identity<T>{}...))>;
		};

	public:
		using type = unbox_t<impl<list, make_index_sequence_t<N>>>;
	};
	template<std::size_t N, class list>
	using at_t = unbox_t<at<N, list>>;


	template<class listL, class listR>
	class concat
	{
		template<class...A, class...B>
		static List<A..., B...>impl(List<A...>, List<B...>);
	public:
		using type = decltype(impl(std::declval<listL>(), std::declval<listR>()));
	};


	template<std::size_t N, class list>
	struct drop
	{
		template<class T, class IdxSeq>struct impl;
		template<class...T, std::size_t ...Idxs>
		struct impl<List<T...>, index_sequence<Idxs...>>
		{
			template<class...U>
			static List<unbox_t<U>...> trans(deteil::do_nothing<Idxs>..., U...);
			using type = decltype(trans(identity<T>{}...));
		};
	public:
		using type = unbox_t<impl<list, make_index_sequence_t<N>>>;
	};
	template<std::size_t N, class list>
	using drop_t = unbox_t<drop<N,list>>;

	template<class list>
	struct head
		:at<0, list>
	{};
	template<class list>
	using head_t = unbox_t<head<list>>;


	template<class list>
	struct tail
		:drop<1, list>
	{};
	template<class list>
	using tail_t = unbox_t<tail<list>>;
}
//HiOrderFunction
namespace otmp
{
	template<class Func, class...Args>
	struct eval
		:Func::template apply<Args...>
	{};
	namespace HOFdeteil
	{
		template<class Func, class...T>
		auto map_impl(List<T...>, Func)
			->List<unbox_t<eval<Func, T>>...>;

		template<class Func, class...T>
		auto apply_impl(List<T...>, Func)
			->eval<Func, T...>;
	}


	template<class list, class Func>
	struct map : identity<decltype(HOFdeteil::map_impl(std::declval<list>(), std::declval<Func>()))>	{};
	template<class list, class Func>
	using map_t = unbox_t<map<list, Func>>;

	
	template<class list, class Func>
	struct apply : decltype(HOFdeteil::apply_impl(std::declval<list>(), std::declval<Func>()))	{};
	template<class list, class Func>
	using apply_t = unbox_t<apply<list, Func>>;

	template<std::size_t>
	struct Arg
	{};

	template<class list, class Func>
	struct bind
	{
		using type = bind;
		template<class...T>
		class apply
		{
			template<class U>
			struct impl :identity<U>	{};
			template<std::size_t N>
			struct impl<Arg<N>>:at<N, List<T...>>{};
		public:
			using type = apply_t<map_t<list, lift<impl>>, Func>;
		};
	};

	template<class Func, class...Args>
	struct S
	{
		template<class...T>
		class apply
		{
			template<class U>
			struct impl :identity<U>{};
			template<std::size_t N>
			struct impl<Arg<N>>:at<N, List<T...>>{};
			template<class ...U>
			struct impl<S<U...>>:eval<S<U...>, T...>{};
		public:
			using type = apply_t<map_t<List<Args...>, lift<impl>>, Func>;
		};
	};

	template<class list, class Func>
	class fold1
	{
		template<std::size_t Begin, std::size_t Len>
		struct impl : apply<List<unbox_t<impl<Begin, Len / 2>>, unbox_t<impl<Begin + Len / 2, Len - Len / 2>>>, Func >{};

		template<std::size_t Begin>
		struct impl<Begin, 1> : at<Begin, list>{};

	public:
		using type = typename impl<0, getLength_t<list>::value>::type;
	};
	template<class list, class Func>
	using fold1_t = unbox_t<fold1<list, Func>>;

	template<class list, class Func, class Unit>
	struct fold
		:if_t<is_emptyList_t<list>, identity<Unit>, fold1<list, Func>>
	{};
	template<class list, class Func, class Unit>
	using fold_t = unbox_t<fold<list, Func, Unit>>;



	template<class list, class Func>
	class filter_if
	{
		template<class T>
		struct impl
			:if_<apply<List<T>, Func>, List<T>, List<>>
		{};
	public:

		using type = fold_t<map_t<list, lift<impl>>, lift<concat>, List<>>;
	};
	template<class list, class Func>
	using filter_if_t = unbox_t<filter_if<list, Func>>;
}
//Logic
namespace otmp
{
	namespace deteil
	{
		template<bool R, bool S, class list>
		class logic_impl
		{
			template<class...T>
			static Bool_t<S> impl(List<std::integral_constant<T, S>...>);
			static Bool_t<!S> impl(...);
		public:
			using type = decltype(impl(std::declval<list>()));
		};
	}

	template<class list, class Func>
	struct all_of :deteil::logic_impl<true, true, map_t<list, Func>>	{};
	template<class list, class Func>
	using all_of_t = unbox_t<all_of<list, Func>>;

	template<class list, class Func>
	struct any_of :deteil::logic_impl<false, false, map_t<list, Func>>	{};
	template<class list, class Func>
	using any_of_t = unbox_t<any_of<list, Func>>;
}

namespace otmp
{

	template<class list>
	struct cat
		: fold<list, lift<concat>, List<>>
	{};
	template<class list>
	using cat_t = unbox_t<cat<list>>;


}
#endif