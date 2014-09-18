#pragma once
#include"magicalOTMP.hpp"
#include<new>
#include<functional>
namespace test
{
	namespace deteil
	{
		using namespace otmp;
		template<class Idx1, class Idx2>
		struct Max
			:wrap<std::integral_constant<std::size_t, ((Idx1::value>Idx2::value) ? Idx1::value : Idx2::value)> > 
		{};
		template<class T>
		struct getSize
			:std::integral_constant<std::size_t,sizeof(T)>
		{};
		
		template<class list>
		using maxAlign = fold1<map<list, lift<std::alignment_of>>, lift<Max>>;
		template<class list>
		using maxSize = fold1<map<list, lift<getSize>>, lift<Max>>;
	}

	template<class...T>
	class variant
	{
		using TypeList = otmp::List<T...>;
		using maxsize = deteil::getSize<TypeList>;
		using maxalign = deteil::maxAlign<TypeList>;

		using storage_type = typename std::aligned_storage<maxsize::value, maxalign::value>::type;
		storage_type storage_;
		std::size_t which_;
	public:
		template<class Visitor>
		void apply(Visitor visitor)
		{
			std::array<std::function<void()>,sizeof...(T)>
				arr{ { [&]{visitor(*reinterpret_cast<T*>(&st)); }... } };
			arr[which_]();
		}
	};

}