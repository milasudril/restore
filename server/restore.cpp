//@	{"target":{"name":"restore.o"}}

#include "./config.hpp"

#include <west/io_inet_server_socket.hpp>

namespace
{
	jopp::string const& get_socket_type(jopp::object const& socket_config)
	{
		auto const type = socket_config.find("type");
		if(type == std::end(socket_config))
		{ throw std::runtime_error{"No socket type specified"}; }

		auto const type_value = type->second.get_if<jopp::string>();
		if(type_value == nullptr)
		{ throw std::runtime_error{"Socket type must be specified as a string"}; }

		return *type_value;
	}

	jopp::object const& get_socket_params(jopp::object const& socket_config)
	{
		auto const params = socket_config.find("params");
		if(params == std::end(socket_config))
		{ throw std::runtime_error{"No socket parameters are given"}; }

		auto const params_value =params->second.get_if<jopp::object>();
		if(params_value == nullptr)
		{ throw std::runtime_error{"Socket parameters must be an object"}; }

		return *params_value;
	}

	jopp::string const& get_remote_address(jopp::object const& socket_params)
	{
		auto const addr = socket_params.find("remote_address");
		if(addr == std::end(socket_params))
		{ throw std::runtime_error{"No remote address is given"}; }

		auto const addr_value = addr->second.get_if<jopp::string>();
		if(addr_value == nullptr)
		{ throw std::runtime_error{"Remote address must be a string"}; }

		return *addr_value;
	}

	struct inet_port_range
	{
		int min;
		int max;
	};

	auto get_server_port_range(jopp::object const& socket_params)
	{
		auto const range = socket_params.find("server_port_range");
		if(range == std::end(socket_params))
		{ throw std::runtime_error{"No server port range is given"}; }

		auto const range_value = range->second.get_if<jopp::object>();
		if(range_value == nullptr)
		{ throw std::runtime_error{"Server port range must be an obejct"}; }

		auto const range_min = range_value->find("min");
		if(range_min == std::end(*range_value))
		{ throw std::runtime_error{"Server port range has no min value"}; }
		auto const min_value = range_min->second.get_if<jopp::number>();
		if(min_value == nullptr)
		{ throw std::runtime_error{"Server port min value must be a number"}; }
		if(*min_value < 0.0 || *min_value > 65535.0)
		{ throw std::runtime_error{"Server port min value is out of range"}; }
		auto const port_min = static_cast<int>(*min_value);

		auto const range_max = range_value->find("max");
		if(range_max == std::end(*range_value))
		{ throw std::runtime_error{"Server port range has no max value"}; }
		auto const max_value = range_min->second.get_if<jopp::number>();
		if(max_value == nullptr)
		{ throw std::runtime_error{"Server port max value must be a number"}; }
		if(*max_value < 0.0 || *max_value > 65535.0)
		{ throw std::runtime_error{"Server port max value is out of range"}; }
		auto const port_max = static_cast<int>(*max_value);

		if(port_min > port_max)
		{ throw std::runtime_error{"Server port max must be greater than server port min"}; }

		return inet_port_range{port_min, port_max};
	}

	auto create_inet_socket(jopp::object const& socket_params)
	{
		auto const& remote_address = get_remote_address(socket_params);
		auto const port_range = get_server_port_range(socket_params);
		return west::io::inet_server_socket{
			west::io::inet_address{remote_address.c_str()},
			std::ranges::iota_view{port_range.min, port_range.max + 1},
			128
		};
	}

	auto create_socket(jopp::object const& socket_config)
	{
		auto const& socket_type = get_socket_type(socket_config);
		if(socket_type == "inet")
		{ return create_inet_socket(get_socket_params(socket_config)); }
		else
		{ throw std::runtime_error{"Unsupported socket type"}; }
	}
}

int main(int argc, char** argv)
{
	if(argc < 2)
	{
		fprintf(stderr, "Config file not specified\n");
		return -1;
	}

	auto const cfg = restore::config::load(argv[1]);
	auto const socket_cfg = cfg.find("socket_config");
	if(socket_cfg == std::end(cfg))
	{
		fprintf(stderr, "No socket configuration specified\n");
		return -1;
	}

	auto const socket_cfg_obj = socket_cfg->second.get_if<jopp::object>();
	if(socket_cfg_obj == nullptr)
	{
		fprintf(stderr, "socket_config must be an object");
		return -1;
	}

	auto http_socket = create_socket(*socket_cfg_obj);
	printf("Listening on port %u\n", http_socket.port());
	fflush(stdout);

	return 0;
}