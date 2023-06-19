#ifndef RESTORE_CONFIG_HPP
#define RESTORE_CONFIG_HPP

#include "./json_loaders.hpp"

#include <jopp/types.hpp>
#include <stdexcept>

namespace restore::config
{
	inline jopp::object load(char const* filename)
	{
		auto obj = json::load(filename);

		if(auto ret = obj.get_if<jopp::object>(); ret != nullptr)
		{ return std::move(*ret); }

		throw std::runtime_error{"Root element in config file is not an object"};
	}
}

#endif