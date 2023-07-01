#ifndef RESTORE_SERVER_SOCKET_HPP
#define RESTORE_SERVER_SOCKET_HPP

#include <jopp/types.hpp>
#include <west/io_inet_server_socket.hpp>

namespace restore
{
	struct inet_port_range
	{
		int min;
		int max;
	};

	inline auto to_inet_port_range(jopp::object const& port_range)
	{
		auto const min_value = port_range.get_field_as<jopp::number>("min");
		if(min_value < 0.0 || min_value > 65535.0)
		{ throw std::runtime_error{"Server port min value is out of range"}; }
		auto const port_min = static_cast<int>(min_value);

		auto const max_value = port_range.get_field_as<jopp::number>("max");
		if(max_value < 0.0 || max_value > 65535.0)
		{ throw std::runtime_error{"Server port min value is out of range"}; }
		auto const port_max = static_cast<int>(max_value);

		if(port_min > port_max)
		{ throw std::runtime_error{"Server port max must be greater than server port min"}; }

		return inet_port_range{port_min, port_max};
	}

	struct inet_server_socket_params
	{
		west::io::inet_address remote_address;
		inet_port_range server_port_range;
	};

	inline auto to_inet_server_socket_params(jopp::object const& socket_params)
	{
		return inet_server_socket_params{
			.remote_address = west::io::inet_address{socket_params.get_field_as<jopp::string>("remote_address").c_str()},
			.server_port_range = to_inet_port_range(socket_params.get_field_as<jopp::object>("server_port_range"))
		};
	}

	inline auto create_inet_socket(inet_server_socket_params const& params)
	{
		return west::io::inet_server_socket{
			params.remote_address,
			std::ranges::iota_view{params.server_port_range.min, params.server_port_range.max + 1},
			128
		};
	}

	inline auto create_server_socket(jopp::object const& socket_config)
	{
		auto const& socket_type = socket_config.get_field_as<jopp::string>("type");
		if(socket_type == "inet")
		{ return create_inet_socket(to_inet_server_socket_params(socket_config.get_field_as<jopp::object>("params"))); }
		else
		{ throw std::runtime_error{"Unsupported socket type"}; }
	}
}
#endif