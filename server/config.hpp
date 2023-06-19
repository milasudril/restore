#ifndef RESTORE_CONFIG_HPP
#define RESTORE_CONFIG_HPP

#include "./json_loaders.hpp"

namespace restore::config
{
	inline jopp::object load(char const* filename)
	{ return json::load_object(filename); }
}

#endif