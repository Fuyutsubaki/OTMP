#ifndef MAGIC_OTMP_HPP
#define MAGIC_OTMP_HPP
#include<type_traits>

namespace otmp
{
	template<bool r>
	struct Bool_
		:std::conditional<r, std::true_type, std::false_type>
	{};

	template<class T>
	struct id
	{
		using type = T;
	};

	template<class T>
	struct unbox
	{
		using type = typename T::type;
	};

	template<class T>
	using unbox_t = typename unbox<T>::type;

	template<class T>
	unbox_t<T> unboxval();
	template<class R, class T, class F>
	struct if_
		:std::conditional<!!unbox_t<R>::value, T, F>::type
	{};

	template<template<class...>class Func>
	struct lift
	{
		using type = lift;
		template<class...T>
		struct apply
			:Func<T...>
		{};
	};

	namespace helper
	{
		template<class Func, class...Args>
		struct eval
			:Func::template apply<Args...>
		{};
	}
}


namespace otmp
{
	template<std::size_t ...Idxs>
	struct index_sequence
	{
		using type = index_sequence;
	};
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
		static auto trans(index_sequence<Idxs...>, std::true_type)
			->index_sequence<Idxs..., (N / 2 + Idxs)..., N - 1>;
	public:
		using type = decltype(trans(make_index_sequence_t<N / 2>{}, unbox_t<Bool_<N % 2>>{}));
	};
	template<>
	struct make_index_sequence<0>
	{
		using type = index_sequence<>;
	};
}



namespace otmp
{
	template<typename...T>
	struct List
	{
		using type = List;
	};

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
		using type = typename impl<unbox_t<list>>::type;
	};
	template<class list>
	using getLength_t = unbox_t<getLength<list>>;


	template<class list>
	struct is_emptyList
		:if_<getLength<list>, id<std::false_type>, id<std::true_type>>::type
	{};
	template<class list>
	using is_emptyList_t = unbox_t<is_emptyList<list>>;
	

	template<std::size_t N, class list>
	class at
	{
		static_assert(N < getLength<list>::type::value, "out of range");
		template<std::size_t>
		struct do_nothing{ do_nothing(...); };
		template<class T, class IdxSeq>struct impl;
		template<class...T, std::size_t ...Idxs>
		struct impl<List<T...>, index_sequence<Idxs...>>
		{
			template<class U>
			static U trans(do_nothing<Idxs>..., U, ...);
			using type = unbox_t<decltype(trans(id<T>{}...))>;
		};

	public:
		using type = unbox_t<impl<unbox_t<list>, make_index_sequence_t<N>>>;
	};
	template<std::size_t N, class list>
	using at_t = unbox_t<at<N, list>>;

}

namespace otmp
{
	namespace deteil
	{
		template<class Func, class...T>
		auto map_impl(List<T...>, Func)
			->List<unbox_t<helper::eval<Func, T>>...>;

		template<class Func, class...T>
		auto apply_impl(List<T...>, Func)
			->helper::eval<Func, T...>;
	}


	template<class list, class Func>
	struct map : decltype(deteil::map_impl(unboxval<list>(), unboxval<Func>()))	{};
	template<class list, class Func>
	using map_t = unbox_t<map<list, Func>>;

	template<class list, class Func>
	struct apply : decltype(deteil::apply_impl(unboxval<list>(), unboxval<Func>()))	{};
	template<class list, class Func>
	using apply_t = unbox_t<apply<list, Func>>;

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
		:if_<is_emptyList<list>, id<Unit>, fold1<list, Func>>::type
	{};
	template<class list, class Func, class Unit>
	using fold_t = unbox_t<fold<list, Func, Unit>>;


	template<class list>
	class cat
	{
		template<class L, class R>
		struct impl
		{
			template<class...A, class...B>
			static List<A..., B...>trans(List<A...>, List<B...>);
			using type = decltype(trans(std::declval<L>(), std::declval<R>()));
		};
	public:
		using type = fold_t<list, lift<impl>, List<>>;
	};
	template<class list>
	using cat_t = unbox_t<cat<list>>;


	template<class list, class Func>
	class filter_if
	{
		template<class T>
		struct impl
			:if_<apply<List<T>, Func>, List<T>, List<>>
		{};
	public:
		using type = cat_t<map<list, lift<impl>>>;
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
			static Bool_<S> impl(List<std::integral_constant<T, S>...>);
			static Bool_<!S> impl(...);
		public:
			using type = unbox_t<decltype(impl(unboxval<list>()))>;
		};
	}

	template<class list, class Func>
	struct all_ :deteil::logic_impl<true, true, map<list, Func>>	{};
	template<class list, class Func>
	using all_t = unbox_t<all_<list, Func>>;

	template<class list, class Func>
	struct any_ :deteil::logic_impl<false, false, map<list, Func>>	{};	
	template<class list, class Func>
	using any_t = unbox_t<any_<list, Func>>;
}
#endif