//@	{"target":{"name":"utils.o"}}

#include "./utils.hpp"

#include <ctime>

namespace
{
	std::array<char const*, 7> daynames{
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};

	std::array<char const*, 12> monthnames{
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
}

std::string restore::to_string(std::chrono::system_clock::time_point t)
{
	auto const time = std::chrono::system_clock::to_time_t(t);

	tm res{};
	gmtime_r(&time, &res);

	std::string ret{daynames[std::clamp(res.tm_wday, 0, 7)]};
	ret.append(", ")
		.append(std::to_string(res.tm_mday))
		.append(" ")
		.append(monthnames[std::clamp(res.tm_mon, 0, 11)])
		.append(" ")
		.append(std::to_string(res.tm_year + 1900))
		.append(" ");

	if(res.tm_hour < 10)
	{ ret.append("0"); }
	ret.append(std::to_string(res.tm_hour)).append(":");

	if(res.tm_min < 10)
	{ ret.append("0"); }
	ret.append(std::to_string(res.tm_min)).append(":");

	if(res.tm_sec < 10)
	{ ret.append("0"); }
	ret.append(std::to_string(res.tm_sec));

	ret.append(" GMT");

	return ret;
}
