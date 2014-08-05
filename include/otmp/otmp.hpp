#ifndef OTMP_OTMP_HPP
#define OTMP_OTMP_HPP
#include<type_traits>

namespace otmp
{
	template<bool r>
	struct Bool_
		:std::conditional<r, std::true_type, std::false_type>
	{};

	template<class T>
	struct identity
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

	template<class R, class T, class F>
	struct if_
		:std::conditional<!!R::value, T, F>
	{};

	template<template<class...>class Func>
	struct lift
	{
		template<class...T>
		struct apply
			:Func<T...>
		{};
	};

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

	/////////////////////////////////////////////////////////////////////////////////

	template<typename...T>
	struct List
	{};

	template<class list>
	class getLength
	{
		template<class...T>
		static std::integral_constant<std::size_t, (sizeof...(T))> impl(List<T...>);
	public:
		using type = decltype(impl(list{}));
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
		static_assert(N < getLength_t<list>::value, "out of range");
		template<std::size_t>
		struct do_nothing{ do_nothing(...); };
		template<class T, class IdxSeq>struct impl;
		template<class...T, std::size_t ...Idxs>
		struct impl<List<T...>, index_sequence<Idxs...>>
		{
			template<class U>
			static U trans(do_nothing<Idxs>..., U, ...);
			using type = unbox_t<decltype(trans(identity<T>{}...))>;
		};

	public:
		using type = unbox_t<impl<list, make_index_sequence_t<N>>>;
	};
	template<std::size_t N, class list>
	using at_t = unbox_t<at<N, list>>;

	template<class Func, class list>
	struct eval
	{
		template<class...T>
		static auto trans(List<T...>)->unbox_t<typename Func::template apply<T...>>;
		using type = decltype(trans(list{}));
	};
	template<class Func, class list>
	using eval_t = unbox_t<eval<Func, list>>;

	template<class list, class Func>
	class map
	{
		template<class...T>
		static auto trans(List<T...>)
			->List<eval_t<Func, List<T>>...>;
	public:
		using type = decltype(trans(list{}));
	};
	template<class list, class Func>
	using map_t = unbox_t<map<list, Func>>;

	template<class list, class Func>
	class fold1
	{
		template<std::size_t Begin, std::size_t Len>
		struct impl
			:eval<Func, List<unbox_t<impl<Begin, Len / 2>>, unbox_t<impl<Begin + Len / 2, Len - Len / 2>>> >
		{};
		template<std::size_t Begin>
		struct impl<Begin, 1>
			:at<Begin, list>
		{};
	public:
		using type = unbox_t<impl<0, getLength_t<list>::value>>;
	};
	template<class list, class Func>
	using fold1_t = unbox_t<fold1<list, Func>>;

	template<class list, class Func, class Unit>
	struct fold
		:if_<is_emptyList_t<list>, identity<Unit>, fold1<list, Func>>::type
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
			using type = decltype(trans(L{}, R{}));
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
			:if_<eval_t<Func, List<T>>, List<T>, List<>>
		{};
	public:
		using type = cat_t<map_t<list, lift<impl>>>;
	};
	template<class list, class Func>
	using filter_if_t = unbox_t<filter_if<list, Func>>;


	template<class list, class Pred>
	class all
	{
		template<class T>
		struct impl
			:Bool_<eval_t<Pred, List<T>>::value>
		{};

		template<class...T>
		static std::true_type trans(List<std::integral_constant<T, true>...>);
		static std::false_type trans(...);

	public:
		using type = decltype(trans(map_t<list, lift<impl>>{}));
	};

	template<class list, class Pred>
	using all_t = unbox_t<all<list, Pred>>;

	template<std::size_t N>
	class make_index_list
	{
		template<std::size_t...Idxs>
		static auto trans(index_sequence<Idxs...>)->List<std::integral_constant<std::size_t, Idxs>...>;
	public:
		using type = decltype(trans(make_index_sequence_t<N>{}));
	};
	template<std::size_t N>
	using make_index_list_t = unbox_t<make_index_list<N>>;




}//namespace otmp

#endif