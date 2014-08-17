#include<iostream>
#include<otmp.hpp>

template<class T>
struct in2
	:otmp::in<T, otmp::Vector<int>>::type
	//:otmp::Bool_<std::is_same<T,int>::value>
{};

int main()
{
	using namespace otmp;
	using list = Vector<float, void, double>;
	using R = map<list, lift<in2>>::type;//‚±‚Ìs‚ğÁ‚·‚Æ‚È‚º‚©“®‚©‚È‚¢
	static_assert(otmp::in<void, otmp::Vector<int>>::type::value, "");
}