#ifndef OTMP_OTMP_HPP
#define OTMP_OTMP_HPP
#include<type_traits>


namespace tupleple
{
	namespace mpl
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
			:std::conditional<R::value, T, F>
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
				->index_sequence<Idxs..., (N / 2 + Idxs)..., N>;
		public:
			using type = decltype(trans(make_index_sequence_t<N / 2>{}, mpl::unbox_t<mpl::Bool_<N % 2>>{}));
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

		template<std::size_t N, class list>
		class at
		{
			static_assert(N < getLength_t<list>::value, "out of range");
			template<std::size_t>
			struct do_nothing{ do_nothing(...); };
			template<class T, class IdxSeq>struct impl;
			template<class...T, std::size_t ...Idxs>
			struct impl<List<T...>,index_sequence<Idxs...>>
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
			struct impl;
			template<std::size_t Begin, std::size_t Len>
			struct wrap
				:eval <Func, List<unbox_t<impl<Begin, Len / 2>>, unbox_t<impl<Begin + Len / 2, Len - Len / 2>>> >
			{};

			template<std::size_t Begin, std::size_t Len>
			struct impl
				:std::conditional<
				(Len == 1),
				at<Begin, list>,
				wrap<Begin, Len>
				>::type
			{};
		public:
			using type = unbox_t<impl<0, getLength_t<list>::value>>;
		};
		template<class list, class Func>
		using fold1_t = unbox_t<fold1<list, Func>>;

		template<class list, class Func, class Unit>
		struct fold
			:if_<getLength_t<list>, fold1<list, Func>, identity<Unit>>::type
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
			using type = fold_t<list, lift<impl>,List<>>;
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


	}
}



#endif