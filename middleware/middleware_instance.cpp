//@	{"target":{"name":"middleware_instance.o"}}

#include "./middleware_instance.hpp"
#include "./dummy_params.hpp"

std::string restore::generate_session_key(size_t length_in_bytes)
{
	auto random = west::io::open("/dev/urandom", 0);
	std::array<char, 65536> buffer{};
	std::vector<char> byte_array;
	while(length_in_bytes != 0)
	{
		auto const bytes_to_read = std::min(length_in_bytes, std::size(buffer));
		auto const bytes_read = ::read(random.get(), std::data(buffer), bytes_to_read);
		if(bytes_read == -1)
		{
			if(errno != EWOULDBLOCK && errno != EAGAIN)
			{ throw std::runtime_error{"Failed to generate a session key"}; }
			continue;
		}

		byte_array.insert(std::end(byte_array),
			std::data(buffer),
			std::data(buffer) + bytes_read);

		length_in_bytes -= bytes_read;
	}

	auto to_hex_digit = [](auto nibble) {
		return static_cast<char>(nibble < 10 ? nibble + '0' : (nibble - 10 ) + 'A');
	};

	std::string ret{};

	for(size_t k = 0; k != std::size(byte_array); ++k)
	{
		auto const msb = (0xf0 & buffer[k]) >> 4;
		auto const lsb = 0x0f & buffer[k];

		ret.push_back(to_hex_digit(msb));
		ret.push_back(to_hex_digit(lsb));
	}

	return ret;
}

std::string restore::get_session_key(jopp::object const& key_cfg)
{
	auto const& type = key_cfg.get_field_as<jopp::string>("type");
	auto const& params = key_cfg.get_field_as<jopp::object>("params");

	if(type == "generated")
	{
		auto const length = params.get_field_as<jopp::number>("length");
		if(length < 0.0)
		{ throw std::runtime_error{"Invalid key length"}; }

		return generate_session_key(static_cast<size_t>(length));
	}

	return params.get_field_as<jopp::string>("value");
}

restore::middleware_instance restore::create_middleware_instance(jopp::object const& mw_config)
{
	return middleware_instance{
		.resource_file = restore::resource_file{mw_config.get_field_as<jopp::string>("resource_file").c_str()},
		.storage_file = restore::storage_file{mw_config.get_field_as<jopp::string>("storage_file").c_str()},
		.session_key = get_session_key(mw_config.get_field_as<jopp::object>("session_key")),
		.task_metadata{
			.parameter_types = jopp::json_buffer{restore::get_parameter_types()},
			.parameters = jopp::json_buffer{restore::get_task_parameters()}
		}
	};
}

