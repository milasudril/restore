//@	{"target":{"name":"./json_loaders.o"}}

#include "./json_loaders.hpp"

#include <jopp/parser.hpp>

jopp::container restore::json::load(west::io::fd_ref fd)
{
	jopp::container root;
	jopp::parser parser{root};
	std::array<char, 65536> buffer{};
	auto status = jopp::parser_error_code::completed;
	while(true)
	{
		auto const bytes_read = ::read(fd, std::data(buffer), std::size(buffer));
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

	return root;
}