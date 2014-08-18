#include<iostream>
#include<otmp.hpp>


int main()
{
	using namespace otmp;
	std::cout << typeid(any_of_t<List<void, int&>, lift<std::is_integral>>).name();
}