#include<otmp.hpp>

namespace otmp
{
	//head
	static_assert(std::is_same<head_t<List<int, char, void>>, int>::value, "");
	//tail
	static_assert(std::is_same<tail_t<List<int, char, void>>, List<char, void>>::value, "");

	//map
	static_assert(std::is_same<map_t<List<int, char>, lift<std::add_pointer>>, List<int*, char*>>::value, "");
	//bind
	static_assert(eval<bind<lift<std::is_same>, Arg<0>, int> , int > ::type::value, "");
	//S
	
	static_assert(
		std::is_same<
			eval<
				S<
					lift<if_>,
					S<lift<std::is_integral>, Arg<0>>,
					S<lift<std::add_pointer>, Arg<0>>,
					void
				>
				, int
			>::type
		, int*>::value
		, "");
	//filter
	static_assert(std::is_same<filter_if_t<List<char, void, int>, lift<std::is_integral>>, List<char, int>>::value, "");

	//all
	static_assert(!all_of_t<List<char, void>, lift<std::is_integral>>::value, "");
	static_assert(all_of_t<List<char, int>, lift<std::is_integral>>::value, "");

	//any
	static_assert(any_of_t<List<char, void>, lift<std::is_integral>>::value, "");
	static_assert(!any_of_t<List<void, int&>, lift<std::is_integral>>::value, "");

	//cat
	static_assert(std::is_same<cat_t<List<>>, List<>>::value, "");
	static_assert(std::is_same<cat<List<List<char>, List<>>>::type, List<char>>::value, "");

	//unique
	static_assert(std::is_same < unique_t<List< int, long, int, long, void, int>>, List<int, long, void>>::value, "");
}
