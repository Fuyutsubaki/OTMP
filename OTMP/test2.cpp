#include"magicalOTMP.hpp"


namespace otmp
{
	//head
	static_assert(std::is_same<head<List<int, char, void>>, int>::value, "");
	
	//map
	static_assert(std::is_same<map<List<int, char>, lift<std::add_pointer>>, List<int*, char*>>::value, "");
	//bind
	static_assert(eval<bind<lift<std::is_same>, Arg<0>, int>, int > ::type::value, "");

	//filter
	static_assert(std::is_same<filter_if<List<char, void, int>, lift<std::is_integral>>, List<char, int>>::value, "");

	//all
	static_assert(!all_of<List<char, void>, lift<std::is_integral>>::value, "");
	static_assert(all_of<List<char, int>, lift<std::is_integral>>::value, "");

	//any
	static_assert(any_of<List<char, void>, lift<std::is_integral>>::value, "");
	static_assert(!any_of<List<void, int&>, lift<std::is_integral>>::value, "");
	
	//cat
	static_assert(std::is_same<cat<List<>>, List<>>::value, "");
	static_assert(std::is_same<cat<List<List<char>, List<>>>, List<char>>::value, "");

	////unique
	//static_assert(std::is_same < unique_t<List< int, long, int, long, void, int>>, List<int, long, void>>::value, "");
}
