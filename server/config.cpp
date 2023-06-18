//@	{"target":{"name":"config.o"}}

#include "./config.hpp"

#include <cstdio>
#include <stdexcept>

namespace
{
	struct file_closer
	{
		void operator()(FILE* file) const
		{ fclose(file); }
	};
}

jopp::object restore::config::load(char const* filename)
{
	jopp::container root;
	std::unique_ptr<FILE, file_closer> src{fopen(filename, "rb")};

	if(auto ret = root.get_if<jopp::object>(); ret != nullptr)
	{ return std::move(*ret); }

	throw std::runtime_error{"Failed to load config file"};
}