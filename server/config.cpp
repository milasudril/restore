//@	{"target":{"name":"config.o"}}

#include "./config.hpp"

#include <jopp/parser.hpp>
#include <west/io_fd.hpp>

#include <stdexcept>

jopp::object restore::config::load(char const* filename)
{
	jopp::container root;
	auto src = west::io::open(filename, O_RDONLY, 0);

	jopp::parser parser{root};
	std::array<char, 65536> buffer{};
	auto status = jopp::parser_error_code::completed;
	while(true)
	{
		auto const bytes_read = ::read(src.get(), std::data(buffer), std::size(buffer));
		if(bytes_read == 0)
		{
			if(status != jopp::parser_error_code::completed)
			{ throw std::runtime_error{std::string{"Failed to parse config file: "} + to_string(status)}; }
		}

		// Not handling async I/O here
		if(bytes_read == -1)
		{ throw west::system_error{"Failed to read config file: ", errno}; }

		auto const res = parser.parse(std::span{std::data(buffer), static_cast<size_t>(bytes_read)});
		if(res.ec == jopp::parser_error_code::completed)
		{ break; }
		else
		if(res.ec != jopp::parser_error_code::more_data_needed)
		{ throw std::runtime_error{std::string{"Failed to parse config file: "} + to_string(res.ec)}; }

		status = res.ec;
	}

	if(auto ret = root.get_if<jopp::object>(); ret != nullptr)
	{ return std::move(*ret); }

	throw std::runtime_error{"Root element in config file is not an object"};
}