#ifndef RESTORE_SERVER_SOCKET_CONFIG_HPP
#define RESTORE_SERVER_SOCKET_CONFIG_HPP

#include <jopp/types.hpp>
#include <west/io_inet_server_socket.hpp>

namespace restore
{
	struct inet_port_range
	{
		int min;
		int max;
	};

	auto get_server_port_range(jopp::object const& socket_params)
	{
		auto const& range = socket_params.get_field_as<jopp::object>("server_port_range");

		auto const min_value = range.get_field_as<jopp::number>("min");
		if(min_value < 0.0 || min_value > 65535.0)
		{ throw std::runtime_error{"Server port min value is out of range"}; }
		auto const port_min = static_cast<int>(min_value);

		auto const max_value = range.get_field_as<jopp::number>("min");
		if(max_value < 0.0 || max_value > 65535.0)
		{ throw std::runtime_error{"Server port min value is out of range"}; }
		auto const port_max = static_cast<int>(max_value);

		if(port_min > port_max)
		{ throw std::runtime_error{"Server port max must be greater than server port min"}; }

		return inet_port_range{port_min, port_max};
	}

	auto create_inet_socket(jopp::object const& socket_params)
	{
		auto const& remote_address = socket_params.get_field_as<jopp::string>("remote_address");
		auto const port_range = get_server_port_range(socket_params);
		return west::io::inet_server_socket{
			west::io::inet_address{remote_address.c_str()},
			std::ranges::iota_view{port_range.min, port_range.max + 1},
			128
		};
	}

	auto create_server_socket(jopp::object const& socket_config)
	{
		auto const& socket_type = socket_config.get_field_as<jopp::string>("type");
		if(socket_type == "inet")
		{ return create_inet_socket(socket_config.get_field_as<jopp::object>("params")); }
		else
		{ throw std::runtime_error{"Unsupported socket type"}; }
	}
}
#endif