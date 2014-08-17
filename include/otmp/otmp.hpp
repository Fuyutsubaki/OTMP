#ifndef MAGIC_OTMP_HPP
#define MAGIC_OTMP_HPP
#include<type_traits>


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

	template<class R>
	struct not_
		:identity<std::integral_constant<bool,!R::value>>
	{};
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

namespace otmp
{
	namespace deteil
	{
		template<std::size_t, class>
		struct Pair{};
		template<class...>
		struct Vector_impl;
		template<std::size_t...Idx, class...T>
		struct Vector_impl<index_sequence<Idx...>, T...>
			:Pair<Idx, T>...
		{};
	}

	template<typename...T>
	struct Vector
		:deteil::Vector_impl<make_index_sequence_t<sizeof...(T)>, T...>
	{};

	namespace deteil
	{
		template<std::size_t>
		struct do_nothing{ do_nothing(...); };
		template<class...T>
		static auto getLengthImpl(Vector<T...>)->std::integral_constant<std::size_t, (sizeof...(T))>;

		template<std::size_t N, class T>
		T at_impl(deteil::Pair<N, T>);

		template<class...A, class...B>
		static Vector<A..., B...>concat_impl(Vector<A...>, Vector<B...>);
	}
	
	template<class list>
	struct getLength
		:identity<decltype(deteil::getLengthImpl(std::declval<list>()))>
	{};
	template<class list>
	using getLength_t = unbox_t<getLength<list>>;


	template<class list>
	struct is_emptyList
		:if_<getLength_t<list>, std::false_type, std::true_type>
	{};
	template<class list>
	using is_emptyList_t = unbox_t<is_emptyList<list>>;


	template<std::size_t N, class list>
	struct at
		:identity<decltype(deteil::at_impl<N>(std::declval<list>()))>
	{
		static_assert(N < getLength_t<list>::value, "Out of Range");
	};
	template<std::size_t N, class list>
	using at_t = unbox_t<at<N, list>>;


	template<class listL, class listR>
	struct concat
		:identity<decltype(deteil::concat_impl(std::declval<listL>(), std::declval<listR>()))>
	{};
	template<class listL, class listR>
	using concat_t = unbox_t<concat<listL, listR>>;

	template<std::size_t N, class list>
	class drop
	{
		template<class T, class IdxSeq>struct impl;
		template<class...T, std::size_t ...Idxs>
		struct impl<Vector<T...>, index_sequence<Idxs...>>
		{
			template<class...U>
			static Vector<unbox_t<U>...> trans(deteil::do_nothing<Idxs>..., U...);
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

	template<class ...T>
	struct make_vector
		:identity<Vector<T...>>
	{};
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
		auto map_impl(Vector<T...>, Func)
			->Vector<unbox_t<eval<Func, T>>...>;

		template<class Func, class...T>
		auto apply_impl(Vector<T...>, Func)
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

	template<class Func, class...Args>
	struct bind
	{
		template<class...T>
		class apply
		{
			template<class U>
			struct impl :identity<U>	{};
			template<std::size_t N>
			struct impl<Arg<N>>:at<N, Vector<T...>>{};
		public:
			using type = apply_t<map_t<Vector<Args...>, lift<impl>>, Func>;
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
			struct impl<Arg<N>>:at<N, Vector<T...>>{};
			template<class ...U>
			struct impl<S<U...>>:eval<S<U...>, T...>{};
		public:
			using type = apply_t<map_t<Vector<Args...>, lift<impl>>, Func>;
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


	template<class Func>
	struct not_apply
	{
		template<class...T>
		struct apply
			:not_<unbox_t<eval<Func, T...>>>
		{};
	};
	template<class list, class Func>
	class fold1
	{
		template<std::size_t Begin, std::size_t Len>
		struct impl : eval<Func, unbox_t<impl<Begin, Len / 2>>, unbox_t<impl<Begin + Len / 2, Len - Len / 2>>>{};

		template<std::size_t Begin>
		struct impl<Begin, 1> : at<Begin, list>{};

		template<std::size_t Begin>
		struct impl<Begin, 0> ;

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
			:if_<unbox_t<eval<Func,T>>, Vector<T>, Vector<>>
		{};
	public:
		//using type = map_t < list, lift<impl>>;
		using type = fold_t<map_t<list, lift<impl>>, lift<concat>, Vector<>>;
	};
	template<class list, class Func>
	using filter_if_t = unbox_t<filter_if<list, Func>>;
}
//Logic
namespace otmp
{
	namespace deteil
	{
		template<class T>
		struct booltype_aound
			:identity<std::integral_constant<bool, T::value>>
		{};
		template<bool R, bool S, class list>
		class logic_impl
		{
			template<class...T>
			static Bool_t<S> impl(Vector<std::integral_constant<T, S>...>);
			template<class...T>
			static Bool_t<!S> impl(Vector<T...>);
		public:
			using type = decltype(impl(std::declval<map_t<list, lift<booltype_aound>>>()));
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
		: fold<list, lift<concat>, Vector<>>
	{};
	template<class list>
	using cat_t = unbox_t<cat<list>>;

	template<class elem, class list>
	struct in
		: any_of<list, carry<lift_c<std::is_same>, elem>>
	{};
	template<class elem, class list>
	using in_t = unbox_t<in<elem, list>>;

	template<class listL, class listR>
	struct unique_concat
		:concat<filter_if_t<listL, rcarry<lift<in>, listR>>, listR>
	{};

	template<class list>
	struct unique
		:fold<map_t<list, lift<make_vector>>, lift<unique_concat>, Vector<>>
	{};
	template<class list>
	using unique_t = unbox_t<unique<list>>;
}
#endif