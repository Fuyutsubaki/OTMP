#include<otmp.hpp>

namespace otmp
{
	//head
	static_assert(std::is_same<head_t<Vector<int, char, void>>, int>::value, "");
	//tail
	static_assert(std::is_same<tail_t<Vector<int, char, void>>, Vector<char, void>>::value, "");

	//map
	static_assert(std::is_same<map_t<Vector<int, char>, lift<std::add_pointer>>, Vector<int*, char*>>::value, "");
	//bind
	static_assert(eval<bind<lift_c<std::is_same>, Arg<0>, int> , int > ::type::value, "");
	//S
	
	static_assert(
		std::is_same<
			eval<
				S<
					lift<if_>,
					S<lift_c<std::is_integral>, Arg<0>>,
					S<lift<std::add_pointer>, Arg<0>>,
					void
				>
				, int
			>::type
		, int*>::value
		, "");
	//filter
	static_assert(std::is_same<filter_if_t<Vector<char, void, int>, lift<std::is_integral>>, Vector<char, int>>::value, "");

	//all
	static_assert(!all_of_t<Vector<char, void>, lift<std::is_integral>>::value, "");
	static_assert(all_of_t<Vector<char, int>, lift<std::is_integral>>::value, "");

	//any
	static_assert(any_of_t<Vector<char, void>, lift<std::is_integral>>::value, "");
	static_assert(!any_of_t<Vector<void, int&>, lift<std::is_integral>>::value, "");

	//cat
	static_assert(std::is_same<cat_t<Vector<>>, Vector<>>::value, "");
	static_assert(std::is_same<cat<Vector<Vector<char>, Vector<>>>::type, Vector<char>>::value, "");

	
}