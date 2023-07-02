//@	{"dependencies_extra":[{"ref":"./server_socket.o", "rel":"implementation"}]}

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

	inet_port_range to_inet_port_range(jopp::object const& port_range);

	struct inet_server_socket_params
	{
		west::io::inet_address remote_address;
		inet_port_range server_port_range;
	};

	inet_server_socket_params to_inet_server_socket_params(jopp::object const& socket_params);

	inline auto create_inet_socket(inet_server_socket_params const& params)
	{
		return west::io::inet_server_socket{
			params.remote_address,
			std::ranges::iota_view{params.server_port_range.min, params.server_port_range.max + 1},
			128
		};
	}

	using socket_type = west::io::inet_server_socket;

	socket_type create_server_socket(jopp::object const& socket_config);
}
#endif