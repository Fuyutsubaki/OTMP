#include<otmp.hpp>


namespace otmp
{
	//cat
	static_assert(std::is_same<cat_t<List<>>, List<>>::value, "");
	static_assert(std::is_same<cat_t<List<List<char>, List<>, List<int, long>>>, List<char, int, long>>::value, "");

	//filter
	static_assert(std::is_same<filter_if_t<List<char, void, int>, lift<std::is_integral>>, List<char, int>>::value, "");



	//all
	static_assert(!all_t<List<char, void>, lift<std::is_integral>>::value, "");
	static_assert(all_t<List<char, int>, lift<std::is_integral>>::value, "");
}

int main()
{}