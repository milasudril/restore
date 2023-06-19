#ifndef RESTORE_UTILS_HPP
#define RESTORE_UTILS_HPP

#include <algorithm>

namespace restore
{
	template<class ForwardIt, class T, class Compare>
	ForwardIt binary_find(ForwardIt first, ForwardIt last, const T& value, Compare cmp)
	{
		ForwardIt i = std::lower_bound(first, last, value, cmp);
		if (i != last && !cmp(value, *i))
		{ return i; }
		else
		{ return last; }
	}
}

#endif